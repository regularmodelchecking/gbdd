/*
 * domain.h: 
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

#ifndef GBDD_DOMAIN_H
#define GBDD_DOMAIN_H

#include <set>
#include <gbdd/sgi_ext.h>
#include <assert.h>

namespace gbdd
{

	using namespace std;

/// A set of variables of a BDD to represent a component of a relation
/**
 * A domain is created by giving the base variable, the number of variables, and a step value. The last
 * two variables can be omitted. Some common usage patters:
 *
 * \code
 * Domain d1(0, 5);
 * Domain d2(5, 5);
 * Domain d3(0, 5, 2);
 * Domain d4(1, 5, 2);
 * \endcode
 * 
 * Giving the following sets:
 *
 * d1 = {0,1,2,3,4}
 * d2 = {5,6,7,8,9}
 * d3 = {0,2,4,6,8}
 * d4 = {1,3,5,7,9}
 *
 * d3 and d4 are interleaved, a common way to represent a the state components of a transition relation.
 *
 */

class Domain
{
public:
	typedef unsigned long int Var;
private:

/*
 * Is true if this set is finite
 */
	
	bool _is_finite;

/*
 * Used for finite sets
 */

	set<Var> finite_vars;

/*
 * Used for infinite sets
 */
	
	Var infinite_from;
	Var infinite_step;
	
public:
/// A mapping on variables used for renaming of BDDs
/**
 * A mapping from variables to variables that is the identity relation
 * except the mappings stored in the inherited mapping.
 */

	class VarMap : public hash_map<Var, Var>
	{
	public:
		// Constructor
		VarMap() {}

/// Constructor
/**
 * @param map Hasp map mapping variables
 */
		
		VarMap(const hash_map<Var, Var> &map) : hash_map<Var, Var>(map) {}
		
/// Maps variable
/**
 * @param v Variable to map
 * 
 * @return The mapping of v
 */
		Var operator()(Var v) const
		{
			const_iterator i = find(v);
			
			return (i == end()) ? v : (i->second);
		}

		friend VarMap operator|(const VarMap& map1, const VarMap& map2);
	};



	Domain();
	explicit Domain(Var from, unsigned int n_vars = 1, unsigned int step = 1);
	explicit Domain(const Domain& infinite_base, const Domain& from);
	Domain(const set<Var> &s);
	
	static Domain infinite(Var from = 0, Var step = 1);
	
	bool is_finite() const;
	bool is_infinite() const;
	bool is_empty() const;
	bool is_compatible(const Domain& d2) const;
	bool is_disjoint(const Domain& d2) const;
	unsigned int size() const;

	Var lowest() const;
	Var highest() const;

	Var higher() const;

	bool operator()(Var v) const;

	static VarMap map_vars(const Domain &vs1, const Domain &vs2);

	friend ostream& operator<<(ostream& s, const Domain &vs);

	friend Domain operator|(const Domain& vs1, const Domain& vs2);
	friend Domain operator&(const Domain& vs1, const Domain& vs2);
	friend Domain operator-(const Domain& vs1, const Domain& vs2);

	static Domain sup(const Domain& vs1, const Domain& vs2);

	Domain& operator|=(const Domain &vs);
	Domain& operator&=(const Domain &vs);
	Domain& operator-=(const Domain &vs);

/// transform elements of domain
/**
 * @param op Unary operation
 * 
 * @return The domain such that each variable v is substituted by op(v)
 */
	template <class _UnaryFunction>
	Domain transform(_UnaryFunction op) const;

	Domain operator*(unsigned int v) const;
	Domain operator/(unsigned int v) const;
	Domain operator+(unsigned int v) const;
	Domain operator-(unsigned int v) const;

	friend bool operator==(const Domain& v1, const Domain& v2);
	friend bool operator!=(const Domain& v1, const Domain& v2);

	Domain first_n(unsigned int n) const;
	Domain last_n(unsigned int n) const;

	Domain cut_to_same_size(const Domain& d) const;

	/// Iterator
	class const_iterator
	{
		const Domain* vs;
		Var infinite_v;
		set<Var>::const_iterator finite_i;
		const_iterator(const Domain& vs, Var infinite_v);
		const_iterator(const Domain& vs, set<Var>::const_iterator finite_i);
	public:
		const_iterator();
		const_iterator(const const_iterator& i);

		static const_iterator begin(const Domain& vs);
		static const_iterator end(const Domain& vs);

		Var operator*() const;
			
		const_iterator& operator++();


		friend bool operator==(const_iterator i1, const_iterator i2);
		friend bool operator!=(const_iterator i1, const_iterator i2);
	};

	friend class Domain::const_iterator;

	/// Alias for const_iterator
	typedef const_iterator iterator;

	const_iterator begin() const;
	const_iterator end() const;
};

template <class _UnaryFunction>
Domain Domain::transform(_UnaryFunction op) const
{
	assert(is_finite());

	Domain s;
	set<Var>::const_iterator i = finite_vars.begin();
	
	while (i != finite_vars.end())
	{
		s.finite_vars.insert(op(*i));
		i++;
	}
	
	return s;
}

/// Tuple of domains used to type Relation
class Domains
{
	vector<Domain> doms;
public:
	Domains(unsigned int n = 0);
	Domains(Domain d1);

	bool is_some_infinite() const;

	friend Domains operator*(Domains ds1, Domains ds2);

	Domain& operator[](unsigned int idx);
	const Domain& operator[](unsigned int idx) const;

	friend bool operator==(Domains ds1, Domains ds2);

	static Domains sup(const Domains& ds1, const Domains& ds2);

	Domain union_all() const;

	bool is_disjoint(const Domains& ds2) const;

	Domains intersect_with(Domain d) const;
	Domains cut_to_same_sizes(Domains ds) const;

	/// Iterator
	typedef vector<Domain>::const_iterator const_iterator;

/// Get iterator pointing to beginning
/**
 * @return Iterator pointing to beginning
 */
	const_iterator begin() const { return doms.begin(); }

/// Get iterator pointing to end
/**
 * @return Iterator pointing to end
 */
	const_iterator end() const { return doms.end(); }

/// Get size of domains
/**
 * @return The number of elements in this vector of domains
 */
	unsigned int size() const { return doms.size(); }

/// transform elements of domains
/**
 * @param op Unary operation
 * 
 * @return The domains such that each variable v is substituted by op(v)
 */
	template <class _UnaryFunction>
	Domains transform(_UnaryFunction op) const;
};

template <class _UnaryFunction>
Domains Domains::transform(_UnaryFunction op) const
{
	Domains res = *this;

	for (unsigned int i = 0;i < size();++i)
	{
		res[i] = res[i].transform(op);
	}
	
	return res;
}

}

#endif /* GBDD_DOMAIN_H */
