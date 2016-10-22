/*
 * doxygen_mainpage.h: 
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
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#ifndef DOXYGEN_MAINPAGE_H
#define DOXYGEN_MAINPAGE_H

/** \mainpage GBDD - A package for representing relations with BDDs

\section introduction Introduction

Binary Decision Diagrams are a way to represent predicates on a number
of boolean variables, i.e., they represent a set of assignments to
these variables. By using encoding, any finite set can be represented. 

BDDs are good when there is some structure of the set, giving a small
representation of large sets. A BDD is a binary tree representing
choices on assignments of boolean variables. Each node represents a
variable and each choice (left or right) represents the assignments 0
and 1. If it does not matter what the assignment of the variable is,
i.e., if an assignment is or is not in the set independently of the
value of the variable it is not included in the tree. The leaves of a
BDD is a boolean variable saying whether the assignment represented by
traversing from the root to the leaf is in the set.

Since (finite) sets can be represented using BDDs, so can
relations. Usually, the set of variables are partitioned such that
different variables represent different components of the
relation. Normal relation operators such as image, range, composition
etc. can be implemented using operations on BDDs.

When many relations of different types exist and many different types
of operations are performed, the complexity of keeping track of which
variables represent what becomes harder. Imagine taking the image R(S)
of a binary relation R under a set S. The binary relation R have two
components, each represented by a set of variables V1 and V2, and the
set S is represented by a set of variables V3. The image is contructed
by intersecting the BDD representing the relation R with the BDD
representing S, constraining the first compoment of the relation R to
the set S, and projecting on the second component. For this to work,
the set of variables V1 used to represent the first compoment of R
must be the same as the set of variables V3 used to represent S. If
they are not the same, the BDD representing S has to be renamed such
that each variable in V3 is represented with the corresponding
variable in V1. With complex expressions involving relations of
different arities, the bookkeeping of variables becomes a tedious
task.

The package GBDD is designed to perform this bookkeeping of variables
automatically, providing an abstration on top of BDDs that is simply
finite relations . A relation is defined to be a BDD and a vector of
domains, sets of variables, used in the BDD to represent each
component of the relation. When operations such as composition, image etc. are
performed, renamings are performed automatically if needed. Also,
there is support for allocating variables from a set in different
ways, releiving the user from having to specify exactly what variables
should be used to represent different domains.

Furthermore, more than one BDD package can be plugged into GBDD
providing one unified API for all BDD packages. This makes it easy to
change BDD packages. Currently, A (slow) reference implementation and
an interface to the BuDDy BDD package has been implemented as well
as the CUDD BDD package.

\section spaces Spaces and BDDs

In GBDD, a space refers to one implementation instance of BDDs. All BDDs coming
from a single space shares the common implementation, ensuring the canonicity
of BDDs. Comparing BDDs from two separate spaces has no meaning. Two different
spaces can even have two different implementations.

Spaces are created by the constructor of the desired BDD
implementation, or by using the create_default method of the ancestor
class gbdd::Space, which currently creates a BuDDy space (if compiled in).

A space contains methods for creating BDDs, and all the operations on
BDDs such as product, renaming etc. Added a new BDD implementation or
adding support for an existing BDD package involves subclassing Space
and implementating all the virtual methods.

The class gbdd::Bdd wraps around a BDD identifier and the space it belongs
to for convenience. Its constructor takes a BDD space and a boolean
value to create a leaf and contains several static methods to create
different kinds of BDDs given other BDDs. Two instances of Bdd can
also be combined by the usual C++ operators such as & and |, with the
obvious meanings. There are also more specific methods, like Bdd::value_follow,
which are there for performance. They can be used to implement certain operations
more efficiently than just using normal BDD products.

Using the class Bdd is the normal way of using BDDs in this package, the 
methods in Space are considered low-level.

\section creating Creating BDDs

BDDs can be created directly through constructors and static methods in gbdd::Bdd. There
is also a helping class, gbdd::Bdd::Vars, which takes a gbdd::Space and represents all BDD variables.
This class works as a BDD factory, creating BDDs with a nicer syntax. 

\section domains Domains

A domain, represented by gbdd::Domain, is a set of variables (integers) and represents the set of
variables used to represent a component of a relation. A domain can be
infinite, in case it is periodic in the sense that it can be
represented as a base value and a step value. For example, two common
infinite sets of variables is the set of even variables and the set of
odd variables. 

A vector of domains is represented by the class gbdd::Domains. There
are several useful operations on vectors of domains, for example *
which represents concatenation meaning the cartesian product of the
domains.

\section relations Relations

A gbdd::BddRelation is a BDD with a vector of domains, with length equal to its
arity. Since domains can be infinite, so can relations. Although the
most common case is when the domains are finite, there are situations
when infinite domains are useful. Think of relations with infinite
domains as untyped BDDs, it has no information on what variables are
used to represent what. Sometimes you just want to see a relation
as a BDD, and then infinite domains are useful.

The relation concept is generalized in gbdd::StructureRelation, which operates
on any structure inheriting from gbdd::StructureConstraint, of which gbdd:Bdd is a special
case.

\section reading Further reading

Begin by studying the gbdd::Domain class, then moving on the gbdd::Bdd
and gbdd::BddRelation. These contain code examples that illustrates the
use of the package. See also gbdd::Bdd::Vars for a simplified way to create BDDs.

*/


#endif /* DOXYGEN_MAINPAGE_H */
