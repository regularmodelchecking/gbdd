/*
 * bdd-equivalence-relation.h: 
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
#ifndef BDD_EQUIVALENCE_RELATION_H
#define BDD_EQUIVALENCE_RELATION_H

#include <gbdd/bdd-relation.h>

namespace gbdd
{
	/// Special case of BinaryRelation
	/**
	 * An equivalence relation is a relation which is
	 * reflexive, symmetric and transitive. It partitions the
	 * set it operates on into a set of partitions, where
	 * two members are related iff they are in the same partition.
	 *
	 * This class offers BddEquivalenceRelation::quotient, which can
	 * be used to find the partitions of a set under an equivalence relation.
	 *
	 */
	class BddEquivalenceRelation : public BddBinaryRelation
	{
	public:
		BddEquivalenceRelation(const StructureRelation& r);

		BddEquivalenceRelation(const Domain& domain1,
				       const Domain& domain2,
				       const BddEquivalenceRelation& r);

		BddEquivalenceRelation(const Domain &domain1,
				       const Domain &domain2,
				       Bdd rel_bdd);

		friend BddEquivalenceRelation
		operator&(const BddEquivalenceRelation& rel1,
			  const BddEquivalenceRelation& rel2); 
		
		BddEquivalenceRelation operator!() const;
		
		BddEquivalenceRelation restrict(const BddSet& s) const;
		
		static BddEquivalenceRelation identity(Space* space,
						       const Domain &domain1,
						       const Domain &domain2);

		vector<BddSet> quotient(const BddSet& s) const;
	};
}

#endif /* BDD_EQUIVALENCE_RELATION_H */
