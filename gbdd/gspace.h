/*
 * gspace.h: 
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
#ifndef GSPACE_H
#define GSPACE_H

#include <gbdd/space.h>

namespace gbdd
{

/// Slow reference implementation of Space
class GSpace : public Space
{
	class BddPair
	{
	public:
		Bdd p, q;
		BddPair(Bdd p, Bdd q) : p(p), q(q) {}

		bool operator==(const BddPair bp2) const
		{
			return (p == bp2.p && q == bp2.q);
		}
	};
	
	struct hash_bddpair
	{
		size_t operator()(BddPair np) const
		{
			return np.p + np.q;
		}
	};
	
	template <class T>
	class HashBddPair : public hash_map<BddPair, T, hash_bddpair>
	{};

	class Node
	{
	public:
		Var v;
		Bdd left, right;
		
		Node(Var v, Bdd left, Bdd right):
			v(v),
			left(left),
			right(right)
		{}
	};

	template <class T>
	class HashBdd : public hash_map<Bdd, T>
	{};
	

/* 
 * Vector of unique tables indexed by variables. Each variable
 * has its own unique table. For a variable v, and bdd nodes
 * p and q, the expression unique_tables[v][(BddPair(p, q))]
 * denotes the bdd bdd_if(v, p, q).
 */
	
	vector<HashBddPair<Bdd> > unique_tables;

/*
 * Vector of nodes. For a bdd p, the expression node_table[p]
 * denotes the node for p.
 */

	vector<Node> node_table;

/*
 * A byte is used to represent an operation (16 possible)
 */
	
	typedef unsigned char Operation;
	
	static Operation fn_to_operation(GSpace::ProductFunction& fn);
/*
 * product_cache[op] represents the cache for operation op
 */
	
	vector<HashBddPair<Bdd> > product_cache;

	Node& get_node(Bdd bdd);
	HashBddPair<Bdd>& get_cache(Operation op);

	Bdd bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod,
			HashBdd<Bdd>& cache);

	template <class _VarFunction>
       	Bdd bdd_rename_linear(Bdd p, _VarFunction fn);
public:
	GSpace();
	/// Destructor
	virtual ~GSpace() {}

	void gc();

	void bdd_ref(Bdd p);
	void bdd_unref(Bdd p);

	bool bdd_is_leaf(Bdd p);
       
	bool bdd_leaf_value(Bdd p);

	Bdd bdd_then(Bdd p);
	Bdd bdd_else(Bdd p);
	Var bdd_var(Bdd p);

	Bdd bdd_leaf(bool v);
	Bdd bdd_var_then_else(Var v, Bdd p_then, Bdd p_else);
	Bdd bdd_var_true(Var v);
	Bdd bdd_var_false(Var v);

	Bdd bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod);
	Bdd bdd_rename(Bdd p, const VarMap& fn);
	Bdd bdd_product(Bdd p, Bdd q, ProductFunction& fn);
	Bdd bdd_product(Bdd p, UnaryProductFunction& fn);

	void bdd_print(ostream &os, Bdd p);
		
};

}	

#endif /* GSPACE_H */
