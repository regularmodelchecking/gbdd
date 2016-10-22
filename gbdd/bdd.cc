/*
 * bdd.cc: 
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

#include <gbdd/bdd.h>
#include <math.h>
#include <iostream>
#include <queue>

namespace gbdd
{

Bdd::Factory::Factory(Space* space):
	space(space)
{}

Bdd::Factory::~Factory()
{}

Bdd* Bdd::Factory::ptr_constant(bool v) const
{
	return new Bdd(space, v);
}

Bdd* Bdd::Factory::ptr_var(Var v, bool var_v) const
{
	return new Bdd(var_v ? Bdd::var_true(space, v) : Bdd::var_false(space, v));
}

BoolConstraint* Bdd::ptr_convert(const BoolConstraint::Factory& f) const
{
	assert(false);
}

	
/// Create undefined BDD
Bdd::Bdd()
{
	space = NULL;
}

/// Constructs a leaf BDD
/**
 *
 * @param space Space to create leaf BDD in
 * @param v Value of leaf
 * 
 */

Bdd::Bdd(Space* space, bool v)
{
	space->lock_gc();

	this->space = space;
	space_bdd = space->bdd_leaf(v);

	space->bdd_ref(space_bdd);

	space->unlock_gc();
}

/// Destructor

Bdd::~Bdd()
{
	if (space != NULL)
	{
		space->bdd_unref(space_bdd);
	}
}

/// Copy constructor

Bdd::Bdd(const Bdd &p)
{
	space = p.space;

	if(space != NULL)
	{
		space_bdd = p.space_bdd;
		
		space->bdd_ref(space_bdd);
	}
}

/// Private constructor to construct BDDs from BDD nodes
/**
 * 
 * @param space Space that BDD resides in
 * @param bdd BDD residing in \p space
 * 
 */

Bdd::Bdd(Space* space, Space::Bdd bdd)
{
	this->space = space;
	space_bdd = bdd;
	space->bdd_ref(space_bdd);
}


static bool fn_negate(bool v)
{
	return !v;
}

/// Negates BDD
/**
 * @return The negated BDD
 */

Bdd Bdd::operator! () const
{
	return product(fn_negate);
}

/// OR operator
/**
 * @param p2 BDD to or with
 * 
 * @return This BDD or'ed with \p p2
 */

Bdd Bdd::operator| (const Bdd& p2) const
{
	return bdd_product(*this, p2, fn_or);
}

/// AND operator
/**
 * @param p2 BDD to and with
 * 
 * @return This BDD and'ed with \p p2
 */

Bdd Bdd::operator& (const Bdd& p2) const
{
	return bdd_product(*this, p2, fn_and);
}

/// Set difference
/**
 * @param p2 BDD to minus with
 * 
 * @return This BDD minus'ed with \p p2
 */

Bdd Bdd::operator- (const Bdd& p2) const
{
	return bdd_product(*this, p2, fn_minus);
}

/// Assignment operator
Bdd& Bdd::operator= (const Bdd &p)
{
	if (space != NULL)
	{
		space->bdd_unref(space_bdd);
	}

	space = p.space;

	if (space != NULL)
	{
		space_bdd = p.space_bdd;

		space->bdd_ref(space_bdd);
	}

	return *this;
}

/// Assignment OR
Bdd& Bdd::operator|= (const Bdd &p)
{
	return (*this = *this | p);
}

/// Assignment AND
Bdd& Bdd::operator&= (const Bdd &p)
{
	return (*this = *this & p);
}

/// Assignment Set minus
Bdd& Bdd::operator-= (const Bdd &p)
{
	return (*this = *this - p);
}

/// Equality
/**
 * @param p1 BDD to compare
 * @param p2 BDD to compare with
 * 
 * @return Whether \a p1 is equal to \a p2
 */

bool operator== (const Bdd& p1, const Bdd& p2)
{
	return p1.space_bdd == p2.space_bdd;
}

bool Bdd::operator==(const StructureConstraint& b2) const
{
	// TODO: check type

	return *this == (const Bdd&)b2;
}

/// Test if BDD is a leaf
/**
 * @return True if this BDD is a leaf
 */
bool Bdd::bdd_is_leaf() const
{
	return space->bdd_is_leaf(space_bdd);
}

/// Get leaf value of BDD
/**
 * @return Leaf value of this BDD
 */
bool Bdd::bdd_leaf_value() const
{
	return space->bdd_leaf_value(space_bdd);
}

