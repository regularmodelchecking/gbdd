/*
 * structure-relation.cc: 
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

#include <gbdd/structure-relation.h>

namespace gbdd
{
	
static Domain::Var expand_even(Domain::Var v) { return 2 * v; }
static Domain::Var expand_odd(Domain::Var v) { return 2 * v + 1; }

static Domain::Var expand3_0(Domain::Var v) { return 3 * v; }
static Domain::Var expand3_1(Domain::Var v) { return 3 * v + 1; }
static Domain::Var expand3_2(Domain::Var v) { return 3 * v + 2; }

/// Constructor for uninitialized value

StructureRelation::StructureRelation():
	domains(new Domains())
{}

/// Clone structure object
/**
 * @param r Relation to clone structure object from
 * 
 * @return structure object clone or 0 if \a r is uninitialized
 */

StructureConstraint* StructureRelation::copy_bb_from_relation(const StructureRelation& r)
{
	if (r.bb.get() == 0)
	{
		return 0;
	}
	else
	{
		return r.get_bdd_based().ptr_clone();
	}
}

void StructureRelation::reset(const Domains& ds, const StructureConstraint& bb)
{
	this->domains.reset(new Domains(ds));
	this->bb.reset(bb.ptr_clone());
}
/// Copy constructor
/**
 * 
 *
 * @param r Relation to copy
 */

StructureRelation::StructureRelation(const StructureRelation& r):
	bb(copy_bb_from_relation(r)),
	domains(new Domains(r.get_domains()))
{}

/// Create new relation from domains and a structure object
/**
 * @param ds Domains for relation
 * @param bb structure object denoting the relation, using the variables in \a domains
 *
 */

StructureRelation::StructureRelation(const Domains& ds, const StructureConstraint& bb):
	bb(bb.ptr_clone()),
	domains(new Domains(ds))
{}

/// Create new relation from domains and a structure object
/**
 * @param ds Domains for relation
 * @param bb structure object denoting the relation, using the variables in \a domains
 *
 */

StructureRelation::StructureRelation(const Domains& ds, StructureConstraint* bb):
	bb(bb),
	domains(new Domains(ds))
{}
	
/// Changes the domain of a relation with automatic renaming
/**
 * @param ds New domains for relation
 * @param r       Relation to adapt
 * 
 */

StructureRelation::StructureRelation(const Domains& ds, const StructureRelation& r)
{
	if (r.get_domains() == ds)
	{
		*this = r;
		return;
	}

	Domains old_domains = r.get_domains();
	Domains new_domains = ds;

	/* Check if some domain is infinite, in this case make then
	 * finite looking at the variables in the BDD
	 */

	if (old_domains.is_some_infinite())
	{
		Domain::Var high = r.get_bdd_based().highest_var();

		unsigned int i;
		for (i = 0;i < old_domains.size();++i)
		{
			if(old_domains[i].is_finite() == false)
			{

				old_domains[i] &= Domain(0, high + 1);
			}
		}
	}

	/* Cut new domains */

	new_domains = new_domains.cut_to_same_sizes(old_domains);

	Domain old_vars = old_domains.union_all();
	Domain new_vars = new_domains.union_all();

	Domain all_vars = old_vars | new_vars;

	StructureConstraint::VarPool pool;
	pool.alloc(old_vars | new_vars);

	Domain temp_vars = pool.alloc(old_vars.size());
	Domain::VarMap old_to_temp = Domain::map_vars(old_vars, temp_vars);

	Domain::VarMap temp_to_new;

	Domains::const_iterator i1;
	Domains::const_iterator i2;

	i1 = old_domains.begin();
	i2 = new_domains.begin();

	while (i1 != old_domains.end())
	{
		assert (i2 != new_domains.end());

		temp_to_new = temp_to_new | Domain::map_vars(i1->transform(old_to_temp), *i2);

		++i1;
		++i2;
	}

	this->domains.reset(new Domains(new_domains));
	this->bb.reset(auto_ptr<StructureConstraint>(r.get_bdd_based().ptr_rename(old_to_temp))->ptr_rename(temp_to_new));

	// Check if some domain were cut and extend it otherwise

	for (unsigned int i = 0;i < ds.size();++i)
	{
		if (new_domains[i] != ds[i])
		{
			if (ds[i].is_finite())
			{
				*this = extend_domain(i, ds[i]);
			}
			else
			{
				(*(this->domains))[i] = ds[i];
			}
		}
	}
}

/// Assignment
/**
 * @param r Relation to assign from
 * 
 * @return This object
 */
