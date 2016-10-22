/*
 * bool-constraint.h: 
 *
 * Copyright (C) 2004 Marcus Nilsson (marcusn@it.uu.se)
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
#ifndef BOOL_CONSTRAINT_H
#define BOOL_CONSTRAINT_H

#include <gbdd/structure-constraint.h>

namespace gbdd
{
/**
 * A constraint on boolean variables
 * 
 */
	class BoolConstraint : public StructureConstraint
	{
	public:
		class Factory : public StructureConstraint::Factory
		{
		public:
			virtual ~Factory();
			virtual BoolConstraint* ptr_forall(const BoolConstraint& c) const;
			virtual BoolConstraint* ptr_constant(bool v) const = 0;
			virtual BoolConstraint* ptr_var(Var v, bool var_v) const = 0;

			BoolConstraint* ptr_value(const Domain& vs, unsigned int v) const;
			BoolConstraint* ptr_value_range(const Domain& vs, unsigned int from_v, unsigned int to_v) const;
			BoolConstraint* ptr_vars_equal(const Domain& vs1, const Domain& vs2) const;
		};

		virtual Factory* ptr_factory() const = 0;

		virtual BoolConstraint* ptr_convert(const Factory& f) const = 0;
	};
}
		
#endif /* BOOL_CONSTRAINT_H */
