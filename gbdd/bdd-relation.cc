/*
 * bdd-relation.cc: 
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

#include <gbdd/bdd-relation.h>
#include <typeinfo>
#include <iostream>

namespace gbdd
{
/// Get BDD of relation
/**
 * @return The BDD of this relation
 */
const Bdd& BddRelation::get_bdd() const
{
	try
	{
		return dynamic_cast<const Bdd&>(get_bdd_based());
	}
	catch (...)
	{
		cout << typeid(get_bdd_based()).name() << endl;
		assert(false);
	}
}

/// Test for false
/**
 * @return Whether this relation is empty
 */
bool BddRelation::is_false() const
{
	return get_bdd().is_false();
}

/// Test for true
/**
 * @return Whether this relation is universal
 */
bool BddRelation::is_true() const
{
	return get_bdd().is_true();
}

/// Creates a membership relation for a vector of sets
/**
 * The domains of each set must be the same
 *
 * @param sets Sets to enumerate
 * 
 * @return A relation R defined such that
 *          R(x, i) iff x is in sets[i]
 */

BddRelation BddRelation::enumeration(vector<BddSet>& sets)
{
	assert(sets.size() > 0);

	StructureConstraint::VarPool pool;

	Domain set_domain = sets[0].get_domain();
	assert(set_domain.is_finite());
	pool.alloc(set_domain);

	Domain dom_enum = pool.alloc(Bdd::n_vars_needed(sets.size()));

	return enumeration(sets, dom_enum);
}

/// Creates a membership relation for a vector of sets
/**
 * The domains of each set must be the same
 *
 * @param sets Sets to enumerate
 * @param dom_enum Domain to use for enumeration
 * 
 * @return A relation R defined such that
 *          R(x, i) iff x is in sets[i]
 */

BddRelation BddRelation::enumeration(vector<BddSet>& sets, Domain dom_enum)
{
	assert(sets.size() > 0);

	Bdd new_rel = Bdd(sets[0].get_space(), false);
	vector<BddSet>::const_iterator i = sets.begin();
	unsigned int set_index = 0;

	while (i != sets.end())
	{
		new_rel |= (i->get_bdd()) & Bdd::value(i->get_space(), dom_enum, set_index);
		
		set_index++;
		i++;
	}

	Domains new_domains(2);
	new_domains[0] = sets[0].get_domain();
	new_domains[1] = dom_enum;

	return BddRelation(new_domains, new_rel);
}

/// Color a vector of relations
/**
 * Colors \a rels by extending domain with index \a domain_index in
 * each relation by \a color_domain. For each relation \a rels [i],
 * the resulting returned relation have the variables in \a color_domain
 * set to the value i.
 *
 * @param domain_index Domain to color
 * @param color_domain Domain to use for colors
 * @param rels Relations to color
 *
 * 
 * @return The colored relations, in the same order as \a rels
 */

vector<BddRelation> BddRelation::color(unsigned int domain_index,
				       Domain color_domain,
				       vector<BddRelation> rels)
{
	vector<BddRelation> res;

	vector<BddRelation>::const_iterator i;
	unsigned int current_color = 0;
	for(i = rels.begin();i != rels.end();++i)
	{
		Bdd colored_rel = 
			(i->get_bdd()) &
			Bdd::value(i->get_space(), color_domain, current_color);

		Domains colored_doms = i->get_domains();
		colored_doms[domain_index] |= color_domain;

		res.push_back(BddRelation(colored_doms, colored_rel));

		++current_color;
	}

	return res;
}

/// Inserts an element into the relation
/**
 * The domains of the relation are extended if necessary
 *
 * @param vals Element to insert
 */

void BddRelation::insert(const vector<unsigned int>& vals)
{
	assert(vals.size() == arity());

	// Extend domains if necessary

	StructureConstraint::VarPool pool;
	pool.alloc(get_domains().union_all());

	Bdd new_v (get_space(), true);

	for (unsigned int i = 0;i < arity();++i)
	{
		unsigned int v = vals[i];
		unsigned n_vars = Bdd::n_vars_needed(v+1);

		if (get_domain(i).size() < n_vars)
		{
			// Extend domain
			
			unsigned int vars_needed = n_vars - get_domain(i).size();

			// Make sure that new variables are higher than the one in current domain
			pool.alloc(Domain(0, get_domain(i).higher()));
		
			Domain extra_vars = pool.alloc(vars_needed);

			static_cast<StructureRelation&>(*this) = extend_domain(i, get_domain(i) | extra_vars);
		}

		new_v &= Bdd::value(get_space(), get_domain(i), v);
	}


	StructureRelation::reset(get_domains(), get_bdd() | new_v);

	return;
}

void BddRelation::insert(unsigned int v1, unsigned int v2)
{
	vector<unsigned int> v;
	v.push_back(v1);
	v.push_back(v2);

	insert(v);
}


/// Create an empty set 
/**
 * The domains of the returned set will be empty, but will be
 * extended automatically with BddSet::insert
 *
 * @param space Space to represent set with
 * 
 * @return A set with empty domain which is empty
 */

