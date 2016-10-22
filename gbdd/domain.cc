/*
 * domain.cc: 
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

#include <gbdd/domain.h>
#include <iostream>
#include <assert.h>

namespace gbdd
{


/// Creates an empty domain
/**
 * 
 *
 */
Domain::Domain():
	_is_finite(true)
{}

/// Creates a finite domain
/**
 * 
 *
 * @param from The first variable
 * @param n_vars The number of variables
 * @param step Step starting \a from for the variables
 * 
 * @return The finite domain {\a from + i * \a step | 0 <= \a n_vars < n_vars }
 */

Domain::Domain(Var from, unsigned int n_vars, unsigned int step):
	_is_finite(true)
{
	while(n_vars > 0)
	{
		finite_vars.insert(from);
		from += step;
		--n_vars;
	}
}

/// Creates a finite domain given a set of variables
/**
 * 
 *
 * @param s The set of variables
 * 
 * @return A domain containing the variables in \a s
 */

Domain::Domain(const set<Var> &s) : 
	_is_finite(true),
	finite_vars(s)
{}

/// Lay out one domain onto an infinite base
/**
 * 
 *
 * @param infinite_base The base to lay out domain onto
 * @param from The domain to lay out
 * 
 * @return The domain such that each i variable in \a from is now the i variable in \a infinite_base
 */

Domain::Domain(const Domain& infinite_base, const Domain& from)
{
	assert(!infinite_base.is_finite());
	assert(from.is_finite());

	*this = (from * infinite_base.infinite_step) + infinite_base.infinite_from;
}

/// Creates an infinite domain
/**
 * 
 *
 * @param from The first variable
 * @param step Step starting \a from for the variables
 * 
 * @return The infinite domain {\a from + i * \a step | i >= 0 }
 */

Domain Domain::infinite(Var from, Var step)
{
	Domain vs;
	vs._is_finite = false;
	vs.infinite_from = from;
	vs.infinite_step = step;

	return vs;
}

/// Checks if domain is finite
/**
 * 
 *
 * @return Whether the domain is finite
 */

bool Domain::is_finite() const
{
	return _is_finite;
}

/// Checks if domain is infinite
/**
 * 
 *
 * @return Whether the domain is infinite
 */

bool Domain::is_infinite() const
{
	return !is_finite();
}

/// Checks if domain is empty
/**
 * 
 *
 * @return Whether the domain is empty
 */

bool Domain::is_empty() const
{
	return is_finite() && finite_vars.empty();
}

/// Checks if two domains are compatible
/**
 * Two domains are compatible iff they contain the same number of variables
 *
 * @param d2 Second domain
 * 
 * @return Whether either this domain and \a d2 are both infinite or have the same number of variables
 */

bool Domain::is_compatible(const Domain& d2) const
{
	const Domain& d1 = *this; 

	return (d1.is_infinite() && d2.is_infinite()) ||
		(d1.is_finite() && d2.is_finite() && d1.size() == d2.size());
}

/// Checks if two domains are disjoint
/**
 * 
 *
 * @param d2 Second domain
 * 
 * @return Whether this domain and \a d2 do not have common variables
 */

bool Domain::is_disjoint(const Domain& d2) const
{
	const Domain& d1 = *this;

	if (d1.is_infinite() && d2.is_infinite())
	{
		// Crude approximation, should be correct but ineffecient
		return d1.first_n(d2.infinite_from + d2.infinite_step).is_disjoint(d2);
	}

	if (d1.is_infinite()) return d2.is_disjoint(d1);

	for (const_iterator i = begin();i != end();++i)
	{
		if (d2(*i)) return false;
	}

	return true;
}

/// Get size of domain
/**
 * Gets the number of variables in the domain. This requires that the domain is finite.
 *
 * @return The number of variables in this domain
 */

unsigned int Domain::size() const
{
	assert(is_finite());

	return finite_vars.size(); 
}

/// Get the lowest variable in domain
/**
 * Requires a non-empty domain
 *
 * @return The lowest variable in domain
 */

