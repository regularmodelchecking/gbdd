/*
 * bdd-equivalence-relation.cc: 
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

#include <gbdd/bdd-equivalence-relation.h>
#include <gbdd/sgi_ext.h>
#include <assert.h>

#define max(a, b) ((a>b) ? a : b)


namespace gbdd
{

/// Copy constructor
/**
 * @param r Relation to copy
 */
BddEquivalenceRelation::BddEquivalenceRelation(const StructureRelation& r):
	BddBinaryRelation(r)
{
	// TODO: Check type
}

/// Constructor with domain adaptation
/**
 * @param domain1 First domain of new relation
 * @param domain2 Second domain of new relation
 * @param r Relation to adapt
 */

BddEquivalenceRelation::BddEquivalenceRelation(const Domain& domain1,
					       const Domain& domain2,
					       const BddEquivalenceRelation& r):
	BddBinaryRelation(domain1, domain2, r)
{}

/// Constructor
/**
 * @param domain1 First domain of new relation
 * @param domain2 Second domain of new relation
 * @param rel_bdd BDD representing relation
 */
BddEquivalenceRelation::BddEquivalenceRelation(const Domain &domain1,
					       const Domain &domain2,
					       Bdd rel_bdd):
	BddBinaryRelation(domain1, domain2, rel_bdd)
{}

/// Equality
/**
 * 
 *
 * @param rel1 First relation
 * @param rel2 Second relation
 * 
 * @return Whether \a rel1 and \a rel2 are equal
 */

BddEquivalenceRelation
operator&(const BddEquivalenceRelation& rel1,
	  const BddEquivalenceRelation& rel2)
{
	return static_cast<const BddRelation&>(rel1) & static_cast<const BddRelation&>(rel2);
}

BddEquivalenceRelation
BddEquivalenceRelation::operator!() const
{
	return !static_cast<const BddRelation&>(*this);
}

/// Restrict equivalence relation
/**
 * 
 *
 * @param s Set to restrict with
 * 
 * @return Equivalence relation with both domains restricted to \a s
 */

BddEquivalenceRelation
BddEquivalenceRelation::restrict(const BddSet& s) const
{
	return restrict_range(s).restrict_image(s);
}

/// Construct identity relation
/**
 * 
 *
 * @param space Space of returned relation
 * @param domain1 First domain of returned relation
 * @param domain2 Second domain of returned relation
 * 
 * @return The identity relation with domains \a domain1 and \a domain2
 */

BddEquivalenceRelation
BddEquivalenceRelation::identity(Space* space,
				 const Domain &domain1,
				 const Domain &domain2)
{
	return BddEquivalenceRelation(domain1,
				      domain2,
				      Bdd::vars_equal(space, domain1, domain2));
}

static void find_with_geq_var(Bdd p, Bdd::Var v, hash_set<Bdd>& res)
{
	if (p.bdd_is_leaf())
	{
		res.insert(p);
	}
	else
	{
		Bdd::Var p_v = p.bdd_var();

		if (p_v >= v)
		{
			res.insert(p);
		}
		else
		{
			find_with_geq_var(p.bdd_then(), v, res);
			find_with_geq_var(p.bdd_else(), v, res);
		}
	}

	return;
}

/// Get quotients 
/**
 * 
 *
 * @param s Set to divide
 * 
 * @return The equivalence classes of \a s with respect to this relation
 */
vector<BddSet>
BddEquivalenceRelation::quotient(const BddSet& s) const
{
	Domain dom0 = get_domain(0);
	Domain dom1 = get_domain(1);

	if(!dom0.is_finite())
	  { // all, dom0 and dom1 should be both infinite, i.e.
	    // they must be interleaved...
	    Bdd::Var vh = get_bdd().highest_var();
	    unsigned int sz = max(vh, 0) + 1;

	    dom0 = dom0.cut_to_same_size(Domain(0, sz));
	    dom1 = dom1.cut_to_same_size(Domain(0, sz));
	  }
	else if(dom0.size() == 0)
	  return vector<BddSet>(1, s);

	//	VarPool pool;

	//	Domain new_dom = pool.alloc(Domain(0, dom0.size()));
	Domain new_dom(dom0.size(), dom0.size());
	Domain new_im(2*dom0.size(), dom1.size());
		
	BddEquivalenceRelation adapted =
		BddEquivalenceRelation(new_dom,
				    new_im,
				    get_bdd().rename(Domain::map_vars(dom0, new_dom) |
						     Domain::map_vars(dom1, new_im)));

	hash_set<Bdd> found_sets = adapted.get_bdd().with_geq_var(*(new_im.begin()));

	// Assumption: new_im does not contain "holes".
	Domain dom_found_sets = Domain::infinite(*(new_im.begin()));
	Bdd bdd_s = BddSet(get_domain(1), s).get_bdd().rename(dom1, new_im);

	vector<BddSet> res;
	{
		hash_set<Bdd>::iterator i = found_sets.begin();
		while (i != found_sets.end())
		{
			if (!((*i & bdd_s).is_false()))
			res.push_back(BddSet(dom_found_sets, *i & bdd_s));
			
			++i;
		}
	}
	
	return res;
}

}