BddSet::BddSet(Space* space):
	ViewT(Domain(), Bdd(space))
{}
		

/// Constructor for a singelton set
/**
 * Creates a singelton set containing the value \a v (binary encoded
 * using BDD)
 *
 * @param space Space for this set
 * @param v   Value in the singelton set 
 */

BddSet::BddSet(Space* space, unsigned int v):
	ViewT(Domain(), Bdd(space))
{
	insert(v);
}

/// Constructor for an interval set
/**
 * Creates a set containing the interval \a from_v to \a to_v (binary encoded
 * using BDD) with the domains from \a set
 *
 * @param space Space to use for the set
 * @param from_v Interval start
 * @param to_v Interval end
 */

BddSet::BddSet(Space* space, unsigned int from_v, unsigned int to_v):
	ViewT(Domain(0, Bdd::n_vars_needed(to_v+1)),
	      Bdd(Bdd::value_range(space, Domain(0, Bdd::n_vars_needed(to_v+1)), from_v, to_v)))
{}

/// Constructor for a singelton set
/**
 * Creates a singelton set containing the value \a v (binary encoded
 * using BDD) with the domains from \a set
 *
 * @param set BddSet to take domain from
 * @param v   Value in the singelton set 
 */

BddSet::BddSet(const BddSet &set, unsigned int v):
	ViewT(set.get_domain(),
	      Bdd::value(set.get_space(), set.get_domain(), v))
{}

/// Constructor for an interval set
/**
 * Creates a set containing the interval \a from_v to \a to_v (binary encoded
 * using BDD) with the domains from \a set
 *
 * @param set BddSet to take domain from
 * @param from_v Interval start
 * @param to_v Interval end
 */

BddSet::BddSet(const BddSet& set, unsigned int from_v, unsigned int to_v):
	ViewT(set.get_domain(),
	      Bdd(Bdd::value_range(set.get_space(), set.get_domain(), from_v, to_v)))
{}

/// Empty set
/**
 * Creates an empty set using the domain of \a set
 * 
 * @param set BddSet to take domain from
 */

BddSet BddSet::empty(const BddSet& set)
{
	return BddSet(set.get_domain(), Bdd(set.get_space(), false));
}

/// Universal set
/**
 * Creates the universal set using the domain of \a set
 *
 * @param set BddSet to take domain from
 */

BddSet BddSet::universal(const BddSet& set)
{
	return BddSet(set.get_domain(), Bdd(set.get_space(), true));
}

/// Inserts a value into the set
/**
 * The domain of the set is extended if necessary
 *
 * @param v Value to insert
 * 
 * @return A pair of an iterator to the inserted value and a boolean value
 * saying whether the value was not there before.
 */

pair<BddSet::iterator,bool> BddSet::insert(unsigned int v)
{
	unsigned n_vars = Bdd::n_vars_needed(v+1);

	if (get_domain().size() < n_vars)
	{
		StructureConstraint::VarPool pool;
		pool.alloc(get_domain());

		Domain new_domain = get_domain();
		unsigned int vars_needed = n_vars - get_domain().size();

		// Make sure that new variables are higher than the one in current domain
		pool.alloc(Domain(0, get_domain().higher()));
		
		Domain extra_vars = pool.alloc(vars_needed);

		static_cast<StructureRelation&>(*this) = extend_domain(get_domain() | extra_vars);
        }
	else
	{
		if (member(v))
		{
			return pair<iterator, bool>(iterator(this, v), false);
		}
	}

	StructureRelation::reset(get_domain(), get_bdd() | Bdd::value(get_space(), get_domain(), v));

	return pair<iterator, bool>(iterator(this, v), true);
}

/// Test membership of value
/**
 * @param v Value to test
 * 
 * @return Whether v is a member of the set
 */
bool BddSet::member(unsigned int v) const
{
	if (Bdd::n_vars_needed(v+1) > get_domain().size()) return false;

	return get_bdd().value_member(get_domain(), v);
}

/// Test for emptiness
/**
 * @return Whether set is empty
 */
bool BddSet::is_empty() const
{
	return is_false();

}

/// Color set
/**
 * Colors \a sets by extending domain of
 * each set by \a color_domain. For each set \a sets [i],
 * the resulting returned set have the variables in \a color_domain
 * set to the value i.
 *
 * @param color_domain Domain to use for colors
 * @param sets Sets to color
 * 
 * @return The colored sets, in the same order as \a sets
 */
vector<BddSet> BddSet::color(Domain color_domain,
			     vector<BddSet> sets)
{
	vector<BddRelation> rels;
	{
		vector<BddSet>::const_iterator i;
		for (i = sets.begin();i != sets.end();++i)
		{
			rels.push_back(*i);
		}
	}

	vector<BddRelation> colored = BddRelation::color(0, color_domain, rels);

	vector<BddSet> res;

	{
		vector<BddRelation>::const_iterator i;
		for(i = colored.begin();i != colored.end();++i)
		{
			res.push_back(BddSet(*i));
		}
	}

	return res;
}