Domain::Var Domain::lowest() const
{
	assert(!is_empty());

	return *begin();
}

/// Get the highest variable in domain
/**
 * Requires a non-empty domain
 *
 * @return The highest variable in domain
 */

Domain::Var Domain::highest() const
{
	assert(!is_empty() && is_finite());

	return *(finite_vars.rbegin());
}

/// Get a variable higher then any variable in domain
/**
 * Requires a finite domain
 *
 * @return A variable such that there are no variable in the domain less than this variable
 */

Domain::Var Domain::higher() const
{
	assert(is_finite());

	return is_empty() ? 0 : (highest() + 1);
}

/// Membership test
/**
 * 
 *
 * @param v Variable to test
 * 
 * @return Whether \a v is in the domain
 */
bool Domain::operator()(Var v) const
{
	return is_finite() ? 
		finite_vars.find(v) != finite_vars.end() :
		(v >= infinite_from && ((v - infinite_from) % infinite_step) == 0);
}


/// Get largest domain
/**
 * 
 *
 * @param vs1 First set
 * @param vs2 Second set
 * 
 * @return Either \a vs1 or \a vs2 such that the returned set has at more or the same cardinality as both of them
 */

Domain Domain::sup(const Domain& vs1, const Domain& vs2)
{
	if (!vs1.is_finite()) return vs1;
	if (!vs2.is_finite()) return vs2;

	return vs1.size() < vs2.size() ? vs2 : vs1;
}


/// Union of domains
/**
 * 
 *
 * @param vs1 First domain
 * @param vs2 Second domain
 * 
 * @return The domain containing all the variables that is in \a vs1 or \a vs2
 */

Domain operator|(const Domain& vs1, const Domain& vs2)
{
	assert(vs1.is_finite() && vs2.is_finite());

	Domain new_vs = vs1;
	
	new_vs.finite_vars.insert(vs2.finite_vars.begin(), vs2.finite_vars.end());
	
	return new_vs;
}

/// Intersection of domains
/**
 * 
 *
 * @param vs1 First domain
 * @param vs2 Second domain
 * 
 * @return The domain containing all the variables that is both in \a vs1 and \a vs2
 */

Domain operator&(const Domain& vs1, const Domain& vs2)
{
	assert(vs1.is_finite() || vs2.is_finite());
	
	if(!(vs2.is_finite())) return vs2 & vs1;

	/* vs2 is now finite */

	Domain res;
	Domain::const_iterator i1 = vs1.begin();
	Domain::const_iterator i2 = vs2.begin();
	while(i2 != vs2.end() && i1 != vs1.end())
	{
		Domain::Var v1 = *i1;
		Domain::Var v2 = *i2;

		if (v1 == v2) res.finite_vars.insert(v1);

		if (v2 <= v1) ++i2;
		if (v1 <= v2) ++i1;
	}

	return res;
}

/// Difference
/**
 * 
 *
 * @param vs1 First domain
 * @param vs2 Second domain
 * 
 * @return The domain containing all the variables that is in \a vs1 but not in \a vs2
 */

Domain operator-(const Domain& vs1, const Domain& vs2)
{
	assert(vs1.is_finite() && vs2.is_finite());
	Domain res;

	set_difference(vs1.finite_vars.begin(), vs1.finite_vars.end(), vs2.finite_vars.begin(), vs2.finite_vars.end(),
		       inserter(res.finite_vars, res.finite_vars.begin()));

	return res;
}

/// Union assignment
/**
 * Same as domain = domain | \a vs
 *
 * @param vs Domain to add
 *
 * @return Itself
 */

Domain& Domain::operator|=(const Domain &vs)
{
	*this = *this | vs;
	return *this;
}

/// Intersection assignment
/**
 * Same as domain = domain & \a vs
 *
 * @param vs Domain to intersect
 *
 * @return Itself
 */

Domain& Domain::operator&=(const Domain &vs)
{
	*this = *this & vs;
	return *this;
}

/// Difference assignment
/**
 * Same as domain = domain - vs
 *
 * @param vs Domain to subtract with
 *
 * @return Itself
 */