/// Get then-branch of BDD
/**
 * @return then-branch of this BDD
 */
Bdd Bdd::bdd_then() const
{
	return Bdd(space, space->bdd_then(space_bdd));
}

/// Get else-branch of BDD
/**
 * @return else-branch of this BDD
 */
Bdd Bdd::bdd_else() const
{
	return Bdd(space, space->bdd_else(space_bdd));
}

/// Get variable of this node
/**
 * @return Variable of this node
 */
Bdd::Var Bdd::bdd_var() const
{
	return space->bdd_var(space_bdd);
}


/// Construct BDD that tests for variable
/**
 * @param space Space that BDD should reside in
 * @param v Variable to test
 * 
 * @return The BDD testing if \p v is true
 */

Bdd Bdd::var_true (Space* space, Var v)
{
	space->lock_gc();

	Bdd res(space, space->bdd_var_true(v));

	space->unlock_gc();

	return res;
}

/// Construct BDD that tests for variable
/**
 * @param space Space that BDD should reside in
 * @param v Variable to test
 * 
 * @return The BDD testing if \p v is false
 */
Bdd Bdd::var_false (Space* space, Var v)
{
	space->lock_gc();

	Bdd res(space, space->bdd_var_false(v));

	space->unlock_gc();

	return res;
}

/// Construct BDD 
/**
 * @param space Space that BDD should reside in
 * @param v Variable to test
 * @param p_then then-branch i \p v is true
 * @param p_else else-branch i \p v is false
 * 
 * @return The BDD testing variable \p v taking \p p_then if \p v is true and \p p_else if \p v is false
 */

Bdd Bdd::var_then_else(Space* space, Var v, Bdd p_then, Bdd p_else)
{
	space->lock_gc();

	Bdd res(space, space->bdd_var_then_else(v, p_then.space_bdd, p_else.space_bdd));

	space->unlock_gc();

	return res;
}

/// Encodes a value as a BDD using a binary representation
/**
 * @param space BDD space
 * @param vs Variables to use for encoding
 * @param v Value to encode
 * 
 * @return A BDD using the variables \p vs to encode the value \p v
 */

Bdd Bdd::value(Space* space, const Domain &vs, unsigned int v)
{
	Bdd p(space, true);

	Domain::iterator next_var = vs.begin();
	unsigned int i = 0;

	while (next_var != vs.end())
	{
		p &= (v & (1 << i)) ?
			var_true(space, *next_var):
			var_false(space, *next_var);

		++next_var;
		++i;
	}

	return p;
}

/// Encodes a value range as a BDD
/**
 * @param space BDD space
 * @param vs Variables to use for encoding
 * @param from_v Beginning of interval to encode
 * @param to_v End of interval to encode
 * 
 * @return A BDD using the variable \p vs to encode the value range [\p from_v ..\p to_v]
 */

Bdd Bdd::value_range(Space* space, const Domain& vs, unsigned int from_v, unsigned int to_v)
{
	unsigned int size = 1 << vs.size();

	if (size == 1) return Bdd(space, from_v == 0 && to_v == 0);

	if (from_v == 0 && (to_v + 1) == size) return Bdd(space, true);
	if(from_v > to_v) return Bdd(space, false);

	// Recursive case
	// split_v is the value of the most significant bit
	unsigned int split_v = size / 2;

	Domain vs_rec = vs;
	Var highest_var = vs.highest();
	vs_rec -= Domain(highest_var);

	Bdd low = value_range(space, vs_rec, from_v, min(to_v, split_v - 1));
	Bdd high =
		(to_v >= split_v) ? 
		value_range(space, vs_rec, max(from_v, split_v) - split_v, to_v - split_v):
		Bdd(space, false);

	return
		(var_true(space, highest_var) & high) |
		(var_false(space, highest_var) & low);
}

/// Get number of variables needed for encoding of values
/**
 * @param n_values Number of values to represent
 * 
 * @return Minimum number of variables needed to encode 0..\p n_values - 1
 */

unsigned int Bdd::n_vars_needed(unsigned int n_values)
{
	unsigned int n_vars = 0;

	while (((unsigned int)(1 << n_vars)) < n_values) ++n_vars;

	return n_vars;
}


