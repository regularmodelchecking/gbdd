/*
 * bdd.h: 
 *
 * Copyright (C) 2000 Marcus Nilsson (marcusn@docs.uu.se)
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
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#ifndef GBDD_BDD_H
#define GBDD_BDD_H

#include <gbdd/space.h>
#include <gbdd/domain.h>
#include <gbdd/bool-constraint.h>
#include <set>
#include <gbdd/sgi_ext.h>

namespace gbdd
{

/// A BDD in some space
/**
The following code illustrates the typical use of BDDs in gbdd. It creates
a domain \c vs containing the variables 0, 1, 2 and 3. Using this set of variables,
it encodes integer values using a binary encoding, creating two BDDs. The first
BDD \c p represents the set {2,3} and the second BDD \c q represents the set
{3,4}. The it tests whether p intersected with q gives the set {3}.

See also gbdd::Bdd::Vars, a simplified way to create BDDs.

\code
Space *space = Space::create_default();

Domain vs(0,4);

Bdd p = Bdd::value(space, vs, 2) | Bdd::value(space, vs, 3);

Bdd q = Bdd::value(space, vs, 3) | Bdd::value(space, vs, 4);

return (p & q) == Bdd::value(space, vs, 3);

\endcode
*/

class Bdd : public BoolConstraint
{
/**
 * Implementation space of this BDD
 * 
 */
	Space* space;

/*
 * BDD in space
 */
	Space::Bdd space_bdd;

	Bdd(Space* space, Space::Bdd bdd);
public:
/**
 * Variable in a BDD
 * 
 */
	typedef Space::Var Var;
/**
 * Mapping from variables to variables
 * 
 */
	typedef Space::VarMap VarMap;

	Bdd();
	Bdd(Space* space, bool v = false);
	~Bdd();
	Bdd(const Bdd &p);

/// Get space of BDD
/**
 * @return The implementation space of this BDD
 */
	Space* get_space() const
	{
		return space;
	}

	class Factory : public BoolConstraint::Factory
	{
		Space* space;
	public:
		~Factory();
		Factory(Space* space);
		
		Bdd* ptr_constant(bool v) const;
		Bdd* ptr_var(Var v, bool var_v) const ;
	};

	Factory* ptr_factory() const
	{
		return new Factory(get_space());
	}

	BoolConstraint* ptr_convert(const BoolConstraint::Factory& f) const;

	Bdd operator! () const;
	Bdd operator| (const Bdd& p2) const;
	Bdd operator& (const Bdd& p2) const;
	Bdd operator- (const Bdd& p2) const;
	
	Bdd& operator= (const Bdd &p);
	
	Bdd& operator|= (const Bdd &p);
	Bdd& operator&= (const Bdd &p);
	Bdd& operator-= (const Bdd &p);
	
	virtual bool operator==(const StructureConstraint& b2) const;
	friend bool operator==(const Bdd& p1, const Bdd& p2);

	bool bdd_is_leaf() const;
	bool bdd_leaf_value() const;

	Bdd bdd_then() const;
	Bdd bdd_else() const;
	Var bdd_var() const;

/// Product of two variables
/**
 * 
 *
 * @param space Space of returned BDD
 * @param v1 First variable
 * @param v2 Second variable
 * @param fn Product function
 * 
 * @return The BDD representing fn(v1,v2)
 */
	template <class Product>
	static Bdd var_product(Space *space, Var v1, Var v2, Product fn);
	

/// Equality of two variables
/**
 * 
 * @param space Space of returned BDD
 * @param v1 First variable
 * @param v2 Second variable
 * 
 * @return The BDD representing v1 == v2
 */
	static Bdd var_equal (Space* space, Var v1, Var v2)
	{
		return var_product(space, v1, v2, fn_iff);
	}

	static Bdd var_true (Space* space, Var v);
	static Bdd var_false (Space* space, Var v);

	static Bdd var_then_else(Space* space, Var v, Bdd p_then, Bdd p_else);

	static unsigned int n_vars_needed(unsigned int n_values);
	static Bdd value(Space* space, const Domain &vs, unsigned int v);
	static Bdd value_range(Space* space, const Domain& vs, unsigned int from_v, unsigned int to_v);
private:
	Bdd value_follow(Domain::const_iterator start, 
			 Domain::const_iterator end, 
			 unsigned int v) const;
public:
	bool value_member(const Domain& vs, unsigned int v) const;
	Bdd value_follow(const Domain& vs, unsigned int v) const;
	
