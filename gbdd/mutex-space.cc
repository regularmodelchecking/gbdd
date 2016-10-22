/*
 * mutex-space.cc: 
 *
 * Copyright (C) 2002 Marcus Nilsson (marcusn@docs.uu.se)
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

#include <gbdd/mutex-space.h>

namespace gbdd
{

MutexSpace::MutexSpace(auto_ptr<Space> space):
	space(space)
{
	pthread_mutex_init(&space_mutex, NULL);
	pthread_mutex_init(&locks_mutex, NULL);
}

MutexSpace::~MutexSpace()
{
	pthread_mutex_destroy(&space_mutex);
	pthread_mutex_destroy(&locks_mutex);
}

void MutexSpace::lock_gc()
{
	space->lock_gc();
	lock();
}

void MutexSpace::unlock_gc()
{
	unlock();
	space->unlock_gc();
}

void MutexSpace::lock()
{
	pthread_mutex_lock(&locks_mutex);
	
	if (locks > 0 && locking_thread == pthread_self())
	{
		// Note that this must mean that we have space_mutex locked as well
		locks++;
	}
	else
	{
		// Locks are set in order space, locks => no deadlock
		
		pthread_mutex_unlock(&locks_mutex);
		
		pthread_mutex_lock(&space_mutex);
		pthread_mutex_lock(&locks_mutex);
		
		locks++;
		locking_thread = pthread_self();
		
	}
	
	pthread_mutex_unlock(&locks_mutex);
	
}

void MutexSpace::unlock()
{
	pthread_mutex_lock(&locks_mutex);
	
	locks--;
	if (locks == 0)
	{
		pthread_mutex_unlock(&space_mutex);
	}
	
	pthread_mutex_unlock(&locks_mutex);
}


void MutexSpace::gc() { lock(); space->gc() ; unlock(); }
 
void MutexSpace::bdd_ref(Bdd p) { lock(); space->bdd_ref(p); unlock(); }
void MutexSpace::bdd_unref(Bdd p) { lock(); space->bdd_unref(p) ; unlock(); }
 
bool MutexSpace::bdd_is_leaf(Bdd p)  { lock(); bool res = space->bdd_is_leaf(p) ; unlock(); return res; }
 
bool MutexSpace::bdd_leaf_value(Bdd p)  { lock(); bool res = space->bdd_leaf_value(p) ; unlock(); return res; }
 
gbdd::Space::Bdd MutexSpace::bdd_then(Bdd p)  { lock(); Bdd res = space->bdd_then(p) ; unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_else(Bdd p)  { lock(); Bdd res = space->bdd_else(p) ; unlock(); return res; }
gbdd::Space::Var MutexSpace::bdd_var(Bdd p)  { lock(); Var res = space->bdd_var(p) ; unlock(); return res; }
 
gbdd::Space::Bdd MutexSpace::bdd_leaf(bool v)  { lock(); Bdd res = space->bdd_leaf(v) ; unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_var_true(Var v)  { lock(); Bdd res = space->bdd_var_true(v) ; unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_var_false(Var v)  { lock(); Bdd res = space->bdd_var_false(v) ; unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_var_then_else(Var v, Bdd p_then, Bdd p_else)  { lock(); Bdd res = space->bdd_var_then_else(v, p_then, p_else) ; unlock(); return res; }
	
gbdd::Space::Bdd MutexSpace::bdd_highest_var(Bdd p)  { lock(); Bdd res = space->bdd_highest_var(p) ; unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_project(Bdd p, VarPredicate& fn_var, ProductFunction& fn_prod)  
{ lock(); Bdd res = space->bdd_project(p, fn_var, fn_prod); unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_rename(Bdd p, const VarMap& fn)  { lock(); Bdd res = space->bdd_rename(p, fn) ; unlock(); return res; }
gbdd::Space::Bdd MutexSpace::bdd_product(Bdd p, Bdd q, ProductFunction& fn)  { lock(); Bdd res = space->bdd_product(p, q, fn) ; unlock(); return res;}
gbdd::Space::Bdd MutexSpace::bdd_product(Bdd p, UnaryProductFunction& fn)  { lock(); Bdd res = space->bdd_product(p, fn) ; unlock(); return res; }
	
void MutexSpace::bdd_print(ostream &os, Bdd p)  { lock(); space->bdd_print(os, p) ; unlock(); }

unsigned int MutexSpace::get_n_nodes(void) const { return space->get_n_nodes(); }

}