StructureRelation& StructureRelation::operator=(const StructureRelation& r)
{
	domains.reset(new Domains(r.get_domains()));
	bb.reset(r.get_bdd_based().ptr_clone());

	return *this;
}

/// Destructor
/**
 */
StructureRelation::~StructureRelation()
{
	// WORKAROUND for bug in GCC
	//
	// With multiple virtual inheritance, this destructor gets wrongly called twice. By resetting these manually,
	// this does no harm.

	domains.reset();
	bb.reset();
}

/// Get domains for relation
/**
 * 
 *
 * @return Domains for relation
 */
const Domains& StructureRelation::get_domains() const
{
	return *domains;
}

/// Get domain with specified index
/**
 * Domain indices are counted from 0
 *
 * @param domain_index Index of domain to get
 * 
 * @return Domain of relation with index \a domain_index
 */

const Domain& StructureRelation::get_domain(unsigned int domain_index) const
{
	return get_domains()[domain_index];
}
	
/// Get arity of relation
/**
 * @return Arity of relation
 */


unsigned int StructureRelation::arity() const
{
	return get_domains().size();
}

/// Get structure object
/**
 *
 * @return structure object
 */
const StructureConstraint& StructureRelation::get_bdd_based() const
{
	assert(bb.get() != 0);
	return *bb;
}

/// Extends domain with new variables with some value
/**
 * The resulting relation will have \a to as the new domain at index
 * \a domain_index, and will contain a structure object where all the new variables
 * are constrained to \a new_vars_value It is required the first
 * variables in \a to is the same as in the current domain.
 *
 * @param domain_index Index of domain to extend
 * @param to Domain to extend to
 * @param new_vars_value Value to constrain new variables to
 * 
 * @return The relation with extended domain 
 */


StructureRelation
StructureRelation::extend_domain(unsigned int domain_index, const Domain& to, bool new_vars_value) const
{
	Domain from = get_domain(domain_index);

	Domain::const_iterator from_i = from.begin();
	Domain::const_iterator to_i = to.begin();

	while(from_i != from.end())
	{
		assert(to_i != to.end());
		assert(*from_i == *to_i);

		++from_i;
		++to_i;
	}

	auto_ptr<StructureConstraint> new_rel(get_bdd_based().ptr_clone());
	
	while(to_i != to.end())
	{
		new_rel.reset(new_rel->ptr_constrain_value(*to_i, new_vars_value));

		++to_i;
	}

	Domains new_domains = get_domains();
	new_domains[domain_index] = to;

	return StructureRelation(new_domains, *new_rel);
}

/// Reduces domain by projecting remaining variables	
/**
 * @param domain_index Index of domain to reduce
 * @param to           Domain to reduce to
 * 
 * @return The relation with reduced domain
 */


StructureRelation StructureRelation::reduce_domain(unsigned int domain_index,
				 const Domain& to) const
{
	Domain from = get_domain(domain_index);

	Domain::const_iterator from_i = from.begin();
	Domain::const_iterator to_i = to.begin();

	while(to_i != to.end())
	{
		assert(from_i != from.end());
		assert(*from_i == *to_i);

		++from_i;
		++to_i;
	}

	Domain remaining;
	while(from_i != from.end())
	{
		remaining |= Domain(*from_i);
		++from_i;
	}

	auto_ptr<StructureConstraint> new_rel(get_bdd_based().ptr_project(remaining));

	Domains new_domains = get_domains();
	new_domains[domain_index] = to;

	return StructureRelation(new_domains, *new_rel);
}

/// Escape relation from a domain
/**
 * Tries to change the domains of a relation such that it does not interfer with another domain
 *
 * @param r Relation to escape
 * @param d Domain to escape from
 * @param d_new New domain to use instead of \a d
 * 
 * @return The escaped relation such that its domains are disjoint from \a d_new
 */

static StructureRelation escape_from_domain(const StructureRelation& r, const Domain& d, Domain& d_new)
{
	d_new = d;

	if (r.get_domains().is_disjoint(d)) return r;

	if (d.is_infinite() || r.get_domains().is_some_infinite())
	{
		Domains doms = r.get_domains();
		for (unsigned int i = 0;i < doms.size();++i)
		{
			doms[i] = doms[i] * 2;
		}

		d_new = d * 2 + 1;

		return StructureRelation(doms, r);
	}

	// All domains are finite from here

	Domains new_doms = r.get_domains();

	// For domains that overlap with d, allocate from unused, i.e. not from d or new_doms
	StructureConstraint::VarPool pool;
	pool.alloc(d);
	pool.alloc(new_doms.union_all());

	for (unsigned int i = 0;i < new_doms.size();++i)
	{
		if (!new_doms[i].is_disjoint(d))
		{
			// We have overlap, escape

			new_doms[i] = pool.alloc(new_doms[i].size());
		}
	}

	return StructureRelation(new_doms, r);
}