	template <class Product>
	static Bdd vars_product(Space* space,
				const Domain &vs1,
				const Domain &vs2,
				Product fn);

	static Bdd vars_equal(Space* space, const Domain &vs1, const Domain &vs2);

	class FiniteVar
	{
		Space* _space;
		Domain _vs;
	public:
		FiniteVar(Space* space, const Domain& vs):
			_space(space),
			_vs(vs)
			{}

		Bdd operator==(unsigned int v) const { return value(_space, _vs, v); }
		Bdd operator==(const FiniteVar& fv2) const;

		const Domain& get_domain() const { return _vs; }
		Space* get_space() const { return _space; }
	};

	class FiniteVars
	{
		Space* _space;
		Domains _ds;
	public:
		FiniteVars(Space* space, const Domains& ds):
			_space(space),
			_ds(ds)
			{}

		FiniteVars(const FiniteVar& v):
			_space(v.get_space()),
			_ds(v.get_domain())
			{}

		FiniteVar operator[](size_t index) const { return FiniteVar(_space, _ds[index]); }
		const Domains& get_domains() const { return _ds; }
		Space* get_space() const { return _space; }
	};

	class BooleanVar : public FiniteVar
	{
	public:
		BooleanVar(Space* space, Var v):
			FiniteVar(space, Domain(v))
			{}

		operator Bdd() const { return static_cast<const FiniteVar&>(*this) == 1; }
		Bdd operator!() const { return static_cast<const FiniteVar&>(*this) == 0; }
		Bdd operator&(const BooleanVar& v2) const { return static_cast<Bdd>(*this) & v2; }
		Bdd operator|(const BooleanVar& v2) const { return static_cast<Bdd>(*this) | v2; }
	};

/**
 * Represents all variables in a BDD space. This class works as a factory for BDDs in the following way.
 * One instance of this class works as an array, which
 * can be indexed by different keys to obtain different kind of variables. The simplest is boolean
 * variables, which is obtained by using integers, so if x is an instance of gbdd::Vars, then
 * x[2] is the BDD variable with index 2. The expression x[2] & x[3] can then be used to generate
 * a BDD representing that variable 2 and 3 must be true.
 * 
 */
	class Vars
	{
		Space* _space;
	public:
		Vars(Space* space):
			_space(space)
			{}

		BooleanVar operator[](Var v) const { return BooleanVar(_space, v); }
		FiniteVar operator[](const Domain& vs) const { return FiniteVar(_space, vs); }
		FiniteVars operator[](const Domains& ds) const { return FiniteVars(_space, ds); }
	};

		

/// Product
/**
 * 
 *
 * @param fn Product function
 * 
 * @return Product of this BDD with respect to \a fn
 */
	template <class Product>
	Bdd product(Product fn) const;

/// Product
/**
 * 
 *
 * @param p1 First BDD
 * @param p2 Second BDD
 * @param fn Product function
 * 
 * @return Product of \a p1 and \a p2 with respect to \a fn
 */

	template<class Product>
	static Bdd bdd_product(const Bdd&p1, const Bdd& p2, Product fn);

/// Projection
/**
 * 
 *
 * @param fn_var Predicate describing variables to project
 * @param fn_prod Product function to use for projection
 * 
 * @return Projection of this BDD with \a fn_prod with respect to all variables v such that fn_var(v)
 */
	template <class VarPredicate, class Product>
	Bdd project(VarPredicate fn_var, Product fn_prod) const;

/// OR projection
/**
 * 
 *
 * @param fn_var Predicate describing variables to project
 * 
 * @return Projection of this BDD with OR with respect to all variables v such that fn_var(v)
 */
	template <class VarPredicate>
	Bdd project(VarPredicate fn_var) const
	{
		return project(fn_var, fn_or);
	}

/// Synonym for OR projection
/**
 * 
 *
 * @param fn_var Predicate describing variables to project
 * 
 * @return Projection of this BDD with OR with respect to all variables v such that fn_var(v)
 */
	template <class VarPredicate>
	Bdd exists(VarPredicate fn_var) const
	{
		return project(fn_var);
	}

/// Forall projection
/**
 * @param fn_var Predicate describing variables to project
 * 
 * @return !((!p).exists(fn_var)) for p as this BDD
 */
	template <class VarPredicate>
	Bdd forall(VarPredicate fn_var) const
	{
		return !((!*this).exists(fn_var));
	}

/// Rename according to map
/**
 * 
 *
 * @param map Map describing renaming
 * 
 * @return The BDD renamed with \a map
 */
	Bdd rename(const VarMap& map) const
	{
		space->lock_gc();

		Bdd res(space, space->bdd_rename(space_bdd, map));

		space->unlock_gc();

		return res;
	}

/// Renaming between variable sets
/**
 * 
 *
 * @param vs1 Current set of variables
 * @param vs2 New set of variables
 * 
 * @return The renamed BDD such that vs1[i] is renamed to vs2[i]
 */
	Bdd rename(const Domain& vs1, const Domain& vs2) const
	{
		return rename(Domain::map_vars(vs1, vs2));
	}

private:	
	unsigned int n_assignments(Domain::const_iterator first,
				   Domain::const_iterator last) const;
	void assignments_value(Domain::const_iterator first,
			       Domain::const_iterator last,
			       unsigned int current_base,
			       unsigned int current_v,
			       set<unsigned int>& result) const;
	
