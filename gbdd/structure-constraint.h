/*
 * structure-constraint.h: 
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

#ifndef STRUCTURE_CONSTRAINT_H
#define STRUCTURE_CONSTRAINT_H

#include <gbdd/domain.h>
#include <memory>

namespace gbdd
{
	using gbdd::Domain;

	class BoolConstraint;
	
/**
 * A structure based on BDDs
 * 
 */
	class StructureConstraint
	{
	public:
		virtual ~StructureConstraint() {}

		/// Pool of variables that can be allocated in different ways
		/**
		 * An instance of VarPool is a set of variables that can be allocated in different ways. This is useful if
		 * one does not care exactly which variables you want, or if one wants to make sure that the variables do
		 * not get resused.
		 */ 
		class VarPool
		{
			Domain vars_allocated;
		public:
			VarPool();
			
			
			bool alloc(const Domain &vs);
			
			Domain alloc(unsigned int n_vars);
			
			Domains alloc_interleaved(unsigned int n_vars_per_chunk,
						  unsigned int n_chunks);
			
		};

		static bool fn_or(bool v1, bool v2) { return v1 || v2; }
		static bool fn_and(bool v1, bool v2) { return v1 && v2; }
		static bool fn_neq(bool v1, bool v2) { return v1 != v2; }
		static bool fn_iff(bool v1, bool v2) { return (v1 && v2) || (!v1 && !v2); }
		static bool fn_implies(bool v1, bool v2) { return !v1 || v2; }
		static bool fn_minus(bool v1, bool v2) { return (v1 && !v2); }

		typedef Domain::VarMap VarMap;
		typedef Domain::Var Var;
/**
 * Factory for structures
 * 
 */

		class Factory
		{
		public:
			virtual ~Factory();
/// Create constraint for any structure with a boolean constraint
/**
 * 
 *
 * @param c Constraint which should hold at every position
 * 
 * @return A constraint satisfying all structure where \a c holds at every position
 */
			virtual StructureConstraint* ptr_forall(const BoolConstraint& c) const = 0;
		};


		virtual Factory* ptr_factory() const = 0;

/// Rename variables
/**
 * @param map Renaming
 * 
 * @return The structure constraint object renamed with \a map
 */
		virtual StructureConstraint* ptr_rename(VarMap map) const = 0;

/// Rename variables
/**
 * \a vs1 and \a vs2 must be of the same size
 *
 * @param vs1 First set of variables
 * @param vs2 Second set of variables
 * 
 * @return Thee structure constraint object renamed such that vs1[i] is renamed to vs2[i]
 */

		StructureConstraint* ptr_rename(const Domain& vs1, const Domain& vs2) const
			{
				return ptr_rename(Domain::map_vars(vs1, vs2));
			}
		
/// Project variables
/**
 * 
 *
 * @param vs Domain to project
 * 
 * @return The structure constraint object with the variables in \a vs projected away
 */
		virtual StructureConstraint* ptr_project(Domain vs) const = 0;

/// Constrain variable
/**
 * @param v Variable to constrain
 * @param value Value to constrain \a v with
 * 
 * @return The structure constraint object with \a set to \a value
 */
		virtual StructureConstraint* ptr_constrain_value(Var v, bool value) const = 0;
		
/// Product
/**
 * 
 *
 * @param b2 structure constraint object to take product with
 * @param fn Product function
 * 
 * @return Product of this object and \a b2 with respect to \a fn
 */
		virtual StructureConstraint* ptr_product(const StructureConstraint& b2, bool (*fn)(bool v1, bool v2)) const = 0;

/// Negation
/**
 * @return Negation of this object
 */
		virtual StructureConstraint* ptr_negate() const = 0;

/// Cloning
/**
 * @return A copy of this object
 */
		virtual StructureConstraint* ptr_clone() const = 0;

/// Get highest variable occuring in this object
/**
 * @return The highest variable occuring in this object, or 0 if none
 */
		virtual Var highest_var() const = 0;

/// Get lowest variable occuring in this object
/**
 * @return The lowest variable occuring in this object, or 0 if none
 */
		virtual Var lowest_var() const = 0;

/// Equality
/**
 * 
 *
 * @param b2 structure constraint object to compare with
 * 
 * @return Whether this object and \a b2 are equivalent
 */
		virtual bool operator==(const StructureConstraint& b2) const = 0;
	};
}

#endif /* STRUCTURE_CONSTRAINT_H */
