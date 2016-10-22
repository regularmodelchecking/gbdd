/*
 * test-relation.cc: 
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

#include <gbdd/gbdd.h>
#include <iostream>

using namespace gbdd;

typedef Bdd::Var Var;
typedef Space::VarMap VarMap;

static Space *space = Space::create_default();

static bool test_composition()
{
	Bdd::Vars x(space); 
	Bdd::FiniteVars y = x[Domain(0,5) * Domain(5,5)];
	Bdd::FiniteVars z = x[Domain(3,5) * Domain(9,5)];

	BddRelation rel1(y, (y[0] == 0 | y[0] == 2 | y[0] == 5) & y[1] == 10);
	BddRelation rel2(y, (y[0] == 0 | y[0] == 1 | y[0] == 2) & y[1] == 10);

	BddRelation mapper(z,
			   (z[0] == 0 & z[1] == 0) |
			   (z[0] == 2 & z[1] == 1) |
			   (z[0] == 5 & z[1] == 2));
	
	BddRelation composed_rel1 = rel1.compose(0, mapper);

	return (composed_rel1 == rel2);
}

static bool test_intersection()
{
	Bdd::Vars x(space);
	Bdd::FiniteVars y = x[Domain(0,5) * Domain(5,5)];
	Bdd::FiniteVars z = x[Domain(3,5) * Domain(9,5)];

	BddRelation rel1 = BddRelation(y.get_domains(), y[0] == 1 & (y[1] == 2 | y[1] == 3));
	BddRelation rel2 = BddRelation(y.get_domains(), y[0] == 1 & y[1] == 2);
	BddRelation rel3 = BddRelation(z.get_domains(), z[0] == 1 & z[1] == 2);

 	return (!((rel1 & rel2).is_false())) &&
 		((rel1 & rel2) == (rel1 & rel3));
}

static bool test_sets()
{
	BddSet s(Domain(2,3), Bdd(space, true));

	BddSet::const_iterator i = s.begin();
	
	unsigned int should_be = 0;

	while(i != s.end())
	{
		if (should_be != *i) return false;

		should_be++;
		++i;
	}

	return should_be == 8;
}

static bool test_sets_ops()
{
	BddSet s(Domain(2, 3), Bdd(space, false));
	BddSet s3 = BddSet(s, 3);
	BddSet s5 = BddSet(s, 3);

	s |= BddSet(s, 3);
	s |= BddSet(s, 5);

	BddSet::const_iterator i = s.begin();
	
	if (i == s.end()) return false;
	if (*i != 3) return false;

	++i;

	if (i == s.end()) return false;
	if (*i != 5) return false;

	++i;

	return (i == s.end());
}

static bool test_sets_insert()
{
	BddSet s1(space);

	s1.insert(6);
	s1.insert(15);

	BddSet s2(Domain(0,4), Bdd(space, false));

	s2 |= BddSet(s2, 6);
	s2 |= BddSet(s2, 15);

	BddSet s3(space);
	
	s3.insert(6);

	BddSet s4(space);
	s4.insert(15);

	BddSet s5(space);
	s5.insert(5);
	s5.insert(6);
	s5.insert(7);
	s5.insert(8);
	BddSet s6(space, 5, 8);

	return s1 == s2 && (s3|s4) == s1 && s5 == s6;
}

static bool test_relations_insert()
{
	BddRelation r1(space, 2);

	r1.insert(1, 5);
	r1.insert(2, 6);

	Bdd::Vars x(space); 
	Bdd::FiniteVars y = x[Domain(0,4) * Domain(4,8)];

	BddRelation r2(y.get_domains(), 
		       (y[0] == 1 & y[1] == 5) |
		       (y[0] == 2 & y[1] == 6));

	return r1 == r2;
}

static bool test_identity()
{
	Bdd::Vars x(space);
	Bdd::FiniteVars z = x[Domain(0,2) * Domain(2,2)];

	BddEquivalenceRelation id =
		BddEquivalenceRelation::identity(space, z[0].get_domain(), z[1].get_domain());

	BddSet s0(z[0].get_domain(), z[0] == 0);
	BddSet s1(s0, 1);
	BddSet s2(s0, 2);
	BddSet s3(s0, 3);

	StructureSet im_s0 = id.image_under(s0);
	StructureSet ra_s3 = id.range_under(s3);

	return 
		(id.image_under(s0) == s0) &&
		(id.image_under(s1) == s1) &&
		(id.image_under(s2) == s2) &&
		(id.image_under(s3) == s3) &&
		(id.image_under(s1 | s2) == (s1 | s2)) &&
		(id.range_under(s0) == s0) &&
		(id.range_under(s1) == s1) &&
		(id.range_under(s2) == s2) &&
		(id.range_under(s3) == s3) &&
		(id.range_under(s1 | s2) == (s1 | s2));
}
		
static bool test_equivalence()
{
	Bdd::Vars x(space);
	Bdd::FiniteVars z = x[Domain(0,2) * Domain(2,2)];

	BddEquivalenceRelation id =
		BddEquivalenceRelation::identity(space, z[0].get_domain(), z[1].get_domain());

	BddEquivalenceRelation is_even =
		BddEquivalenceRelation(z[0].get_domain(), z[1].get_domain(),
				       (z[0] == 0 | z[0] == 2) &
				       (z[1] == 0 | z[1] == 2));

	BddEquivalenceRelation is_odd =
		BddEquivalenceRelation(z[0].get_domain(), z[1].get_domain(),
				       (z[0] == 1 | z[0] == 3) &
				       (z[1] == 1 | z[1] == 3));

	BddEquivalenceRelation is_evenodd =
		BddEquivalenceRelation(is_even|is_odd);

	BddSet all(z[0].get_domain(), Bdd(space, true));
	BddSet even = BddSet(all, 0) | BddSet(all, 2);
	BddSet odd = BddSet(all, 1) | BddSet(all, 3);

	vector<BddSet> quotient_evenodd = (is_evenodd).quotient(all);
	vector<BddSet> quotient_id = id.quotient(all);

	return 
		(quotient_id.size() == 4) &&
		((quotient_evenodd[0] == even && quotient_evenodd[1] == odd) ||
		 (quotient_evenodd[0] == odd && quotient_evenodd[1] == even));
}

static bool test_infinite()
{
	Bdd::Vars x(space);
	Bdd::FiniteVars z = x[Domain(0, 10, 2) * Domain(1, 10, 2)];

	Domain domain1 = Domain::infinite(0, 2);
	Domain domain2 = Domain::infinite(1, 2);

	Bdd encode_1 = z[0] == 0;
	Bdd encode_2 = z[1] == 0;

	return
		BddRelation(domain1, encode_1) == BddRelation(domain2, encode_2) &&
		BddSet(BddRelation(domain1 * domain2, encode_1 & encode_2).project_on(0)).get_bdd() == encode_1;
}

       
int main(int argc, char **argv)
{
	struct
	{
		const char *name;
		bool (*test_f)(void);
	}
	tests[] =
	{
		{"Composition", test_composition},
		{"Intersection", test_intersection},
		{"Sets", test_sets},
		{"Sets ops", test_sets_ops},
		{"Sets insert", test_sets_insert},
		{"Relations insert", test_relations_insert},
		{"Identity relation", test_identity},
		{"Equivalence relation", test_equivalence},
		{"Infinite domains", test_infinite}
	};

	unsigned int i;

	for (i = 0;i < sizeof(tests) / sizeof(tests[0]);i++)
	{
		cout << tests[i].name << "...";
		cout.flush();

		cout << (tests[i].test_f() ? "Ok" : "Fail");

		cout << endl;
	}

}