	void with_geq_var(Var v, hash_set<Bdd>& res) const;
	static Bdd with_image_geq_var(Space* space, Space::Bdd space_p, Space::Bdd space_im, Var v);
public:      
	unsigned int n_assignments(const Domain& vs) const;
	set<unsigned int> assignments_value(const Domain& vs) const;
	hash_set<Bdd> with_geq_var(Var v) const;
	Bdd with_image_geq_var(Bdd im, Var v) const;
	hash_set<Bdd> nodes() const;

	Var highest_var() const;
	Var lowest_var() const;

	Domain vars() const;

	bool is_false() const;
	bool is_true() const;

	friend struct hash<Bdd>;

/// Hash value of this BDD
/**
 * @return Hash value for BDD
 */
	unsigned long int hash(void) const;
	
	friend ostream& operator<< (ostream& s, const Bdd& p);

	ostream& print_dot(ostream& os) const;
	
	static void gc(Space* space);

	virtual Bdd* ptr_rename(VarMap map) const;

	virtual Bdd* ptr_project(Domain vs) const;
	virtual Bdd* ptr_constrain_value(Var v, bool value) const;
	
	virtual Bdd* ptr_product(const StructureConstraint& b2, bool (*fn)(bool v1, bool v2)) const;
	virtual Bdd* ptr_negate() const;

	virtual Bdd* ptr_clone() const;

};

template <class Product>
Bdd Bdd::product(Product fn) const
{
	space->lock_gc();

	Bdd res(space, space->bdd_product(space_bdd, fn));

	space->unlock_gc();

	return res;
}

template<class Product>
Bdd Bdd::bdd_product(const Bdd& p1, const Bdd& p2, Product fn)
{
	p1.space->lock_gc();

	Bdd res(p1.space, p1.space->bdd_product(p1.space_bdd, p2.space_bdd, fn));

	p1.space->unlock_gc();

	return res;
}

template <class VarPredicate, class Product>
Bdd Bdd::project(VarPredicate fn_var, Product fn_prod) const
{
	space->lock_gc();

	Bdd res(space, space->bdd_project(space_bdd, fn_var, fn_prod));

	space->unlock_gc();

	return res;
}


template <class Product>
Bdd Bdd::var_product (Space* space, Var v1, Var v2, Product fn)
{
	return bdd_product(var_true(space, v1), var_true(space, v2), fn);
}

/**
 * vars_product:
 * @param space Space of BDD returned
 * @param vs1 First set of variables
 * @param vs2 Second set of variables
 * @param fn Product function
 *
 * Product of several variables
 * 
 * Returns: The BDD representing product between variables in \a vs1
 *          and \a vs2. The i'th variable in \a vs1 if related with \a fn to
 *          the i'th variable in \a vs2.
 */
	
template <class Product>
Bdd Bdd::vars_product(Space *space,
		      const Domain &vs1,
		      const Domain &vs2,
		      Product fn)
{
	Bdd p(space, true);

	Domain::const_iterator i1 = vs1.begin();
	Domain::const_iterator i2 = vs2.begin();

	while(i1 != vs1.end())
	{
		assert(i2 != vs2.end());

		p &= var_product(space, *i1, *i2, fn);

		++i1;
		++i2;
	}
	
	return p;
}

static Bdd::FiniteVars operator*(const Bdd::FiniteVars& vs1, const Bdd::FiniteVars& vs2)
{
	return Bdd::FiniteVars(vs1.get_space(), vs1.get_domains() * vs2.get_domains());
}

}

DECL_NAMESPACE_SGI
{

/// Hash function for gbdd::Bdd
struct hash<gbdd::Bdd>
{
	size_t operator()(gbdd::Bdd p) const;
};

}

#endif /* GBDD_BDD_H */
