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
#ifndef BTPEERUPLOADER_H
#define BTPEERUPLOADER_H

#include <qvaluelist.h>
#include "request.h"



namespace bt
{
	class Peer;
	class ChunkManager;
	
	/**
	 * @author Joris Guisson
	 * @brief Uploads pieces to a Peer
	 * 
	 * This class handles the uploading of pieces to a Peer. It keeps
	 * track of a list of Request objects. All these Requests where sent
	 * by the Peer. It will upload the pieces to the Peer, making sure
	 * that the maximum upload rate isn't surpassed. 
	 */
	class PeerUploader
	{
		Peer* peer;
		QValueList<Request> requests;
	public:
		/**
		 * Constructor. Set the Peer.
		 * @param peer The Peer
		 */
		PeerUploader(Peer* peer);
		virtual ~PeerUploader();

		/**
		 * Add a Request to the list of Requests.
		 * @param r The Request
		 */
		void addRequest(const Request & r);
		
		/**
		 * Remove a Request from the list of Requests.
		 * @param r The Request
		 */
		void removeRequest(const Request & r);
		
		/**
		 * Update the PeerUploader. This will check if there are Request, and
		 * will try to handle them.
		 * @param cman The ChunkManager
		 * @param opt_unchoked PeerID of optimisticly unchoked peer
		 * @return The number of bytes uploaded
		 */
		Uint32 update(ChunkManager & cman,const PeerID & opt_unchoked);
	};

}

#endif