Bdd Bdd::value_follow(Domain::const_iterator current_var,
		      Domain::const_iterator end_var,
		      unsigned int current_v) const
{
	if (current_var == end_var) return *this;

	if (space->bdd_is_leaf(space_bdd))
	{
		return *this;
	}
	else
	{
		Var v = space->bdd_var(space_bdd);

		assert (v >= *current_var);

		if (v > *current_var)
		{
			++current_var;
			return value_follow(current_var, end_var,
					    current_v / 2);
		}
		else
		{
			Bdd p_cont =
				(current_v & 0x01) ?
				Bdd(space, space->bdd_then(space_bdd)):
				Bdd(space, space->bdd_else(space_bdd));

			++current_var;

			return p_cont.value_follow(current_var,
						   end_var,
						   current_v / 2);
		}
	}
}
       

/// Membership of value
/**
 * Tests the assignment of vs by encoding v is a valid
 * assignment in the BDD
 * 
 * @param vs Variables to encode value in
 * @param v Value to encode
 * 
 * @return True if \p v encoded in \p vs is a member.
 */

bool Bdd::value_member(const Domain& vs, unsigned int v) const
{
	Bdd p = value_follow(vs, v);

	assert(p.bdd_is_leaf());

	return p.bdd_leaf_value();
}

/// Membership of value
/**
 * Follows a value encoded in the variables that must be lowest in the BDD (i.e. at the top)
 * 
 * @param vs Variables to encode value in
 * @param v Value to encode
 * 
 * @return The BDD gain by following \a v encoded with \a vs
 */

Bdd Bdd::value_follow(const Domain& vs, unsigned int v) const
{
	return value_follow(vs.begin(), vs.end(), v);
}

/// Construct BDD representing equality between variables
/**
 * @param space Space for resulting BDD
 * @param vs1 Domain for one set of variables
 * @param vs2 Domain for another set of variables
 * 
 * @return A BDD representing assignments where the variables in \p vs1 are equal to the correspondning variable in \p vs2
 */

Bdd Bdd::vars_equal(Space* space, const Domain &vs1, const Domain &vs2)
{
	return vars_product(space, vs1, vs2, fn_iff);
}

unsigned int Bdd::n_assignments(Domain::const_iterator current_var,
				Domain::const_iterator end_var) const
{ 	
	unsigned int res;

	if (space->bdd_is_leaf(space_bdd))
	{
		if (space->bdd_leaf_value(space_bdd))
		{
			if (current_var == end_var)
				res = 1;
			else
			{
				++current_var;
				res = 2 * n_assignments(current_var, end_var);
			}
		}
		else
		{
			res = 0;
		}
	}
	else
	{
		Var v = space->bdd_var(space_bdd);
		assert (current_var != end_var && v >= *current_var);

		if (v == *current_var)
		{
			Bdd p_then = Bdd(space, space->bdd_then(space_bdd));
			Bdd p_else = Bdd(space, space->bdd_else(space_bdd));
			++current_var;

			res =
				p_then.n_assignments(current_var, end_var) +
				p_else.n_assignments(current_var, end_var);
		}
		else
		{
			++current_var;
			
			res = 2 * n_assignments(current_var, end_var);
		}
	}

	return res;
}

/// Get number of possible assignments
/**
 * Calculates the number of assignments to variables in \p vs that
 * makes the BDD true. All variables in BDD must be in \p vs.
 * 
 * @param vs Variable to assign values to
 * 
 * @return The number of possible assignments
 */

unsigned int Bdd::n_assignments(const Domain& vs) const
{
	return n_assignments(vs.begin(), vs.end());
}

void Bdd::assignments_value(Domain::const_iterator current_var,
			    Domain::const_iterator end_var,
			    unsigned int current_base,
			    unsigned int current_v,
			    set<unsigned int>& result) const
{ 	
	if (space->bdd_is_leaf(space_bdd))
	{
		if (space->bdd_leaf_value(space_bdd))
		{
			if (current_var == end_var)
			{
				result.insert(current_v);
			}
			else
			{
				++current_var;
				assignments_value(current_var, end_var, current_base << 1, current_v | current_base, result);
				assignments_value(current_var, end_var, current_base << 1, current_v, result);
			}
		}
	}
	else
	{
		Var v = space->bdd_var(space_bdd);
		assert (current_var != end_var && v >= *current_var);

		if (v == *current_var)
		{
			Bdd p_then = Bdd(space, space->bdd_then(space_bdd));
			Bdd p_else = Bdd(space, space->bdd_else(space_bdd));
			++current_var;

			p_then.assignments_value(current_var, end_var, current_base << 1,  current_v | current_base, result);
			p_else.assignments_value(current_var, end_var, current_base << 1,  current_v, result);
		}
		else
		{
			++current_var;
			
			assignments_value(current_var, end_var, current_base << 1, current_v | current_base, result);
			assignments_value(current_var, end_var, current_base << 1, current_v, result);
		}
	}
}

