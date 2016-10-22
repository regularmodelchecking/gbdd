/*
 * gspace.cc: 
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

#include <gbdd/gspace.h>
#include <gbdd/domain.h>
#include <iostream>

namespace gbdd
{

/// Constructor
GSpace::GSpace():
	node_table(2, Node(0, 0, 0)),
	product_cache(16, HashBddPair<Bdd>())
{
}

GSpace::Node &GSpace::get_node(Bdd bdd)
{
	return node_table[bdd];
}

GSpace::HashBddPair<GSpace::Bdd> &GSpace::get_cache(Operation op)
{
	return product_cache[(unsigned int)op];
}

GSpace::Operation GSpace::fn_to_operation(GSpace::ProductFunction& fn)
{
	GSpace::Operation op = 0;

	if (fn(false, false)) op |= 0x01;
	if (fn(true, false)) op |= 0x02;
	if (fn(false, true)) op |= 0x04;
	if (fn(true, true)) op |= 0x08;

	return op;
}

/**
 * gc:
 * 
 * Performs Garbage Collection. All nodes that are not ref:ed are
 * removed.
 */

void GSpace::gc()
{
	return;
}

void GSpace::bdd_ref(Bdd p)
{
	return;
}

void GSpace::bdd_unref(Bdd p)
{
	return;
}

/**
 * bdd_is_leaf:
 * @param p BDD node to check if it is leaf
 * 
 * Checks for leaf node
 *
 * Returns: True if \a p is a leaf node, otherwise false
 */

bool GSpace::bdd_is_leaf(Bdd p)
{
	return (p == 0 || p == 1);
}
       
/**
 * bdd_leaf_value:
 * @param p BDD node to check leaf value of
 * 
 * Returns value of leaf node
 * 
 * Returns: Value of the leaf node \a p
 */

bool GSpace::bdd_leaf_value(Bdd p)
{
	assert(bdd_is_leaf(p));

	return (p == 0) ? false : true;
}

/**
 * bdd_then:
 * @param p BDD node the get then-branch of
 * 
 * Gets then-branch of a BDD
 * 
 * Returns: The node corresponding to then-branch of \a p
 */

GSpace::Bdd GSpace::bdd_then(Bdd p)
{
	assert(!bdd_is_leaf(p));

	return get_node(p).left;
}

/**
 * bdd_else:
 * @param p BDD node to get else-branch of
 * 
 * Gets else-branch of a BDD
 * 
 * Returns: The node corresponding to else-branch of \a p
 */

GSpace::Bdd GSpace::bdd_else(Bdd p)
{
	assert(!bdd_is_leaf(p));

	return get_node(p).right;
}

/**
 * bdd_var:
 * @param p BDD to get variable of
 * 
 * Gets variable of BDD node
 * 
 * Returns: Variable of \a p
 */

GSpace::Var GSpace::bdd_var(Bdd p)
{
	assert(!bdd_is_leaf(p));
	
	return get_node(p).v;
}

/**
 * bdd_leaf:
 * @param v Value of leaf
 * 
 * Create leaf node
 * 
 * Returns: The leaf node corresponding to \a v
 */

GSpace::Bdd GSpace::bdd_leaf(bool v)
{
	return v ? 1 : 0;
}

/**
 * bdd_var_then_else:
 * @param v Variable of node
 * @param p_then BDD of then-branch
 * @param p_else BDD of else-branch
 * 
 * Creates new BDD node
 * 
 * Returns: The bdd node corresponding to if v then p_then else p_else
 */

GSpace::Bdd GSpace::bdd_var_then_else(Var v, Bdd p_then, Bdd p_else)
{
	if (p_then == p_else) return p_then;

	while (unique_tables.size() < (v+1))
	{
		unique_tables.push_back(HashBddPair<Bdd>());
	}

	HashBddPair<Bdd>::iterator i = unique_tables[v].find(BddPair(p_then, p_else));

	if (i == unique_tables[v].end())
	{
		Bdd new_node = node_table.size();
		node_table.push_back(Node(v, p_then, p_else));

		unique_tables[v][BddPair(p_then, p_else)] = new_node;

		return new_node;
	}
	else
	{
		return i->second;
	}
}

GSpace::Bdd GSpace::bdd_var_true(Var v)
{
	return bdd_var_then_else(v, bdd_true(), bdd_false());
}

GSpace::Bdd GSpace::bdd_var_false(Var v)
{
	return bdd_var_then_else(v, bdd_false(), bdd_true());
}

/**
 * bdd_project:
 * @param p BDD to project
 * @param fn_var What variables to project
 * @param fn_prod Product function for project
 *
 * Projects variables given by \a fn_var in \a p, such that 
 * if (v, p1, p2) is replaced by the product of p1 and p2 under \a fn_prod
 * 
 * Returns: The projection
 */

GSpace::Bdd GSpace::bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod)
{
	HashBdd<Bdd> cache;

	return bdd_project(p, fn_var, fn_prod, cache);
}

