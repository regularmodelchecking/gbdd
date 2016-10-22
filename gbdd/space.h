/*
 * space.h: 
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

#ifndef GBDD_SPACE_H
#define GBDD_SPACE_H

#include <gbdd/sgi_ext.h>
#include <gbdd/bool-constraint.h>
#include <vector>
#include <functional>
#include <set>
#include <string>
#include <assert.h>

namespace gbdd
{

using namespace std;

/// An abstract interface to BDD implementations
class Space
{
public:
	
/**
 * A variable in a BDD
 * 
 */
	typedef Domain::Var Var;
	
/**
 * An identity of a BDD
 * 
 */
	typedef unsigned long int Bdd;

/**
 * Space error 
 * 
 */
	class Error
	{
		string desc;
	public:
/// Constructor
/**
 * @param desc Description of error
 */
		Error(string desc) : 
			desc(desc)
			{}

/// Get description
/**
 * @return Description of error
 */
		virtual string description() const;
	};

	/// Unary function with virtual implementation
	template <class Arg, class Result>
	class UnaryFunction : public unary_function<Arg, Result>
	{
	public:
/// Apply function
/**
 * @param x Argument
 * 
 * @return Result of applying function to \a x
 */
		virtual Result operator()(Arg x) = 0;
	};

	/// Binary function with virtual implementation
	template <class Arg1, class Arg2, class Result>
	class BinaryFunction : public binary_function<Arg1, Arg2, Result>
	{
	public:
/// Apply function
/**
 * 
 *
 * @param x First argument
 * @param y Second argument
 * 
 * @return Result of applying function to (\a x,\a y)
 */
		virtual Result operator()(Arg1 x, Arg2 y) = 0;
	};

private:
	template <class _UnaryFunction, class Closure>
	class ClosureUnaryFunction : public _UnaryFunction
	{
		Closure f;
	public:
		virtual ~ClosureUnaryFunction() {};
		ClosureUnaryFunction(Closure f):f(f) {}

		typename _UnaryFunction::result_type operator()(
			typename _UnaryFunction::argument_type x) 
		{ 
			return f(x); 
		}
	};

	template <class _BinaryFunction, class Closure>
	class ClosureBinaryFunction : public _BinaryFunction
	{
		Closure f;
	public:
		ClosureBinaryFunction(Closure f):f(f) {}

		typename _BinaryFunction::result_type operator()(
			typename _BinaryFunction::first_argument_type x,
			typename _BinaryFunction::second_argument_type y) 
		{
			return f(x,y);
		}
	};

	Var bdd_highest_var(Bdd p, hash_set<Bdd>& cache);
public:
	typedef BinaryFunction<bool, bool, bool> ProductFunction;
	typedef UnaryFunction<bool, bool> UnaryProductFunction;
	typedef UnaryFunction<Var, bool> VarPredicate;
	typedef Domain::VarMap VarMap;
public:
	// Destructor
	virtual ~Space() {}

	static Space* create_default(bool diagnostics = false);

/// Garbage collect space
	virtual void gc() = 0;
/// Prevent garbage collection
	virtual void lock_gc();
/// Unprevent garbate collection
	virtual void unlock_gc();

/// Reference BDD
/**
 * Increases reference count of \a p
 *
 * @param p Bdd to reference
 */
	virtual void bdd_ref(Bdd p) = 0;

/// Unreference BDD
/**
 * Decreases reference count of \a p
 *
 * @param p Bdd to unreference
 */
	virtual void bdd_unref(Bdd p) = 0;

/// Test for leaf
/**
 * @param p Bdd to test
 * 
 * @return Whether \a p is a leaf
 */
	virtual bool bdd_is_leaf(Bdd p) = 0;
       
/// Get leaf value
/**
 * @param p Bdd to get leaf of
 * 
 * @return The value of \a p
 */
	virtual bool bdd_leaf_value(Bdd p) = 0;

/// Get then-branch
/**
 * @param p Bdd to get then-branch of
 * 
 * @return Then-branch of \a p
 */
	virtual Bdd bdd_then(Bdd p) = 0;

/// Get else-branch
/**
 * @param p Bdd to get else-branch of
 * 
 * @return Else-branch of \a p
 */
	virtual Bdd bdd_else(Bdd p) = 0;

/// Get variable of internal node
/**
 * @param p Bdd to get variable of
 * 
 * @return Variable of internal node \a p
 */
	virtual Var bdd_var(Bdd p) = 0;

/// Construct leaf node
/**
 * @param v Value of leaf node
 * 
 * @return The leaf Bdd with value \a v
 */
	virtual Bdd bdd_leaf(bool v) = 0;