/// Get all assignments interpreted as values
/**
 * Calculates assignments to variables in \p vs that
 * makes the BDD true. All variables in BDD must be in \p vs.
 * 
 * @param vs Variable to assign values to
 * 
 * @return The set of values where the encoding in \a vs is an assignment
 */

set<unsigned int> Bdd::assignments_value(const Domain& vs) const
{
	set<unsigned int> res;

	assignments_value(vs.begin(), vs.end(), 1, 0, res);

	return res;
}

void Bdd::with_geq_var(Bdd::Var v, hash_set<Bdd>& res) const
{
	if (bdd_is_leaf())
	{
		res.insert(*this);
	}
	else
	{
		Bdd::Var p_v = bdd_var();

		if (p_v >= v)
		{
			res.insert(*this);
		}
		else
		{
			bdd_then().with_geq_var(v, res);
			bdd_else().with_geq_var(v, res);
		}
	}

	return;
}

/// Get subtrees with variable over some threshold
/**
 * @param v Threshold value
 *
 * @return The set of subtrees of this BDD that has a variable node with a variable greater or equal to \p v and do not contain such subtree.
 */

hash_set<Bdd> Bdd::with_geq_var(Bdd::Var v) const
{
	hash_set<Bdd> res;

	with_geq_var(v, res);

	return res;
}


Bdd Bdd::with_image_geq_var(Space* space, Space::Bdd space_p, Space::Bdd space_im, Var v)
{
	if(!space->bdd_is_leaf(space_p) && space->bdd_var(space_p) < v)
	{
		return Bdd::var_then_else(space, space->bdd_var(space_p), 
					  with_image_geq_var(space, space->bdd_then(space_p), space_im, v),
					  with_image_geq_var(space, space->bdd_else(space_p), space_im, v));
	}
	else
	{
		return Bdd(space, space_p == space_im);
	}
}

/// Return all reachable nodes in this BDD
/**
 * 
 *
 * @return All nodes reachable from this BDD using bdd_then and bdd_else
 */

hash_set<Bdd> Bdd::nodes() const
{
	hash_set<Bdd> res;
	std::queue<Bdd> explore;

	res.insert(*this);
	explore.push(*this);
	while (!explore.empty())
	{
		const Bdd& p = explore.front();

		if (!p.bdd_is_leaf())
		{
			Bdd p_then = p.bdd_then();
			Bdd p_else = p.bdd_else();
			
			if (res.find(p_then) == res.end()) { explore.push(p_then); res.insert(p_then); }
			if (res.find(p_else) == res.end()) { explore.push(p_else); res.insert(p_else); }
		}

		explore.pop();
	}

	return res;
}
	


/// Construct BDD representing the set of assignments leading to a subtree
/**
 * @param im Subtree to check
 * @param v Threshold value
 *
 * @return The set of assignments of variables less than \p a leadning to the subtree \p im.
 */

Bdd Bdd::with_image_geq_var(Bdd im, Bdd::Var v) const
{
	return with_image_geq_var(space, space_bdd, im.space_bdd, v);
}

/// Get highest variable
/**
 * @return The highest variable in this BDD
 */
Space::Var Bdd::highest_var() const
{
	return space->bdd_highest_var(space_bdd);
}
		
/// Get lowest variable
/**
 * @return The lowest variable in this BDD
 */
Space::Var Bdd::lowest_var() const
{
	return bdd_is_leaf() ?
		0:
		bdd_var();
}

/// Get set of variables in BDD
/**
 * @return The set of variables that exist in any node in this BDD
 */
Domain Bdd::vars() const
{
	return bdd_is_leaf() ?
		Domain():
		Domain(bdd_var()) | (bdd_then().vars()) | (bdd_else().vars());
}

/// Test for false BDD
/**
 * @return true if BDD is false
 */
bool Bdd::is_false() const
{
	return (space_bdd == space->bdd_false());
}

/// Test for true BDD
/**
 * @return true if BDD is true
 */
bool Bdd::is_true() const
{
	return (space_bdd == space->bdd_true());
}

/// Print a textual representation on a stream	
/**
 * @param s Stream
 * @param p BDD to print
 * 
 * Prints BDD \p p on \p s
 * 
 */

