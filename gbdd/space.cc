/*
 * space.cc: 
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


#include <gbdd/space.h>
#include <gbdd/gspace.h>
#include <gbdd/buddy-space.h>
#include <gbdd/cudd-space.h>
#include <gbdd/gspace.h>

#ifdef GBDD_WITH_BUDDY
#include "buddy.h"
#endif

namespace gbdd
{

string Space::Error::description() const
{
	return desc;
}

Space* Space::create_default(bool diagnostics)
{
#if defined (GBDD_WITH_CUDD)
	return new CuddSpace();
#elif defined (GBDD_WITH_BUDDY)
	Space* s = new BuddySpace();

	if (diagnostics)
	{
		bdd_gbc_hook(bdd_default_gbchandler);
	}
	else
	{
		bdd_gbc_hook(NULL);
	}

	return s;
#else
	return new GSpace();
#endif
}

void Space::lock_gc()
{
	return;
}

void Space::unlock_gc()
{
	return;
}

/// Find highest variable in BDD
/**
 * 
 *
 * @param p Bdd to find highest variable of
 * @param cache Result cache
 * 
 * @return Highest variable in BDD, or 0 if none
 */
Space::Var Space::bdd_highest_var(Bdd p, hash_set<Bdd>& cache)
{
	if (cache.find(p) != cache.end()) return 0;
	cache.insert(p);

	return
		bdd_is_leaf(p) ?
		0:
		max(bdd_var(p), 
		    max(bdd_highest_var(bdd_then(p), cache),
			bdd_highest_var(bdd_else(p), cache)));
}

/// Find highest variable in BDD
/**
 * 
 *
 * @param p Bdd to find highest variable of
 * 
 * @return Highest variable in BDD, or 0 if none
 */
Space::Var Space::bdd_highest_var(Bdd p)
{
	hash_set<Space::Bdd> cache;

	return bdd_highest_var(p, cache);
}

/// Get number of nodes in Space
/**
 * @return The number of nodes currently used in space
 */
unsigned int Space::get_n_nodes(void) const
{
	return 0;
}

typedef Space::VarMap VarMap;

/// Union of maps
/**
 * If a key occurs in \a map1 and \a map2 only one of them will be used
 * 
 * @param map1 First map
 * @param map2 Second map
 * 
 * @return Union of the two maps \a map1 and \a map2
 */
VarMap operator|(const VarMap& map1, const VarMap& map2)
{
	VarMap res = map1;

	res.insert(map2.begin(), map2.end());

	return res;
}

}
