/*
 * bdd-relation.h: 
 *
 * Copyright (C) 2002 Marcus Nilsson (marcus@docs.uu.se)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Authors:
 *    Marcus Nilsson (marcus@docs.uu.se)
 */
#ifndef BDD_RELATION_H
#define BDD_RELATION_H

#include <gbdd/structure-relation.h>
#include <gbdd/structure-binary-relation.h>
#include <gbdd/bdd.h>

namespace gbdd
{
	class BddSet;

	/// A relation implemented as BDD
	/**
A relation is a BDD with a vector of variables stating what variables
in the BDD are used to implement what domain. The number of domains is equal
to the arity of the relation.

The following code illustrates the class. It creates three relations.
The relation \c rel1 is the set {(0,10),(2,10),(5,10)}, the relation
\c rel2 the set {(0,10),(1,10),(2,10)}, and \c mapper the set
{(0,0),(2,1),(5,2)}. Finally it checks that \c rel1 composed
with \c mapper in the first component of \c rel1 gives \c rel2 as
a result. Note also that \c mapper uses other variables than \c rel1 and \c rel2, the \c compose
method takes care of the renaming for us.

\code
Space *space = Space::create_default();

Domains domains1(2);

domains1[0] = Domain(0, 5);
domains1[1] = Domain(5, 5);

Domains domains2(2);

domains2[0] = Domain(3, 5);
domains2[1] = Domain(9, 5);


BddRelation rel1 = BddRelation(domains1,
			 (Bdd::value(space, domains1[0], 0) |
			  Bdd::value(space, domains1[0], 2) |
			  Bdd::value(space, domains1[0], 5)) &
			 Bdd::value(space, domains1[1], 10));

BddRelation rel2 = BddRelation(domains1,
			 (Bdd::value(space, domains1[0], 0) |
			  Bdd::value(space, domains1[0], 1) |
			  Bdd::value(space, domains1[0], 2)) &
			 Bdd::value(space, domains1[1], 10));
	
BddRelation mapper = BddRelation(domains2,
			   (Bdd::value(space, domains2[0], 0) &
			    Bdd::value(space, domains2[1], 0)) |
			   (Bdd::value(space, domains2[0], 2) &
			    Bdd::value(space, domains2[1], 1)) |
			   (Bdd::value(space, domains2[0], 5) &
			    Bdd::value(space, domains2[1], 2)));
	
BddRelation composed_rel1 = rel1.compose(0, mapper);

return (composed_rel1 == rel2);
\endcode
*/
	class BddRelation : public SpecializedRelation<Bdd, BddRelation, BddSet>
	{
	public:
		typedef SpecializedRelation<Bdd, BddRelation, BddSet> SpecT;
/// Uninitialized constructor
		BddRelation() {}

/// Copy constructor
/**
 * @param r Relation to copy
 */
		BddRelation(const StructureRelation& r):SpecT(r) {};

/// Constructor for empty relation
/**
 * @param vs Variables to represent the relation with
 * @param value value=true gives universal relation, value=false gives empty
 */
		BddRelation(const Bdd::FiniteVars& vs, bool value = false):
			SpecT(vs.get_domains(), Bdd(vs.get_space(), value))
			{}
/// Constructor
/**
 * @param vs Variables for relation
 * @param rel_bdd Bdd over \a vs describing relation
 */

		BddRelation(const Bdd::FiniteVars& vs, Bdd rel_bdd):SpecT(vs.get_domains(), rel_bdd) {}

/// Constructor
/**
 * @param ds Domains of relation
 * @param rel_bdd Bdd representing the relation
 */
		BddRelation(const Domains &ds, Bdd rel_bdd):SpecT(ds, rel_bdd) {};

/// Adapt domains with automatic renaming
/**
 * @param ds New domains
 * @param r Relation to adapt
 */
		BddRelation(const Domains &ds, const BddRelation& r):SpecT(ds, r) {};

		const Bdd& get_bdd() const;

/// Create empty relation
/**
 * This can be used to create an empty relation which can be populated with the insert method, which
 * automatically extends the domains
 *
 * @param Space to implemented BDD relation
 * @param arity Arity of relation
 */
		BddRelation(Space* space, unsigned int arity):
			SpecT(Domains(arity), Bdd(space, false))
			{}

/// Get space of relation
/**
 * @return The BDD space used to implement relation
 */
		Space* get_space() const { return get_bdd().get_space(); }

		bool is_false() const;
		bool is_true() const;

		friend ostream& operator<<(ostream &out, const BddRelation &r);

		static BddRelation enumeration(vector<BddSet>& sets);
		static BddRelation enumeration(vector<BddSet>& sets, Domain dom_enum);

		static vector<BddRelation> color(unsigned int domain_index,
					      Domain color_domain,
					      vector<BddRelation> rels);

		void insert(const vector<unsigned int>& vals);
		void insert(unsigned int v1, unsigned int v2);
	};

	class BddSet : public StructureSetView<Bdd, BddRelation, BddSet>
	{
		typedef StructureSetView<Bdd, BddRelation, BddSet> ViewT;
	public:
		BddSet() {}
		BddSet(const StructureRelation& r):ViewT(r) {};
		BddSet(const Domain &d, Bdd rel_bdd):ViewT(d, rel_bdd) {};
		BddSet(const Domain &d, const BddSet& r):ViewT(d, r) {};

		BddSet(Space* space);
		BddSet(Space* space, unsigned int v);
		BddSet(Space* space, unsigned int from_v, unsigned int to_v);

		BddSet(const BddSet& s, unsigned int v);
		BddSet(const BddSet& s, unsigned int from_v, unsigned int to_v);

		static BddSet empty(const BddSet& set);
		static BddSet universal(const BddSet& set);

		bool member(unsigned int v) const;

		bool is_empty() const;

		static vector<BddSet> color(Domain color_domain,
					 vector<BddSet> sets);

		BddRelation compress(void) const;

		unsigned int size() const;

		friend struct hash<BddSet>;

		/// Iterator for iterating over member of a BddSet 

		class const_iterator
		{
			set<unsigned int>::const_iterator shadow_set_i;
			set<unsigned int> shadow_set;
			const BddSet* ptr_s;
		public:
			const_iterator(const BddSet* s, bool begin);
			const_iterator(const BddSet* s, unsigned int v);
			const_iterator(const const_iterator& i);

                        unsigned int operator*() const;

                        const_iterator& operator++();

                        friend bool operator==(const const_iterator& i1, const const_iterator& i2);
                        friend bool operator!=(const const_iterator& i1, const const_iterator& i2);
		};
			
		typedef const_iterator iterator;

		const_iterator begin() const;
		const_iterator end() const;

		pair<iterator,bool> insert(unsigned int v);
	};

	typedef StructureBinaryView<Bdd, BddRelation, BddSet> BddBinaryRelation;
}		

DECL_NAMESPACE_SGI
{

/// Hash function for gbdd::BddSet
struct hash<gbdd::BddSet>
{
	size_t operator()(gbdd::BddSet s) const
	{
		return hash<gbdd::Bdd>()(s.get_bdd());
	}
};

}

#endif /* BDD_RELATION_H */
