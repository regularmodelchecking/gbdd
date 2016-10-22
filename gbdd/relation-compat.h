/*
 * relation-compat.h: 
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
#ifndef RELATION_COMPAT_H
#define RELATION_COMPAT_H

#include <gbdd/bdd-relation.h>
#include <gbdd/bdd-equivalence-relation.h>

namespace gbdd
{
	typedef BddRelation Relation;
	typedef BddSet Set;
	typedef BddBinaryRelation BinaryRelation;
	typedef BddEquivalenceRelation EquivalenceRelation;

	typedef StructureConstraint BddBased;
	typedef StructureRelation BddBasedRelation;
	typedef StructureSet BddBasedSet;
	typedef StructureBinaryRelation BddBasedBinaryRelation;
}	

#endif /* RELATION_COMPAT_H */
