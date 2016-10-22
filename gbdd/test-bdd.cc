/*
 * test-hash.cc: 
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

typedef Space::Var Var;
typedef Space::VarMap VarMap;

static Space *space = Space::create_default();

static bool test_valuemember()
{
	Bdd::Vars x(space);

	Domain vs(0,8);
	Bdd::FiniteVar z1 = x[vs];
	
	Bdd p = z1 == 2 | z1 == 3;

	return p.value_member(vs, 2) &&
		p.value_member(vs, 3) &&
		!(p.value_member(vs, 5)) &&
		!(p.value_member(vs, 0)) &&
		!(p.value_member(vs, 1)) &&
		!(p.value_member(vs, 6)) &&
		!(p.value_member(vs, 7));
}

static bool test_assignments()
{
	Bdd::Vars x(space);
	
	Domain vs(0, 8);
	Domain vs_3(2, 3);

	Bdd p = x[2] | x[3];
	Bdd q = x[2] & x[3];


	set<unsigned int> s;
	s.insert(1);
	s.insert(2);
	s.insert(3);
	s.insert(5);
	s.insert(6);
	s.insert(7);

	return (p.n_assignments(vs) == (3 * 64)) &&
		(q.n_assignments(vs) == 64) &&
		p.assignments_value(vs_3) == s;
}

static bool test_product()
{
	Bdd::Vars x(space);
	Bdd::FiniteVar z1 = x[Domain(0,4)];

	Bdd p = z1 == 2 | z1 == 3;

	Bdd q = z1 == 3 | z1 == 4;

	return (p & q) == (z1 == 3);
}

static bool test_varalloc()
{
	Bdd::VarPool pool;

	Domain vs1 = pool.alloc(5);

	Domains v_vs = pool.alloc_interleaved(3, 2);

	Domain vs2 = pool.alloc(5);

	return
		(vs1 == Domain(0, 5)) &&
		(v_vs[0] == Domain(5, 3, 2)) &&
		(v_vs[1] == Domain(6, 3, 2)) &&
		(vs2 == Domain(11, 5));
}

static bool test_rename()
{
	Bdd::Vars x(space);

	Bdd::VarPool pool;

	Bdd::FiniteVars z = x[pool.alloc_interleaved(10, 2)];

	Bdd bdd_encode_0 = z[0] == 73;
	Bdd bdd_encode_1 = z[1] == 73;

	VarMap map = Domain::map_vars(z[0].get_domain(), z[1].get_domain());

	return (bdd_encode_1 == bdd_encode_0.rename(map));
}

static bool test_project()
{
	Bdd::Vars x(space);

	Bdd p = x[2] & x[3];

	Bdd q = x[2];

	return (p.project(Domain(3)) == q);
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
		{"Value membership", test_valuemember},
		{"Assignments", test_assignments},
		{"Variable allocation", test_varalloc},
		{"Rename", test_rename},
		{"Product", test_product},
		{"Projection", test_project}
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
