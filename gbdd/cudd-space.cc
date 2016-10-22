/*
 * cudd-space.cc: 
 *
 * Copyright (C) 2003 Marcus Nilsson (marcusn@docs.uu.se)
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
 *    Marcus Nilsson (marcusn@it.uu.se)
 */

#include <gbdd/cudd-space.h>
#include <string>
#include <iostream>

#ifdef GBDD_WITH_CUDD
#include <cudd.h>

namespace gbdd
{


CuddSpace::CuddSpace()
{
	max_vars = 2048;
	manager = Cudd_Init(max_vars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
}



CuddSpace::~CuddSpace()
{
	Cudd_Quit(manager);
}

unsigned int CuddSpace::get_n_nodes(void) const
{
	return 0;
}

void CuddSpace::ensure_n_vars(unsigned int n_vars)
{
	if (max_vars >= n_vars) return;

	assert(false);
}


void CuddSpace::gc()
{
	return;
}

void CuddSpace::bdd_ref(Bdd p)
{
	Cudd_Ref((DdNode*)p);
}

void CuddSpace::bdd_unref(Bdd p)
{
	Cudd_RecursiveDeref(manager, (DdNode*)p);
}

bool CuddSpace::bdd_is_leaf(Bdd p)
{
	return Cudd_IsConstant((DdNode*)p);
}

bool CuddSpace::bdd_leaf_value(Bdd p)
{
	assert(Cudd_IsConstant((DdNode*)p));
	return !Cudd_IsComplement((DdNode*)p);
}

Space::Bdd CuddSpace::bdd_then(Bdd p)
{
	return (Bdd)Cudd_NotCond(Cudd_T((DdNode*)p),
				 Cudd_IsComplement((DdNode*)p));
}

Space::Bdd CuddSpace::bdd_else(Bdd p)
{
	return (Bdd)Cudd_NotCond(Cudd_E((DdNode*)p),
				 Cudd_IsComplement((DdNode*)p));
}

Space::Var CuddSpace::bdd_var(Bdd p)
{
	return (Var)Cudd_NodeReadIndex((DdNode*)p);
}

Space::Bdd CuddSpace::bdd_leaf(bool v)
{
	return v ? 
		(Bdd)Cudd_ReadOne(manager):
		(Bdd)Cudd_ReadLogicZero(manager);
}

Space::Bdd CuddSpace::bdd_var_true(Var v)
{
	ensure_n_vars(v+1);

	return (Bdd)Cudd_bddIthVar(manager, v);
}

Space::Bdd CuddSpace::bdd_var_false(Var v)
{
	ensure_n_vars(v+1);

	return (Bdd)Cudd_Not(Cudd_bddIthVar(manager, v));
}

Space::Bdd CuddSpace::bdd_var_then_else(Var v, Bdd p_then, Bdd p_else)
{
	ensure_n_vars(v+1);

	return (Bdd)Cudd_bddIte(manager, Cudd_bddIthVar(manager, v), (DdNode*)p_then, (DdNode*)p_else);
}

#define max(a, b) ((a > b) ? a : b)

Space::Var CuddSpace::bdd_highest_var(Bdd p, hash_set<Bdd>& cache)
{
	if (cache.find(p) != cache.end()) return 0;
	cache.insert(p);

	Bdd p_then = bdd_then(p);
	Bdd p_else = bdd_else(p);

	switch(bdd_is_leaf(p_then) | (bdd_is_leaf(p_else) << 1))
	{
	case 0:
	{
		Var then_highest = bdd_highest_var(p_then, cache);
		Var else_highest = bdd_highest_var(p_else, cache);

		return max(then_highest, else_highest);
	}
	case 1:
		/* p_then is a leaf, p_else is not */
		return bdd_highest_var(p_else, cache);
	case 2:
		/* p_else is a leaf, p_then is not */
		return bdd_highest_var(p_then, cache);
	case 3:
		/* Both p_then and p_else are leaves */
		return bdd_var(p);
	}
	assert(false);
}

Space::Var CuddSpace::bdd_highest_var(Bdd p)
{
	if (bdd_is_leaf(p)) return 0;

	hash_set<Space::Bdd> cache;

	return bdd_highest_var(p, cache);
}

static bool fn_is_or(Space::ProductFunction& fn)
{
	return fn(true, true) &&
		fn(true, false) &&
		fn(false, true) &&
		fn(true, true);
}

Space::Bdd CuddSpace::varpredicate_to_set(unsigned int n_vars, Space::VarPredicate& fn_var)
{
	Space::Bdd p = bdd_leaf(true);

	int i;
	for (i = n_vars-1;i >= 0;--i)
	{
		if(fn_var(i))
		{
			Space::Bdd old_p = p;

			bdd_ref(old_p);

			p = (Bdd)Cudd_bddAnd(manager, (DdNode*)p, (DdNode*)bdd_var_true(i));
			
			bdd_unref(old_p);
		}
	}

	bdd_ref(p);

	return p;
}


Space::Bdd CuddSpace::bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod)
{
	assert(fn_is_or(fn_prod));

	DdNode* set = (DdNode*)varpredicate_to_set(max_vars, fn_var);

	Bdd res = (Bdd)Cudd_bddExistAbstract(manager, (DdNode*)p, set);

	bdd_unref((Bdd)set);

	return res;
}

Space::Bdd CuddSpace::bdd_rename(Bdd p, const VarMap& fn)
{
	DdNode* X[max_vars];
	DdNode* Y[max_vars];

	VarMap::const_iterator i;
	unsigned int index = 0;
	for (i = fn.begin();i != fn.end();++i)
	{
		ensure_n_vars(i->second + 1);

		if (i->first != i->second)
		{
			X[index] = Cudd_bddIthVar(manager, i->first);
			Y[index] = Cudd_bddIthVar(manager, i->second);

			index++;
		}
	}

	return (Bdd)Cudd_bddSwapVariables(manager, (DdNode*)p, X, Y, index);
}

typedef DdNode* (*OpFunction)(DdManager*, DdNode*, DdNode*);

static DdNode* op_diff(DdManager* manager, DdNode* x, DdNode* y)
{
	return Cudd_bddAnd(manager, x, Cudd_Not(y));
}

static DdNode* op_imp(DdManager* manager, DdNode* x, DdNode* y)
{
	return Cudd_bddOr(manager, Cudd_Not(x), y);
}

static DdNode* op_biimp(DdManager* manager, DdNode* x, DdNode* y)
{
	return Cudd_Not(Cudd_bddXor(manager, x, y));
}



static OpFunction op_table[] =
{
	/* (true, true) (true, false) (false, true) (false, false) */
	NULL, /* 0000 */
        NULL, /* 0001 */
	NULL, /* 0010 */
	NULL, /* 0011 */
	op_diff, /* 0100 */
	NULL, /* 0101 */
	NULL, /* 0110 */
	NULL, /* 0111 */
	Cudd_bddAnd, /* 1000 */
	op_biimp, /* 1001 */
	NULL, /* 1010 */
	op_imp, /* 1011 */
	NULL, /* 1100 */
	NULL, /* 1101 */
	Cudd_bddOr, /* 1110 */
	NULL, /* 1111 */
};

static OpFunction fn_to_op(Space::ProductFunction& fn)
{
	unsigned int index = 0;

	if (fn(true, true)) index |= 0x08;
	if (fn(true, false)) index |= 0x04;
	if (fn(false, true)) index |= 0x02;
	if (fn(false, false)) index |= 0x01;

	return op_table[index];
}

Space::Bdd CuddSpace::bdd_product(Bdd p, Bdd q, ProductFunction& fn)
{
	OpFunction op = fn_to_op(fn);

	assert(op != NULL);

	return (Bdd)op(manager, (DdNode*)p, (DdNode*)q);
}

Space::Bdd CuddSpace::bdd_product(Bdd p, UnaryProductFunction& fn)
{
	if (fn(true) && fn(false)) return bdd_leaf(true);
	if (!fn(true) && !fn(false)) return bdd_leaf(true);

	if (fn(true) && !fn(false)) return p;

	return (Bdd)Cudd_Not(p);
}

void CuddSpace::bdd_print(ostream &os, Bdd p)
{
  if (bdd_is_leaf(p))
    {
	    os << bdd_leaf_value(p);
    }
  else
    {
	    Var index = bdd_var(p);
	    
	    os << "(v" << index << ": ";

	    bdd_print(os, bdd_then(p));
	    os << "|";
	    bdd_print(os, bdd_else(p));

	    os << ")";
    }
}
	
}

#endif /* GBDD_WITH_CUDD */

