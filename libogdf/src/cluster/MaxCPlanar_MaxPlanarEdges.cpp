/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-05-02 23:18:35 +1000 (Fri, 02 May 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of a constraint class for the Branch&Cut algorithm
 * for the Maximum C-Planar SubGraph problem.
 * 
 * These constraints represent the planarity-constraints belonging to the
 * ILP formulation. These constraints are dynamically separated.
 * For the separation the planarity test algorithm by Boyer and Myrvold is used.
 * 
 * \author Mathias Jansen
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/

#ifdef USE_ABACUS

#include <ogdf/internal/cluster/MaxCPlanar_MaxPlanarEdges.h>

using namespace ogdf;


MaxPlanarEdgesConstraint::MaxPlanarEdgesConstraint(ABA_MASTER *master, int edgeBound, List<nodePair> &edges) :
	ABA_CONSTRAINT(master, 0, ABA_CSENSE::Less, edgeBound, false, false, true)
{
	m_graphCons = false;
	ListConstIterator<nodePair> it;
	for (it=edges.begin(); it.valid(); ++it) {
		m_edges.pushBack(*it);
	}
}

MaxPlanarEdgesConstraint::MaxPlanarEdgesConstraint(ABA_MASTER *master, int edgeBound) :
	ABA_CONSTRAINT(master, 0, ABA_CSENSE::Less, edgeBound, false, false, true)
{
	m_graphCons = true;
}


MaxPlanarEdgesConstraint::~MaxPlanarEdgesConstraint() {}


double MaxPlanarEdgesConstraint::coeff(ABA_VARIABLE *v) {
	
	if (m_graphCons) return 1.0;
	
	EdgeVar *e = (EdgeVar*)v;
	ListConstIterator<nodePair> it;
	for (it=m_edges.begin(); it.valid(); ++it) {
		if ( ((*it).v1 == e->sourceNode() && (*it).v2 == e->targetNode()) || 
		     ((*it).v1 == e->targetNode() && (*it).v2 == e->sourceNode()) )
		{return 1.0;}
	}
	return 0.0;
}

#endif // USE_ABACUS
