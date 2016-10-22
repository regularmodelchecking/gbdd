/*
 * cudd-space.h: 
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
#ifndef CUDD_SPACE_H
#define CUDD_SPACE_H

#include <gbdd/config.h>

#ifdef GBDD_WITH_CUDD
#include <gbdd/space.h>
#include <stdio.h>

extern "C" {
#include <cudd.h>
}

namespace gbdd
{
	/// Wrapper for the BuDDy implementation of BDDs
	class CuddSpace : public Space
	{
		unsigned int max_vars;

		DdManager* manager;

		void ensure_n_vars(unsigned int n_vars);
		Var bdd_highest_var(Bdd p, hash_set<Bdd>& cache);
		Bdd varpredicate_to_set(unsigned int n_vars, Space::VarPredicate& fn_var);
	public:
		CuddSpace();
		virtual ~CuddSpace();

		void gc();

		void bdd_ref(Bdd p);
		void bdd_unref(Bdd p);
		
		bool bdd_is_leaf(Bdd p);
		
		bool bdd_leaf_value(Bdd p);
		
		Bdd bdd_then(Bdd p);
		Bdd bdd_else(Bdd p);
		Var bdd_var(Bdd p);
		
		Bdd bdd_leaf(bool v);
		Bdd bdd_var_true(Var v);
		Bdd bdd_var_false(Var v);
		Bdd bdd_var_then_else(Var v, Bdd p_then, Bdd p_else);
		
		Bdd bdd_highest_var(Bdd p);
		Bdd bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod);
		Bdd bdd_rename(Bdd p, const VarMap& fn);
		Bdd bdd_product(Bdd p, Bdd q, ProductFunction& fn);
		Bdd bdd_product(Bdd p, UnaryProductFunction& fn);
		
		void bdd_print(ostream &os, Bdd p);

		unsigned int get_n_nodes(void) const;
	};
}	

#endif /* GBDD_WITH_CUDD */

#endif /* CUDD-SPACE_H */
