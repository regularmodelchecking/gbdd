/*
 * buddy-space.cc: 
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

#include <gbdd/buddy-space.h>
#include <string>

#ifdef GBDD_WITH_BUDDY
#include <buddy.h>

#undef bdd_ithvar
#undef bdd_nithvar

namespace gbdd
{

static void errhandler(int e)
{
   fprintf(stderr, "BDD error: %s\n", bdd_errstring(e));
   throw Space::Error(bdd_errstring(e)); 
}

BuddySpace::BuddySpace(unsigned int initial_n_nodes, unsigned int cache_size)
{
	max_vars = 1;

	::bdd_init(initial_n_nodes, cache_size);
	::bdd_setvarnum(max_vars);
	::bdd_error_hook(errhandler);
}



BuddySpace::~BuddySpace()
{
	::bdd_done();
}

unsigned int BuddySpace::get_n_nodes(void) const
{
	return bdd_getnodenum();
}

void BuddySpace::ensure_n_vars(unsigned int n_vars)
{
	if (max_vars >= n_vars) return;

	max_vars = n_vars;
	::bdd_setvarnum(max_vars);
}


void BuddySpace::gc()
{
	::bdd_gbc();
}

void BuddySpace::bdd_ref(Bdd p)
{
	(void)::bdd_addref(p);
}

void BuddySpace::bdd_unref(Bdd p)
{
	(void)::bdd_delref(p);
}

bool BuddySpace::bdd_is_leaf(Bdd p)
{
	return p == bdd_true() || p == bdd_false();
}

bool BuddySpace::bdd_leaf_value(Bdd p)
{
	return (p == bdd_true());
}

Space::Bdd BuddySpace::bdd_then(Bdd p)
{
	return ::bdd_high(p);
}

Space::Bdd BuddySpace::bdd_else(Bdd p)
{
	return ::bdd_low(p);
}

Space::Var BuddySpace::bdd_var(Bdd p)
{
	return ::bdd_var(p);
}

Space::Bdd BuddySpace::bdd_leaf(bool v)
{
	return v ? ::bddtrue.id() : ::bddfalse.id();
}

Space::Bdd BuddySpace::bdd_var_true(Var v)
{
	ensure_n_vars(v+1);

	return bdd_ithvar(v);
}

Space::Bdd BuddySpace::bdd_var_false(Var v)
{
	ensure_n_vars(v+1);

	return bdd_nithvar(v);
}

Space::Bdd BuddySpace::bdd_var_then_else(Var v, Bdd p_then, Bdd p_else)
{
	ensure_n_vars(v+1);

	return ::bdd_ite(bdd_var_true(v), p_then, p_else);
}

#define max(a, b) ((a > b) ? a : b)

Space::Var BuddySpace::bdd_highest_var(Bdd p, hash_set<Bdd>& cache)
{
	if (cache.find(p) != cache.end()) return 0;
	cache.insert(p);

	Bdd p_then = ::bdd_high(p);
	Bdd p_else = ::bdd_low(p);

	switch((p_then < 2) | ((p_else < 2) << 1))
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
		return ::bdd_var(p);
	}
	assert(false);
}

Space::Var BuddySpace::bdd_highest_var(Bdd p)
{
	if (p < 2) return 0;

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

static Space::Bdd varpredicate_to_set(unsigned int n_vars, Space::VarPredicate& fn_var)
{
	Space::Bdd p = bddtrue.id();

	int i;
	for (i = n_vars-1;i >= 0;--i)
	{
		if(fn_var(i))
		{
			Space::Bdd old_p = p;

			bdd_addref(old_p);

			p = bdd_and(p, bdd_ithvar(i));
			
			bdd_delref(old_p);
		}
	}

	bdd_addref(p);

	return p;
}


Space::Bdd BuddySpace::bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod)
{
	assert(fn_is_or(fn_prod));

	Bdd set = varpredicate_to_set(max_vars, fn_var);

	Bdd res = bdd_exist(p, set);

	bdd_delref(set);

	return res;
}

Space::Bdd BuddySpace::bdd_rename(Bdd p, const VarMap& fn)
{
	bddPair* pair = bdd_newpair();

	VarMap::const_iterator i;
	for (i = fn.begin();i != fn.end();++i)
	{
		ensure_n_vars(i->second + 1);

		if (i->first != i->second)
			bdd_setpair(pair, i->first, i->second);
	}

	Bdd res = bdd_replace(p, pair);

	bdd_freepair(pair);

	return res;
}

static int op_table[] =
{
	/* (true, true) (true, false) (false, true) (false, false) */
	-1, /* 0000 */
        -1, /* 0001 */
	-1, /* 0010 */
	-1, /* 0011 */
	bddop_diff, /* 0100 */
	-1, /* 0101 */
	-1, /* 0110 */
	-1, /* 0111 */
	bddop_and, /* 1000 */
	bddop_biimp, /* 1001 */
	-1, /* 1010 */
	bddop_imp, /* 1011 */
	-1, /* 1100 */
	-1, /* 1101 */
	bddop_or, /* 1110 */
	-1, /* 1111 */
};

static int fn_to_op(Space::ProductFunction& fn)
{
	unsigned int index = 0;

	if (fn(true, true)) index |= 0x08;
	if (fn(true, false)) index |= 0x04;
	if (fn(false, true)) index |= 0x02;
	if (fn(false, false)) index |= 0x01;

	return op_table[index];
}

Space::Bdd BuddySpace::bdd_product(Bdd p, Bdd q, ProductFunction& fn)
{
	int op = fn_to_op(fn);

	assert(op != -1);

	return bdd_apply(p, q, op);
}

Space::Bdd BuddySpace::bdd_product(Bdd p, UnaryProductFunction& fn)
{
	if (fn(true) && fn(false)) return bdd_true();
	if (!fn(true) && !fn(false)) return bdd_false();

	if (fn(true) && !fn(false)) return p;

	return bdd_not(p);
}

void BuddySpace::bdd_print(ostream &os, Bdd p)
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

#endif /* GBDD_WITH_BUDDY */
