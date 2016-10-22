/*
 * structure-relation.h: 
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

#ifndef STRUCTURE_RELATION_H
#define STRUCTURE_RELATION_H

#include <gbdd/structure-constraint.h>

namespace gbdd
{
	class StructureSet;

	/// Typed Structure objects
	/**
A structure relation is a typed gbdd::StructureConstraint object. Any
gbdd::StructureConstraint object is based on representing a structure which is
based on variables. By typing, we associate with the object
gbdd::Domains, a vector of gbdd::Domain, i.e. a set of variables. The
gbdd::StructureConstraint object is interpreted with the domains as a relation of
arity the number of domains.

This is an implementation class, inheriting classes are
specializations to this class, adding only specialized operations
implemented with existing methods in this class.
	*/

	class StructureRelation
	{
/**
 * The structure object
 * 
 */
		auto_ptr<StructureConstraint> bb;
/**
 * Type
 * 
 */
		auto_ptr<Domains> domains;
		static StructureConstraint* copy_bb_from_relation(const StructureRelation& r);
	protected:
		void reset(const Domains& ds, const StructureConstraint& bb);
	public:
		StructureRelation();
		StructureRelation(const StructureRelation& r);
		StructureRelation(const Domains& ds, const StructureConstraint& bb);
		StructureRelation(const Domains& ds, StructureConstraint* bb);
		StructureRelation(const Domains& ds, const StructureRelation& r);

		StructureRelation& operator=(const StructureRelation& r);

		virtual ~StructureRelation();

		static StructureRelation cross_product(const Domains& domains, const vector<StructureSet>& sets);

		const Domains& get_domains() const;
		const Domain& get_domain(unsigned int domain_index) const;

		unsigned int arity() const;

		const StructureConstraint& get_bdd_based() const;

		StructureRelation extend_domain(unsigned int domain_index, const Domain& to, bool new_vars_value = false) const;
		StructureRelation reduce_domain(unsigned int domain_index, const Domain& to) const;

		StructureRelation compose(unsigned int domain_index, const StructureRelation& compose_rel) const;

		StructureRelation product(const StructureRelation& r2, bool (*fn)(bool v1, bool v2)) const;

		bool operator==(const StructureRelation& rel2) const;

		friend StructureRelation operator&(const StructureRelation& rel1, const StructureRelation& rel2);
		friend StructureRelation operator|(const StructureRelation& rel1, const StructureRelation& rel2);
		friend StructureRelation operator-(const StructureRelation& rel1, const StructureRelation& rel2);

		StructureRelation& operator&=(const StructureRelation& rel2);
		StructureRelation& operator|=(const StructureRelation& rel2);
		StructureRelation& operator-=(const StructureRelation& rel2);

		StructureRelation operator!() const;

/// IFF product
/**
 * 
 *
 * @param rel1 First relation
 * @param rel2 Second relation
 * 
 * @return IFF product of \a rel1 and \a rel2
 */
		static StructureRelation iff(const StructureRelation &rel1, const StructureRelation &rel2)
		{
			return rel1.product(rel2, StructureConstraint::fn_iff);
		}

/// IMPLIES product
/**
 * 
 *
 * @param rel1 First relation
 * @param rel2 Second relation
 * 
 * @return IMPLIES product of \a rel1 and \a rel2
 */
		static StructureRelation implies(const StructureRelation &rel1, const StructureRelation &rel2)
		{
			return rel1.product(rel2, StructureConstraint::fn_implies);
		}

		StructureSet project_on(unsigned int domain_index) const;

		StructureRelation project(unsigned int domain_index) const;

		StructureRelation restrict(unsigned int domain_index, const StructureSet& to) const;
	};

	template <class StructureT, class RelationT, class SetT>
	class SpecializedRelation : public StructureRelation
	{
	public:
		SpecializedRelation() {}
		SpecializedRelation(const StructureRelation& r):
			StructureRelation(r)
			{}
		SpecializedRelation(const Domains& ds, const StructureT& bb):
			StructureRelation(ds, bb)
			{}
				
		SpecializedRelation(const Domains& ds, const SpecializedRelation& r):
			StructureRelation(ds, r)
			{}

		SpecializedRelation& operator=(const SpecializedRelation& r)
			{
				StructureRelation::operator=(r);
				return *this;
			}

		virtual ~SpecializedRelation()
			{}

		const StructureT& get_bdd_based() const
			{
				return dynamic_cast<const StructureT&>(StructureRelation::get_bdd_based());
			}

		RelationT extend_domain(unsigned int domain_index, const Domain& to, bool new_vars_value = false) const
			{
				return StructureRelation::extend_domain(domain_index, to, new_vars_value);
			}

		RelationT reduce_domain(unsigned int domain_index, const Domain& to) const
			{
				return StructureRelation::reduce_domain(domain_index, to);
			}

		RelationT compose(unsigned int domain_index, const RelationT& compose_rel) const
			{
				return StructureRelation::compose(domain_index, compose_rel);
			}

		RelationT product(const RelationT& r2, bool (*fn)(bool v1, bool v2)) const
			{
				return StructureRelation::product(r2, fn);
			}

		RelationT operator&(const RelationT& rel2) const
			{
				return static_cast<const StructureRelation&>(*this) & static_cast<const StructureRelation&>(rel2);
			}

		RelationT operator|(const RelationT& rel2) const
			{
				return static_cast<const StructureRelation&>(*this) | static_cast<const StructureRelation&>(rel2);
			}

		RelationT operator-(const RelationT& rel2) const
			{
				return static_cast<const StructureRelation&>(*this) - static_cast<const StructureRelation&>(rel2);
			}

		RelationT& operator&=(const RelationT& rel2)
			{
				StructureRelation::operator&=(rel2);
				return dynamic_cast<RelationT&>(*this);
			}

		RelationT& operator|=(const RelationT& rel2)
			{
				StructureRelation::operator|=(rel2);
				return dynamic_cast<RelationT&>(*this);
			}

		RelationT& operator-=(const RelationT& rel2)
			{
				StructureRelation::operator-=(rel2);
				return dynamic_cast<RelationT&>(*this);
			}

		RelationT operator!() const
			{
				return StructureRelation::operator!();
			}

		static RelationT iff(const RelationT &rel1, const RelationT &rel2)
		{
			return rel1.product(rel2, StructureT::fn_iff);
		}

		static RelationT implies(const RelationT &rel1, const RelationT &rel2)
		{
			return rel1.product(rel2, StructureT::fn_implies);
		}

		SetT project_on(unsigned int domain_index) const
			{
				return StructureRelation::project_on(domain_index);
			}

		RelationT project(unsigned int domain_index) const
			{
				return StructureRelation::project(domain_index);
			}

		RelationT restrict(unsigned int domain_index, const SetT& to) const
			{
				return StructureRelation::restrict(domain_index, to);
			}
	};

	template <class StructureT, class RelationT, class SetT>
	class StructureSetView : public RelationT
	{
	public:
		StructureSetView(const StructureRelation& r) : RelationT(r) {}
		StructureSetView() : RelationT() {}
		StructureSetView(const Domain& domain, const StructureT& bb) : RelationT(domain, bb) {}
		StructureSetView(const Domain& domain, const SetT& s) : RelationT(domain, s) {}

		~StructureSetView() {}

/// AND product
/**
 * 
 *
 * @param s2 Set two
 * 
 * @return Intersection of this set and \a s2
 */
		SetT operator&(const SetT& s2) const
			{
				return static_cast<const StructureRelation&>(*this) & static_cast<const StructureRelation&>(s2);
			}

/// OR product
/**
 * 
 *
 * @param s2 Set one
 * 
 * @return Union of this set and \a s2
 */
		SetT operator|(const SetT& s2) const
			{
				return static_cast<const StructureRelation&>(*this) | static_cast<const StructureRelation&>(s2);
			}
		
/// MINUS product
/**
 * 
 *
 * @param s2 Set two
 * 
 * @return This set minus \a s2
 */
		SetT operator-(const SetT& s2) const
			{
				return static_cast<const StructureRelation&>(*this) - static_cast<const StructureRelation&>(s2);
			}
		

		SetT& operator&=(const SetT& rel2)
			{
				StructureRelation::operator&=(rel2);
				return dynamic_cast<SetT&>(*this);
			}

		SetT& operator|=(const SetT& rel2)
			{
				StructureRelation::operator|=(rel2);
				return dynamic_cast<SetT&>(*this);
			}

		SetT& operator-=(const SetT& rel2)
			{
				StructureRelation::operator-=(rel2);
				return dynamic_cast<SetT&>(*this);
			}
/// Negation
/**
 * The universe is taken to be {0..2^n-1} for the domain of the set
 *
 * @return The negation of this set
 */
		SetT operator!() const
			{
				return StructureRelation::operator!();
			}
		
/// Extends domain with new variables with some value
/**
 * The resulting set will have \a to as the new domain
 * and will contain a structure object where all the new variables
 * are constrained to \a new_vars_value It is required the first
 * variables in \a to is the same as in the current domain.
 *
 * @param to Domain to extend to
 * @param new_vars_value Value to constrain new variables to
 * 
 * @return The set with extended domain
 */

		SetT extend_domain(const Domain& to, bool new_vars_value = false) const
			{
				return SetT(to, StructureRelation::extend_domain(0, to, new_vars_value));
			}
		
/// Reduces domain by projecting remaining variables	
/**
 * @param to           Domain to reduce to
 * 
 * @return The set with reduced domain
 */

		
		SetT reduce_domain(const Domain& to) const
			{
				return SetT(to, StructureRelation::reduce_domain(0, to));
			}
		
/// Get domain of set
/**
 * 
 * @return The domain of this set
 */

		Domain get_domain() const
			{
				return StructureRelation::get_domain(0);
			}
		
/// Compose set
/**
 * @param compose_rel Relation to compose with
 * 
 * @return The image of \a compose_rel under this set
 */
		SetT compose(const RelationT& compose_rel) const
			{
				return StructureRelation::compose(0, compose_rel);
			}

		SetT product(const SetT& r2, bool (*fn)(bool v1, bool v2)) const
			{
				return StructureRelation::product(r2, fn);
			}
		
	};

	class StructureSet : public StructureSetView<StructureConstraint, StructureRelation, StructureSet>
	{
		typedef StructureSetView<StructureConstraint, StructureRelation, StructureSet> ViewT;
	public:
		StructureSet(const StructureRelation& r);
		StructureSet();
		StructureSet(const Domain& domain, const StructureConstraint& bb);
		StructureSet(const Domain& domain, const StructureSet& s);

		virtual ~StructureSet();
	};
}

#endif /* STRUCTURE_RELATION_H */