Domain& Domain::operator-=(const Domain &vs)
{
	*this = *this - vs;
	return *this;
}

/// Multiply every variable in domain
/**
 * 
 *
 * @param v Value to multiply with
 * 
 * @return A domain containing all variables mulitplied by \a v
 */

Domain Domain::operator*(unsigned int v) const
{
	return is_finite() ?
		transform(bind2nd(multiplies<int>(), v)):
		infinite(infinite_from * v, infinite_step * v);
}

/// Divide every variable in domain
/**
 * 
 *
 * @param v Value to divide with
 * 
 * @return A domain containing all variables divided by \a v
 */

Domain Domain::operator/(unsigned int v) const
{
	return transform(bind2nd(divides<int>(), v));
}

/// Add to every variable in domain
/**
 * 
 *
 * @param v Value to add with
 * 
 * @return A domain with all variables added \a v
 */

Domain Domain::operator+(unsigned int v) const
{
	return is_finite() ?
		transform(bind2nd(plus<int>(), v)):
		infinite(infinite_from + v, infinite_step);
}

/// Subtract from every variable in domain
/**
 * 
 *
 * @param v Value to subtract 
 * 
 * @return A domain with all variables subtracted \a v
 */

Domain Domain::operator-(unsigned int v) const
{
	return is_finite() ?
		transform(bind2nd(minus<int>(), v)):
		infinite(infinite_from - v, infinite_step);
}

/// Equality
/**
 * 
 *
 * @param v1 First domain
 * @param v2 Second domain
 * 
 * @return Whether \a v1 and \a v2 are equal.
 */

bool operator==(const Domain& v1, const Domain& v2)
{
	if(v1.is_finite() && v2.is_finite())
	{
		return v1.finite_vars == v2.finite_vars;
	}

	if(!(v1.is_finite()) && !(v2.is_finite()))
	{
		return (v1.infinite_from == v2.infinite_from) &&
			(v1.infinite_step == v2.infinite_step);
	}

	return false;
}

/// Inequality
/**
 * 
 *
 * @param v1 First domain
 * @param v2 Second domain
 * 
 * @return Whether \a v1 and \a v2 are not equal.
 */

bool operator!=(const Domain& v1, const Domain& v2)
{
	return !(v1 == v2);
}

/// Get a domain with the lowest variables
/**
 * 
 *
 * @param n How many variables in the new domain
 * 
 * @return A new domain with the \a n lowest variables
 */

Domain Domain::first_n(unsigned int n) const
{
	Domain res;
	const_iterator i = begin();
	while(n > 0 && i != end())
	{
		res.finite_vars.insert(*i);
		++i;
		--n;
	}

	return res;
}

/// Get a domain with the highest variables
/**
 * 
 *
 * @param n How many variables in the new domain
 * 
 * @return A new domain with the \a n highest variables
 */


Domain Domain::last_n(unsigned int n) const
{
	assert(is_finite());

	Domain res;
	set<Var>::const_reverse_iterator i = finite_vars.rbegin();

	while(n > 0 && i != finite_vars.rend())
	{
		res.finite_vars.insert(*i);
		++i;
		--n;
	}

	return res;
}

/// Remove highest variables to get same size as another domain
/**
 * 
 *
 * @param d Domain to compare with
 * 
 * @return A domain with the lowest variables keeping as many as there are in \a d
 */
Domain Domain::cut_to_same_size(const Domain& d) const
{
	return first_n(d.size());
}

/// Uninitialized constructor
Domain::const_iterator::const_iterator()
{}

/// Copy constructor
/**
 * @param i Iterator to copy
 */
Domain::const_iterator::const_iterator(const const_iterator& i):
	vs(i.vs),
	infinite_v(i.infinite_v),
	finite_i(i.finite_i)
{}

/// Constructor for infinite domains
/**
 * @param vs Domain to iterate over
 * @param infinite_v Current variable
 */
Domain::const_iterator::const_iterator(const Domain& vs, Var infinite_v):
	vs(&vs),
	infinite_v(infinite_v)
{}

