/*
 * structure-binary-relation.h: 
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

#ifndef STRUCTURE_BINARY_RELATION_H
#define STRUCTURE_BINARY_RELATION_H

#include <gbdd/structure-relation.h>

namespace gbdd
{
	template <class StructureT, class RelationT, class SetT>
	class StructureBinaryView : public RelationT
	{
	public:
		StructureBinaryView(const StructureRelation& r) : RelationT(r) {}
		StructureBinaryView() : RelationT() {}
		StructureBinaryView(const Domain& domain1, const Domain& domain2, const StructureT& bb) : RelationT(domain1 * domain2, bb) {}
		StructureBinaryView(const Domain& domain1, const Domain& domain2, const StructureBinaryView& s) : RelationT(domain1 * domain2, s) {}

		~StructureBinaryView() {}

		StructureBinaryView operator&(const StructureBinaryView& s2) const
			{
				return static_cast<const StructureRelation&>(*this) & static_cast<const StructureRelation&>(s2);
			}

		StructureBinaryView operator|(const StructureBinaryView& s2) const
			{
				return static_cast<const StructureRelation&>(*this) | static_cast<const StructureRelation&>(s2);
			}

		StructureBinaryView operator-(const StructureBinaryView& s2) const
			{
				return static_cast<const StructureRelation&>(*this) - static_cast<const StructureRelation&>(s2);
			}
		
		StructureBinaryView operator!() const
			{
				return StructureRelation::operator!();
			}

		StructureBinaryView restrict_range(const SetT& s) const
			{
				return StructureBinaryView(StructureRelation::restrict(0, s));
			}
		
		StructureBinaryView restrict_image(const SetT& s) const
			{
				return StructureBinaryView(StructureRelation::restrict(1, s));
			}
		
		StructureBinaryView inverse() const
			{
				return StructureBinaryView(get_domain(1), get_domain(0), get_bdd_based());
			}
		
		static StructureBinaryView cross_product(const Domain& domain1,
							const Domain& domain2,
							const SetT& set1,
							const SetT& set2)
			{
				vector<SetT> sets;
				
				sets.push_back(set1);
				sets.push_back(set2);
				
				return StructureRelation::cross_product(domain1 * domain2, sets);
			}
		
		SetT range() const
			{
				return project_on(0);
			}
		
		SetT image() const
			{
				return project_on(1);
			}
		
		SetT image_under(const SetT& s) const
			{
				return restrict_range(s).image();
			}
		
		SetT range_under(const SetT& s) const
			{
				return restrict_image(s).range();
			}
	};
	
	typedef StructureBinaryView<StructureConstraint, StructureRelation, StructureSet> StructureBinaryRelation;
}

#endif /* STRUCTURE_BINARY_RELATION_H */
