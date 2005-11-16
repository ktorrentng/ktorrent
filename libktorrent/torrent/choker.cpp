/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qptrlist.h>
#include <interfaces/functions.h>
#include "choker.h"
#include "peermanager.h"
#include "oldchokealgorithm.h"

using namespace kt;

namespace bt
{
	
	PeerPtrList::PeerPtrList(PeerCompareFunc pcmp) : pcmp(pcmp)
	{}
		
	PeerPtrList::~PeerPtrList()
	{}
		
	int PeerPtrList::compareItems(QPtrCollection::Item a, QPtrCollection::Item b)
	{
		if (pcmp)
			return pcmp((Peer*)a,(Peer*)b);
		else
			return CompareVal(a,b);
	}
	
	////////////////////////////////////////////
	
	ChokeAlgorithm::ChokeAlgorithm() : opt_unchoked_peer_id(0)
	{
	}
	
	ChokeAlgorithm::~ChokeAlgorithm()
	{
	}
	
	/////////////////////////////////

	Choker::Choker(PeerManager & pman) : pman(pman)
	{
		choke = new OldChokeAlgorithm();
	}


	Choker::~Choker()
	{
		delete choke;
	}

	void Choker::update(bool have_all)
	{
		choke->doChoking(pman,have_all);
	}
	
}