/// Constructor for finite domains
/**
 * @param vs Domain to iterate over
 * @param finite_i Iterator of finite set
 */
Domain::const_iterator::const_iterator(const Domain& vs, set<Var>::const_iterator finite_i):
	vs(&vs),
	finite_i(finite_i)
{}
					      
/// Get iterator pointing to beginning of set
/**
 * @param vs Domain to iterate over
 *
 * @return Iterator pointing to beginning of \a vs
 */
Domain::const_iterator Domain::const_iterator::begin(const Domain& vs)
{
	return vs.is_finite() ? 
		const_iterator(vs, vs.finite_vars.begin()) : 
		const_iterator(vs, vs.infinite_from);
}

/// Get iterator pointing to end of set
/**
 * @param vs Domain to iterate over
 *
 * @return Iterator pointing to end of \a vs
 */
Domain::const_iterator Domain::const_iterator::end(const Domain& vs)
{
	return vs.is_finite() ?
		const_iterator(vs, vs.finite_vars.end()):
		const_iterator(vs, (unsigned int)-1);
}

/// Dereference
/**
 * @return The variable currently pointed to by this iterator
 */
Domain::Var Domain::const_iterator::operator*() const
{
	return vs->is_finite() ? *finite_i : infinite_v;
}

/// Increment
/**
 * Increments iterator to point to next value
 */

Domain::const_iterator& Domain::const_iterator::operator++()
{
	if (vs->is_finite())
	{
		++finite_i;
	}
	else
	{
		infinite_v += vs->infinite_step;
	}

	return *this;
}

/// Equality
/**
 * @param i1 First iterator
 * @param i2 Second iterator
 * 
 * @return Whether \a i1 and \a i2 are equal
 */
bool operator==(Domain::const_iterator i1, Domain::const_iterator i2)
{
	return i1.vs->is_finite() ? (i1.finite_i == i2.finite_i) : (i1.infinite_v == i2.infinite_v);
}

/// Inequality
/**
 * @param i1 First iterator
 * @param i2 Second iterator
 * 
 * @return Whether \a i1 and \a i2 are not equal
 */
bool operator!=(Domain::const_iterator i1, Domain::const_iterator i2)
{
	return !(i1 == i2);
}

/// Get iterator pointing to beginning of set
/**
 * @return Iterator pointing to beginning
 */
Domain::const_iterator Domain::begin() const
{
	return Domain::const_iterator::begin(*this);
}

/// Get iterator pointing to end of set
/**
 * @return Iterator pointing to end
 */
Domain::const_iterator Domain::end() const
{
	return Domain::const_iterator::end(*this);
}

/**
 * map_vars:
 * @param vs1 First set of variables
 * @param vs2 Second set of variables
 *
 * Build mapping between set of variables. Size of \a vs1 must be equal
 * to the size of \a vs2.
 *
 * @return Mapping from \a vs1 to \a vs2, identity mapping for variables not in \a vs1
 */

Domain::VarMap Domain::map_vars(const Domain &vs1, const Domain &vs2)
{
	assert(vs1.is_finite() && vs2.is_finite());

	Domain::VarMap v_map;

	Domain::iterator i1 = vs1.begin();
	Domain::iterator i2 = vs2.begin();

	while(i1 != vs1.end())
	{
		if (!(i2 != vs2.end()))
		{
			std::cerr << "vs1: " << vs1 << " \nvs2: " << vs2 << std::endl;
			assert(i2 != vs2.end());
		}

		v_map[*i1] = *i2;

		++i1;
		++i2;
	}

	assert(i2 == vs2.end());

	return v_map;
}

/// Sends a textual description of a domain into a stream
/**
 * 
 *
 * @param s Stream to send to
 * @param vs Domain to describe
 * 
 */

ostream& operator<< (ostream &s, const Domain &vs)
{
	Domain::iterator i = vs.begin();

	cout << "{";

	if (i != vs.end())
	{
		cout << *i;
		++i;

		while (i != vs.end())
		{
			cout << "," << *i;
			++i;
		}
	}

	cout << "}";

	return s;
}