/// Composes relation with another relation
/**
 * @param compose_domain_index Index of domain to compose with
 * @param compose_rel Binary relation to compose with
 *
 * @return The relation obtained by applying \a compose_rel in the
 *          component of this relation given by \a compose_domain_index.
 */

StructureRelation StructureRelation::compose(unsigned int compose_domain_index, 
					   const StructureRelation& compose_rel) const
{
	assert(compose_rel.arity() == 2);
	assert(get_domain(compose_domain_index).is_compatible(compose_rel.get_domain(0)));

	Domain dom_im;
	StructureRelation escaped_rel = escape_from_domain(*this, compose_rel.get_domain(1), dom_im);

	Domain dom_range = escaped_rel.get_domain(compose_domain_index);

	StructureRelation escaped_compose_rel (dom_range * dom_im, compose_rel);
	
	Domains doms_result = escaped_rel.get_domains();
	doms_result[compose_domain_index] = dom_im;

	auto_ptr<StructureConstraint> combined (escaped_rel.get_bdd_based().ptr_product(escaped_compose_rel.get_bdd_based(), StructureConstraint::fn_and));
	auto_ptr<StructureConstraint> projected (combined->ptr_project(dom_range));

	return StructureRelation(doms_result, *projected);
}

/// Obtain new relation using cross product of sets
/**
 * The domain of set in \a contents must have the same number of
 * variables as the corresponding domain in \a domains.
 *
 * @param domains The domains of the new relation
 * @param contents The sets to take cross product of
 * 
 * @return A new relation having the domains \a domains obtained by
 * taking the cross product of all sets in \a contents.
 *
 */

StructureRelation StructureRelation::cross_product(const Domains& domains,
						 const vector<StructureSet>& contents)
{
	assert(contents.size() != 0);
	assert(domains.size() == contents.size());

	vector<StructureSet>::const_iterator i = contents.begin();
	Domains::const_iterator domains_i = domains.begin();

	StructureSet first_set = StructureSet(domains[0], contents[0]);

	auto_ptr<StructureConstraint> new_rel(first_set.get_bdd_based().ptr_clone());

	++i;
	++domains_i;

	while (i != contents.end())
	{
		assert(domains_i != domains.end());
		
		auto_ptr<StructureConstraint> product(new_rel->ptr_product(StructureSet(*domains_i, *i).get_bdd_based(),
								StructureConstraint::fn_and));

		new_rel = product;

		++i;
		++domains_i;
	}
	
	return StructureRelation(domains, *new_rel);
}

/// AND product
/**
 * 
 *
 * @param rel1 Relation one
 * @param rel2 Relation two
 * 
 * @return AND product of \a rel1 and \a rel2
 */


StructureRelation operator&(const StructureRelation& rel1, const StructureRelation& rel2)
{
	return rel1.product(rel2, StructureConstraint::fn_and);
}

/// OR product
/**
 * 
 *
 * @param rel1 Relation one
 * @param rel2 Relation two
 * 
 * @return OR product of \a rel1 and \a rel2
 */


StructureRelation operator|(const StructureRelation& rel1, const StructureRelation& rel2)
{
	return rel1.product(rel2, StructureConstraint::fn_or);
}

/// MINUS product
/**
 * 
 *
 * @param rel1 Relation one
 * @param rel2 Relation two
 * 
 * @return MINUS product of \a rel1 and \a rel2
 */


StructureRelation operator-(const StructureRelation& rel1, const StructureRelation& rel2)
{
	return rel1.product(rel2, StructureConstraint::fn_minus);
}

/// AND product
/**
 * Assigns this relation to the AND product of this relation and \a rel2
 *
 * @param rel2 Relation two
 * 
 * @return This relation
 */


StructureRelation& StructureRelation::operator&=(const StructureRelation& rel2)
{
	return (*this = *this & rel2);
}

/// OR product
/**
 * Assigns this relation to the OR product of this relation and \a rel2
 *
 * @param rel2 Relation two
 * 
 * @return This relation
 */


StructureRelation& StructureRelation::operator|=(const StructureRelation& rel2)
{
	return (*this = *this | rel2);
}

/// MINUS product
/**
 * Assigns this relation to the MINUS product of this relation and \a rel2
 *
 * @param rel2 Relation two
 * 
 * @return This relation
 */

StructureRelation& StructureRelation::operator-=(const StructureRelation& rel2)
{
	return (*this = *this - rel2);
}