GSpace::Bdd GSpace::bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod,
			      HashBdd<Bdd>& cache)
{
	if (bdd_is_leaf(p)) return p;

	{
		
		HashBdd<Bdd>::iterator i = cache.find(p);
		
		if (i != cache.end())
		{
			return i->second;
		}
	}

	Bdd r =
		fn_var(bdd_var(p)) ?
		bdd_product(bdd_project(bdd_then(p), fn_var, fn_prod, cache),
			    bdd_project(bdd_else(p), fn_var, fn_prod, cache),
			    fn_prod):
		bdd_var_then_else(bdd_var(p),
		       bdd_project(bdd_then(p), fn_var, fn_prod, cache),
		       bdd_project(bdd_else(p), fn_var, fn_prod, cache));

	cache[p] = r;

	return r;
}


/**
 * bdd_product:
 * @param p BDD in product 1
 * @param q BDD in product 2
 * @param fn Product function
 *
 * Creates product of two BDDs
 * 
 * Returns: The product of \a p and \a q w.r.t. the product function \a fn
 */

GSpace::Bdd GSpace::bdd_product(Bdd p, Bdd q, ProductFunction& fn)
{
	HashBddPair<Bdd> &cache = get_cache(GSpace::fn_to_operation(fn));
	{
		
		HashBddPair<Bdd>::iterator i = cache.find(BddPair(p, q));
		
		if (i != cache.end())
		{
			return i->second;
		}
	}
	
	Bdd r =
		(bdd_is_leaf(p) && bdd_is_leaf(q)) ? (bdd_leaf(fn(bdd_leaf_value(p), bdd_leaf_value(q)))):
		(bdd_is_leaf(p) && !bdd_is_leaf(q)) ? bdd_var_then_else(bdd_var(q), bdd_product(p, bdd_then(q), fn), bdd_product(p, bdd_else(q), fn)):
		(!bdd_is_leaf(p) && bdd_is_leaf(q)) ? bdd_var_then_else(bdd_var(p), bdd_product(bdd_then(p), q, fn), bdd_product(bdd_else(p), q, fn)):
		(bdd_var(p) == bdd_var(q)) ? bdd_var_then_else(bdd_var(p),
						    bdd_product(bdd_then(p), bdd_then(q), fn),
						    bdd_product(bdd_else(p), bdd_else(q), fn)):
		(bdd_var(p) < bdd_var(q)) ? bdd_var_then_else(bdd_var(p), bdd_product(bdd_then(p), q, fn), bdd_product(bdd_else(p), q, fn)):
 		                            bdd_var_then_else(bdd_var(q), bdd_product(p, bdd_then(q), fn), bdd_product(p, bdd_else(q), fn));

	cache[BddPair(p, q)] = r;

	return r;
}

/**
 * bdd_product:
 * @param p BDD to take product of
 * @param fn Product function from bool to bool
 *
 * Computes the unary product
 * 
 * Returns: Unary product of \a p w.r.t. \a fn
 */

GSpace::Bdd GSpace::bdd_product(Bdd p, UnaryProductFunction& fn)
{
	if (bdd_is_leaf(p))
	{
		return bdd_leaf(fn(bdd_leaf_value(p)));
	}
	else
	{
		return bdd_var_then_else(bdd_var(p),
			      bdd_product(bdd_then(p), fn),
			      bdd_product(bdd_else(p), fn));
	}
}
	
static Space::Var fn_expand(Space::Var v)
{
	return v * 2;
}

static Space::Var fn_collapse(Space::Var v)
{
	return v / 2;
}

static bool fn_iff(bool v1, bool v2) { return (v1 && v2) || (!v1 && !v2); }
static bool fn_and(bool v1, bool v2) { return v1 && v2; }
static bool fn_or(bool v1, bool v2) { return v1 || v2; }

GSpace::Bdd GSpace::bdd_rename(Bdd p, const VarMap& fn)
{
	Bdd expanded = bdd_rename_linear(p, fn_expand);

	Bdd mapping = bdd_true();
	Domain to_project;
	VarMap::const_iterator i;
	for(i = fn.begin();i != fn.end();++i)
	{
		Bdd v1_iff_v2 = Space::bdd_product(bdd_var_true(i->first * 2), bdd_var_true(i->second * 2 + 1), fn_iff);

		mapping = Space::bdd_product(mapping, v1_iff_v2, fn_and);
		to_project |= Domain(i->first * 2);
	}

	Bdd product = Space::bdd_product(expanded, mapping, fn_and);

  	return bdd_rename_linear(Space::bdd_project(product, to_project, fn_or), fn_collapse);
}

/**
 * Renames \a p according to \a fn
 * 
 * @param p BDD to rename
 * @param fn Renaming function, must be linear!
 * 
 * Returns: \a p [v / f(v)] for all variables v in \a p
 */

template <class _VarFunction>
GSpace::Bdd GSpace::bdd_rename_linear(Bdd p, _VarFunction fn)
{
	if (bdd_is_leaf(p)) return p;

	return bdd_var_then_else(fn(bdd_var(p)), 
		      bdd_rename_linear(bdd_then(p), fn),
		      bdd_rename_linear(bdd_else(p), fn));
}

/// Prints BDD
/**
 * Prints BDD \a p in human readable form to stream \a os
 *
 * @param os Stream to print on
 * @param p BDD to print
 * 
 */

void GSpace::bdd_print(ostream &os, Bdd p)
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