/// Create a sequence of empty domains
/**
 * 
 *
 * @param n Number of domains
 */

Domains::Domains(unsigned int n):
	doms(n)
{}

/// Create domains with one element
/**
 * 
 *
 * @param d1 The domain
 */

Domains::Domains(Domain d1):
	doms(1, d1)
{}

/// Check if some domain is infinite
/**
 * 
 *
 * @return Whether some domain is infinite
 */

bool Domains::is_some_infinite() const
{
	const_iterator i;
	for(i = begin();i != end();++i)
	{
		if(!i->is_finite()) return true;
	}

	return false;
}

/// Cross product of domains
/**
 * 
 *
 * @param ds1 First sequence of domains
 * @param ds2 Second sequence of domains
 * 
 * @return A sequence of domains \a ds1 concatenated by \a ds2
 */
Domains operator*(Domains ds1, Domains ds2)
{
	ds1.doms.insert(ds1.doms.end(), ds2.doms.begin(), ds2.doms.end());

	return ds1;
}

/// Get one of the domains
/**
 * 
 *
 * @param idx Index of domain to get
 * 
 * @return The domain on position \a idx
 */

Domain& Domains::operator[](unsigned int idx)
{
	assert(idx >= 0 && idx < size());
	return doms[idx];
}

/// Get one of the domains
/**
 * 
 *
 * @param idx Index of domain to get
 * 
 * @return The domain on position \a idx
 */

const Domain& Domains::operator[](unsigned int idx) const
{
	return doms[idx];
}

/// Equality
/**
 * 
 *
 * @param ds1 First vector of domains
 * @param ds2 First vector of domains
 * 
 * @return Whether \a ds1 and \a ds2 are equal.
 */

bool operator==(Domains ds1, Domains ds2)
{
	return ds1.doms == ds2.doms;
}

/// Get largest domain for each component
/**
 *
 * @param ds1 First vector of domains
 * @param ds2 Second vector of domains
 * 
 * @return Domains with sup(d1, d2) for each domain d1 in \a ds1 and corresponding d2 in \a ds2
 */

Domains Domains::sup(const Domains& ds1, const Domains& ds2)
{
	assert(ds1.size() == ds2.size());

	Domains res;

	for (unsigned int i = 0;i < ds1.size();++i)
	{
		res = res * Domain::sup(ds1[i], ds2[i]);
	}
	
	return res;
}

/// Get union of all domains
/**
 * 
 *
 * @return The union of all domains
 */
Domain Domains::union_all() const
{
	Domain all;

	vector<Domain>::const_iterator i;

	for(i = doms.begin();i != doms.end();++i)
	{
		all |= *i;
	}

	return all;
}

/// Check if other domains are disjoint
/**
 * 
 *
 * @param ds2 Other domains
 * 
 * @return Whether all variables in this set of domains are not in any domain from \a ds2
 */

bool Domains::is_disjoint(const Domains& ds2) const
{
	for (const_iterator i = begin();i != end();++i)
	{
		for (const_iterator j = ds2.begin();j != ds2.end();++j)
		{
			if (!i->is_disjoint(*j)) return false;
		}
	}

	return true;
}


/// Intersect domains with another domain
/**
 * All domains are intersected with \a d
 *
 * @param d Domain to insersect with
 */
Domains Domains::intersect_with(Domain d) const
{
	Domains res = *this;
	unsigned int i;
	for(i = 0;i < res.size();++i)
	{
		res[i] &= d;
	}

	return res;
}

/// Cut domains to the same size as another sequence of domains
/**
 * 
 *
 * @param ds Sequence of domains to compare with
 * 
 * @return Every domain is cut to the same size as the corresponding domain in \a ds
 */
Domains Domains::cut_to_same_sizes(Domains ds) const
{
	Domains res = *this;
	unsigned int i;
	for(i = 0;i < res.size();++i)
	{
		res[i] = res[i].cut_to_same_size(ds[i]);
	}

	return res;
}

}
