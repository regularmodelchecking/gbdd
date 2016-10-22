/*
 * structure-constraint.cc: 
 *
 * Copyright (C) 2002 Marcus Nilsson (marcus@docs.uu.se)
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
 *    Marcus Nilsson (marcus@docs.uu.se)
 */

#include <gbdd/structure-constraint.h>

namespace gbdd
{
	
StructureConstraint::Factory::~Factory()
{}

/// Create pool of variables with all variables available
StructureConstraint::VarPool::VarPool() : 
	vars_allocated()
{}


/// Try to allocate a set of variables
/**
 * @param vs Sets of variables to allocate
 * 
 * @return true if variables could be allocated, false if allocation failed because some of the variables were already allocated 
 */
		
bool StructureConstraint::VarPool::alloc(const Domain &vs)
{
	if ((vars_allocated & vs).is_empty())
	{
		vars_allocated |= vs;
		return true;
	}
	else
	{
		return false;
	}
}

/// Allocate a number of variables
/**
 * @param n_vars Number of variables to allocate
 * 
 * @return Set of \p n_vars variables (consequtive)
 */

Domain
StructureConstraint::VarPool::alloc(unsigned int n_vars)
{
	Domain::const_iterator i = vars_allocated.begin();
	Domain hole;

	// Find hole

	if (i == vars_allocated.end())
	{
		hole = Domain(0, n_vars);
	}
	else
	{
		Domain::const_iterator j = i;
		++j;

		while (j != vars_allocated.end() &&
			(*j - *i - 1) <= n_vars)
		{
			++i;
			++j;
		}

		if (j == vars_allocated.end())
		{
			hole = Domain(*i + 1, n_vars);
		}
		else
		{
			hole = Domain(*i + 1, n_vars);
		}
	}

	(void)alloc(hole);

	return hole;
}

/// Allocate chunks of variables interleaved	
/**
 * @param n_vars_per_chunk Number of variables per chunk
 * @param n_chunks Number of chunks to allocate
 * 
 * @return A vector of chunks of variables allocated
 */

Domains
StructureConstraint::VarPool::alloc_interleaved(unsigned int n_vars_per_chunk,
				unsigned int n_chunks)
{
	Domain vs = alloc(n_vars_per_chunk * n_chunks);

	Domains v_vs(n_chunks);
	
	unsigned int i,j;
	Domain::const_iterator next_var = vs.begin();

	for (i = 0;i < n_vars_per_chunk;++i)
	{
		for (j = 0;j < n_chunks;++j)
		{
			v_vs[j] |= Domain(*next_var);
			++next_var;
		}
	}

	return v_vs;
}


}