/// Negation
/**
 * The universe in the negation is considered to be {0..2^n-1} for each domain
 * 
 * @return The negation of this relation
 */

StructureRelation StructureRelation::operator!() const
{
	auto_ptr<StructureConstraint> res(get_bdd_based().ptr_negate());
	return StructureRelation(get_domains(), *res);
}

/// Equality
/**
 * Checks equality of two relations. This may involve renaming of one of the relations
 *
 * @param rel2 Relation to compare with
 * 
 * @return Whether this relation is equal to \a rel2, interpreted over respective domains
 */

bool StructureRelation::operator==(const StructureRelation& rel2) const
{
	const StructureRelation& rel1 = *this;

	Domains res_domains = Domains::sup(rel1.get_domains(), rel2.get_domains());
	
	return StructureRelation(res_domains, rel1).get_bdd_based() ==
		StructureRelation(res_domains, rel2).get_bdd_based();
}


/// Product
/**
 * General product function
 *
 * @param r2 Relation to take product with
 * @param fn Product function
 * 
 * @return Product relation R, such that R(x) iff fn(R1(x),R2(x)).
 */


StructureRelation 
StructureRelation::product(const StructureRelation& r2, bool (*fn)(bool v1, bool v2)) const
{
	const StructureRelation& r1 = *this;

	Domains res_domains = Domains::sup(r1.get_domains(), r2.get_domains());

	StructureRelation renamed_r1(res_domains, r1);
	StructureRelation renamed_r2(res_domains, r2);

	auto_ptr<StructureConstraint> res(renamed_r1.get_bdd_based().ptr_product(renamed_r2.get_bdd_based(), fn));

	return StructureRelation(res_domains, *res);
}

// Projects on a component
/**
 * @param domain_index Domain to project on
 * 
 * @return The projected component of this relation
 */

StructureSet StructureRelation::project_on(unsigned int domain_index) const
{
	Domain dom_project;

	Domains::const_iterator i = domains->begin();
	unsigned int index = 0;

	while (i != domains->end())
	{
		if (index != domain_index)
		{
			if(i->is_finite())
			{
				dom_project |= *i;
			}
			else
			{
				dom_project |= (*i & Domain(0, get_bdd_based().highest_var() + 1));
			}
		}

		index++;
		i++;
	}

	auto_ptr<StructureConstraint> res(get_bdd_based().ptr_project(dom_project));

	return StructureSet(get_domain(domain_index), *res);
}

/// Projects away one component
/**
 * @param domain_index Domain to project
 * 
 * Projects away one component
 * 
 * @return A relation where the domain with index \a domain_index is projected away
 */

StructureRelation StructureRelation::project(unsigned int domain_index) const
{
	Domain dom_not_project;

	Domains::const_iterator i = domains->begin();
	unsigned int index = 0;

	while (i != domains->end())
	{
		if (index != domain_index)
		{
			if(i->is_finite())
			{
				dom_not_project |= *i;
			}
			else
			{
				dom_not_project |= (*i & Domain(0, get_bdd_based().highest_var() + 1));
			}
		}

		index++;
		i++;
	}

	auto_ptr<StructureConstraint> res(get_bdd_based().ptr_project(get_domain(domain_index)));

	return StructureRelation(get_domains(), *res);
}


/// Restricts relation
/**
 * @param domain_index Domain to restrict
 * @param to StructureSet to restrict to
 *
 * @return The relation restricted to \a to in the domain denoted by
 *          \a domain_index
 */

StructureRelation StructureRelation::restrict(unsigned int domain_index, const StructureSet& to) const
{
	StructureSet adapted(get_domain(domain_index), to);

	auto_ptr<StructureConstraint> res(adapted.get_bdd_based().ptr_product(get_bdd_based(), StructureConstraint::fn_and));

	return StructureRelation(get_domains(), *res);
}
/// Copy Constructor
/**
 * @param r  Relation to copy from
 */
StructureSet::StructureSet(const StructureRelation& r) : ViewT(r) {}

/// Uninitialized constructor
StructureSet::StructureSet() : ViewT() {}

/// Constructor
/**
 * @param domain Domain of set
 * @param bb Bdd based object representing set
 */

StructureSet::StructureSet(const Domain& domain, const StructureConstraint& bb) : ViewT(domain, bb) {}

/// Copy constructor with automatic renaming
/**
 * @param domain Domain of new set
 * @param s Set to adapt to new domain
 */
StructureSet::StructureSet(const Domain& domain, const StructureSet& s) : ViewT(domain, s) {}

/// Destructor
StructureSet::~StructureSet() {}

}