/// Create renaming compressing set
/**
 * @return A injection from elements in this set to {0..n-1} where n is the size of this set
 */
BddRelation BddSet::compress(void) const
{
	vector<BddSet> singeltons;

	iterator i = begin();
	while(i != end())
	{
		singeltons.push_back(BddSet(*this, *i));
		++i;
	}

	return BddRelation::enumeration(singeltons);
}

/// Get size of set
/**
 * @return Number of elements in set
 */
unsigned int BddSet::size() const
{
	return get_bdd().n_assignments(get_domain());
}

// Iterators

/// Constructor
/**
 * @param s Set to iterate over
 * @param begin True if begin, false if end
 */
BddSet::const_iterator::const_iterator(const BddSet* s, bool begin):
	ptr_s(s),
	shadow_set()
{
	if (begin)
	{
		shadow_set = s->get_bdd().assignments_value(s->get_domain());
	}

	shadow_set_i = begin ? shadow_set.begin() : shadow_set.end();
}

/// Constructor
/**
 * @param s Set to iterate over
 * @param v Value pointed to by this iterator
 */
BddSet::const_iterator::const_iterator(const BddSet* s, unsigned int v):
	ptr_s(s),
	shadow_set(s->get_bdd().assignments_value(s->get_domain()))
{
	shadow_set_i = shadow_set.find(v);
}

/// Copy constructor
/**
 * @param i Iterator to copy
 */
BddSet::const_iterator::const_iterator(const const_iterator& i):
	ptr_s(i.ptr_s),
	shadow_set(i.shadow_set)
{
	shadow_set_i = (i.shadow_set_i == i.shadow_set.end()) ? 
		shadow_set.end() : 
		shadow_set.find(*i.shadow_set_i);
}

/// Dereference
/**
 * @return The value currently pointed to by this iterator
 */
unsigned int BddSet::const_iterator::operator*() const
{
	return *shadow_set_i;
}

/// Increase
/**
 * Increases iterator to next element
 * @return This iterator
 */
BddSet::const_iterator& BddSet::const_iterator::operator++()
{
	shadow_set_i++;
	return *this;
}

/// Equality
/**
 * @param i1 First iterator
 * @param i2 Second iterator
 *
 * @return Whether \a i1 and \a i2 are equal
 */
bool operator==(const BddSet::const_iterator& i1, const BddSet::const_iterator& i2)
{
	if (i1.ptr_s != i2.ptr_s) return false;

	bool i1_end = i1.shadow_set_i == i1.shadow_set.end();
	bool i2_end = i2.shadow_set_i == i2.shadow_set.end();

	if (i1_end && i2_end) return true;

	if (i1_end || i2_end) return false;

	return *i1 == *i2;
}

/// Inequality
/**
 * @param i1 First iterator
 * @param i2 Second iterator
 *
 * @return Whether \a i1 and \a i2 are not equal
 */
bool operator!=(const BddSet::const_iterator& i1, const BddSet::const_iterator& i2)
{
	return !(i1 == i2);
}

	
/// Get iterator pointing to beginning
/**
 * @return Iterator pointing to beginning of this set
 */
BddSet::const_iterator BddSet::begin() const
{
	return const_iterator(this, true);
}

/// Get iterator pointing to ending
/**
 * @return Iterator pointing to beginning of this set
 */
BddSet::const_iterator BddSet::end() const
{
	return const_iterator(this, false);
}

/// Print relation
/**
 * @param out Stream to print on
 * @param r Relation to print
 */
ostream& operator<<(ostream &out, const BddRelation &r)
{
	vector<BddSet> projected;
	{
		unsigned int i;
		for (i = 0;i < r.arity();++i)
		{
			projected.push_back(r.project_on(i));
		}
	}
	vector<BddSet::const_iterator> is;

	{
		unsigned int i;
		
		for(i = 0;i < r.arity();++i)
		{
			is.push_back(projected[i].begin());
		}
	}

	out << "{";

	while(is.size() > 0 && is[0] != projected[0].end())
	{
		{
			vector<StructureSet> element;
			unsigned int i;

			for(i = 0; i < is.size();++i)
			{
				element.push_back(BddSet(projected[i], *(is[i])));
			}

			BddRelation prod = StructureRelation::cross_product(r.get_domains(), element);

			if (!(prod & r).is_false())
			{

				out << "(" << *(is[0]);
				vector<BddSet::const_iterator>::iterator j = is.begin();

				++j;
				
				while(j != is.end())
				{
					out << "," << **j;
					++j;
				}
				
				out << ")";
			}
		}

		// Increase to next

		unsigned int i = r.arity() - 1;

		++(is[i]);

		while (is[i] == projected[i].end() && i > 0)
		{
			is.pop_back();
		        --i;
			++(is[i]);
		}
		
		if(is[i] != projected[i].end())
		{
			++i;
			while (i < projected.size())
			{
				is.push_back(projected[i].begin());
				++i;
			}
		}

	}

	out << "}";

	return out;
}

}
	