ostream& operator<< (ostream& s, const Bdd& p)
{
	p.space->bdd_print(s, p.space_bdd);

	return s;
}

/// Print a dot representation of the BDD
/**
 * Code inspired by the Cudd_DumpDot function in the CUDD package by Fabio Somenzi 
 *
 * @param os Stream to write to
 * 
 * @return The stream \a os
 */

ostream& Bdd::print_dot(ostream& os) const
{
	os << "digraph \"Bdd\" {" << std::endl;
	os << "size = \"7.5,10\"\ncenter = true;\nedge [dir = none];\n";

	typedef unsigned int Label;
	
	hash_set<Bdd> all_nodes = nodes();
	Label next_label = 0;

	typedef hash_map<Var,vector<Bdd> > VarBdds;

	VarBdds nodes_with_var;

	for (hash_set<Bdd>::const_iterator i = all_nodes.begin();i != all_nodes.end();++i)
	{
		Bdd p = *i;

		if (p.bdd_is_leaf())
		{
			if (p.is_true())
			{
				os << p.space_bdd << " [shape = box,label=\"1\"];" << std::endl;
			}
		}
		else
		{
			nodes_with_var[p.bdd_var()].push_back(p);

			os << p.space_bdd << " [label=\"S" << next_label  << "\"];" << std::endl;
			next_label++;

			Bdd p_then = p.bdd_then();
			Bdd p_else = p.bdd_else();

			if (!p_then.is_false()) os << p.space_bdd << " -> " << p_then.space_bdd << " [style = solid];" << std::endl;
			if (!p_else.is_false()) os << p.space_bdd << " -> " << p_else.space_bdd << " [style = dashed];" << std::endl;
		}
	}

	// Create variable labels

	os << "{ node [shape = plaintext]; edge [style = invis];";
	os << "\"VAR NODES\" [style = invis];";

	// Label nodes
	for (VarBdds::const_iterator i = nodes_with_var.begin();i != nodes_with_var.end();++i)
	{
		Var v = i->first;
		os << "v" << v << " [label=\"" << v << "\"];";
	}


	// Connect them with invisible edges, aligning them horizontally
	for (VarBdds::const_iterator i = nodes_with_var.begin();i != nodes_with_var.end();++i)
	{
		Var v = i->first;
		os << "v" << v << " -> ";
	}
	os << "\"VAR NODES\";}" << std::endl;

	// Write rank info

	for (VarBdds::const_iterator i = nodes_with_var.begin();i != nodes_with_var.end();++i)
	{
		const vector<Bdd>& ps = i->second;
		Var v = i->first;

		os << "{ rank = same; v" << v << ";";

		for (vector<Bdd>::const_iterator j = ps.begin();j != ps.end();++j)
		{
			os << j->space_bdd << ";";
		}

		os << "}"  << std::endl;
	}

	// Close graph 

	os << "};"  << std::endl;

	return os;
}


/// Garbage collect BDD space

void Bdd::gc(Space* space)
{
	space->gc();
}

Bdd* Bdd::ptr_rename(VarMap map) const
{
	return new Bdd(rename(map));
}

Bdd* Bdd::ptr_project(Domain vs) const
{
	return new Bdd(project(vs));
}

Bdd* Bdd::ptr_constrain_value(Var v, bool value) const
{
	return new Bdd(*this & (value ? Bdd::var_true(get_space(), v) : Bdd::var_false(get_space(), v)));
}
	
Bdd* Bdd::ptr_product(const StructureConstraint& b2, bool (*fn)(bool v1, bool v2)) const
{
	return new Bdd(bdd_product(*this, (const Bdd&)b2, fn));
}

Bdd* Bdd::ptr_negate() const
{
	return new Bdd(!*this);
}

Bdd* Bdd::ptr_clone() const
{
	return new Bdd(*this);
}

Bdd Bdd::FiniteVar::operator==(const Bdd::FiniteVar& fv2) const
{
	Bdd p(_space, true);
	Domain::const_iterator i1 = _vs.begin();
	Domain::const_iterator i2 = fv2._vs.begin();
	
	while (i1 != _vs.end())
	{
		assert(i2 != _vs.end());
		p &= Bdd::var_equal(_space, *i1, *i2);
		++i1;
		++i2;
	}
	assert(i2 == fv2._vs.end());

	return p;
}

}

namespace std
{


/// Hash function for BDDs
size_t hash<gbdd::Bdd>::operator()(gbdd::Bdd p) const
{
	return p.space_bdd;
}

}