/// Construct true node
/**
 * @return The leaf Bdd with value true
 */
	Bdd bdd_true() { return bdd_leaf(true); }

/// Construct false node
/**
 * @return The leaf Bdd with value false
 */
	Bdd bdd_false() { return bdd_leaf(false); }


/// Construct Bdd node for a variable being true
/**
 * @param v Variable of node
 * 
 * @return A Bdd representing that \a v is true
 */
	virtual Bdd bdd_var_true(Var v) = 0;

/// Construct Bdd node for a variable being false
/**
 * @param v Variable of node
 * 
 * @return A Bdd representing that \a v is false
 */
	virtual Bdd bdd_var_false(Var v) = 0;

/// Construct an internal node
/**
 * @param v Variable of internal node
 * @param p_then Then-branch of node
 * @param p_else Else-branch of node
 * 
 * @return The BDD representing (\a v => \a p_then ) AND ( \a v => \a p_else )
 */
	virtual Bdd bdd_var_then_else(Var v, Bdd p_then, Bdd p_else) = 0;

	virtual Var bdd_highest_var(Bdd p);

/// Project BDD
/**
 * @param p BDD to project
 * @param fn_var Predicate describing variables to project
 * @param fn_prod Product function
 * 
 * @return The projected BDD on variables v with \a fn_var (v) using product function \a fn_prod
 */
	virtual Bdd bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod) = 0;

/// Rename BDD
/**
 * @param p BDD to project
 * @param fn Renaming map
 * 
 * @return The renamed BDD such that variables v are renamed to fn(v)
 */
	virtual Bdd bdd_rename(Bdd p, const VarMap& fn) = 0;

/// BDD product
/**
 * @param p First BDD
 * @param q Second BDD
 * @param fn Product function
 * 
 * @return The BDD representing fn(p, q)
 */
	virtual Bdd bdd_product(Bdd p, Bdd q, ProductFunction& fn) = 0;

/// BDD unary product
/**
 * @param p BDD
 * @param fn Product function
 * 
 * @return The BDD representing fn(p)
 */
	virtual Bdd bdd_product(Bdd p, UnaryProductFunction& fn) = 0;

	template <class _VarPredicate, class _ProductFunction>
	Bdd bdd_project(Bdd p, _VarPredicate fn_var, _ProductFunction fn_prod);

	template <class _ProductFunction>
	Bdd bdd_product(Bdd p, Bdd q, _ProductFunction fn);

	template <class _UnaryProductFunction>
	Bdd bdd_product(Bdd p, _UnaryProductFunction fn);

	virtual void bdd_print(ostream &os, Bdd p) = 0;

/// Get number of nodes in space
/**
 * To be sure not to get dead nodes in the count, you have to lock the space and do a garbage collection before calling this
 *
 * @return Number of nodes 
 */
	virtual unsigned int get_n_nodes(void) const;
		
};

/// Project BDD
/**
 * @param p BDD to project
 * @param fn_var Predicate describing variables to project
 * @param fn_prod Product function
 * 
 * @return The projected BDD on variables v with \a fn_var (v) using product function \a fn_prod
 */
template <class _VarPredicate, class _ProductFunction>
Space::Bdd Space::bdd_project(Bdd p, _VarPredicate fn_var, _ProductFunction fn_prod)
{
	ClosureUnaryFunction<VarPredicate, _VarPredicate> cl_fn_var(fn_var);
	ClosureBinaryFunction<ProductFunction, _ProductFunction> cl_fn_prod(fn_prod);

	return bdd_project(p,
			   (VarPredicate&)cl_fn_var,
			   (ProductFunction&)cl_fn_prod);
}

/// BDD product
/**
 * @param p First BDD
 * @param q Second BDD
 * @param fn Product function
 * 
 * @return The BDD representing fn(p, q)
 */
template <class _ProductFunction>
Space::Bdd Space::bdd_product(Bdd p, Bdd q, _ProductFunction fn)
{
	ClosureBinaryFunction<ProductFunction, _ProductFunction> cl_fn(fn);

	return bdd_product(p, q, (ProductFunction&)cl_fn);
}


/// BDD unary product
/**
 * @param p BDD
 * @param fn Product function
 * 
 * @return The BDD representing fn(p)
 */
template <class _UnaryProductFunction>
Space::Bdd Space::bdd_product(Bdd p, _UnaryProductFunction fn)
{
	ClosureUnaryFunction<UnaryProductFunction, _UnaryProductFunction>
		cl_fn(fn);

	return bdd_product(p, (UnaryProductFunction&)cl_fn);
}

}

#endif /* GBDD_SPACE_H */
