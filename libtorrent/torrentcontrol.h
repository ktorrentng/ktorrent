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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTTORRENTCONTROL_H
#define BTTORRENTCONTROL_H

#include <qobject.h>
#include <qcstring.h> 
#include <qtimer.h>
#include <kurl.h>
#include "globals.h"

class KProgressDialog;

namespace bt
{
	class Choker;
	class Torrent;
	class Tracker;
	class ChunkManager;
	class PeerManager;
	class Downloader;
	class Uploader;
	class Peer;
	class TorrentMonitor;
	
	/**
	 * @author Joris Guisson
	 * @brief Controls just about everything
	 * 
	 * This is the interface which any user gets to deal with.
	 * This class controls the uploading, downloading, choking,
	 * updating the tracker and chunk management.
	 */
	class TorrentControl : public QObject
	{
		Q_OBJECT
	public:
		TorrentControl();
		virtual ~TorrentControl();
		
		/**
		 * Initialize the TorrentControl. 
		 * @param torrent The filename of the torrent file
		 * @param datadir The directory to store the data
		 * @throw Error when something goes wrong
		 */
		void init(const QString & torrent,const QString & datadir);

		/// Get the suggested name of the torrent
		QString getTorrentName() const;

		/// Get the number of bytes downloaded
		Uint32 getBytesDownloaded() const;

		/// Get the number of bytes uploaded
		Uint32 getBytesUploaded() const;

		/// Get the number of bytes left to download
		Uint32 getBytesLeft() const;

		/// Get the total number of bytes
		Uint32 getTotalBytes() const;

		/// Get the download rate in bytes per sec
		Uint32 getDownloadRate() const;

		/// Get the upload rate in bytes per sec
		Uint32 getUploadRate() const;

		/// Get the number of peers we are connected to
		Uint32 getNumPeers() const;

		/// Get the number of chunks we are currently downloading
		Uint32 getNumChunksDownloading() const;

		/// Get the total number of chunks
		Uint32 getTotalChunks() const;

		/// Get the number of chunks which have been downloaded
		Uint32 getNumChunksDownloaded() const;
		
		/// See if we are running
		bool isRunning() const {return running;}

		/// See if the torrent has been started
		bool isStarted() const {return started;}

		/// See if the torrent was saved
		bool isSaved() const {return saved;}

		/// Get the data directory of this torrent
		QString getDataDir() const {return datadir;}

		/// Set the monitor
		void setMonitor(TorrentMonitor* tmo);

		/// See if we have a multi file torrent
		bool isMultiFileTorrent() const;

		/// Set the initial port to try out
		static void setInitialPort(Uint16 port);

		/**
		 * Set the interval between two tracker updates.
		 * @param interval The interval in milliseconds
		 */
		void setTrackerTimerInterval(Uint32 interval);

		/**
		 * Called by the Tracker when an error occurs.
		 */
		void trackerResponseError();

		/**
		 * The HTTPTracker updated.
		 * @param data The data sent by the Tracker
		 */
		void trackerResponse(const QByteArray & data);

		/**
		 * The UDPTracker updated
		 * @param interval The interval in seconds between timer updates
		 * @param leechers The number of leechers
		 * @param seeders The number of seeders
		 * @param ppeers A Buffer containing @a leechers + @a seeders
		 * 	pairs (IP-address,port)
		 */
		void trackerResponse(Uint32 interval,Uint32 leechers,Uint32 seeders,Uint8* ppeers);
	public slots:
		/**
		 * Update the object, this will be called every 100
		 * millisecond, if start was called.
		 */
		void update();
		
		/**
		 * Start the download of the torrent.
		 */
		void start();
		
		/**
		 * Stop the download, closes all connections.
		 */
		void stop();
		
		/**
		 * When the torrent is finished, the final file(s) can be
		 * reconstructed. In case of a multi file torrent, the file parameter
		 * will be interpreted as the directory to store the files.
		 * @param file The path of the file (or directory to store files)
		 * @param dlg User supplied progress dialog, 0 if you don't want one
		 */
		void reconstruct(const QString & file,KProgressDialog* dlg);
		
	private slots:
		void updateTracker() {updateTracker(QString::null);}
		void onNewPeer(Peer* p);
		void onPeerRemoved(Peer* p);
		void doChoking();
		
	signals:
		void finished(bt::TorrentControl* me);
		void trackerError(bt::TorrentControl* me,const QString & error);
		
	private:	
		void updateTracker(const QString & ev,bool last_succes = true);
		
	private:
		Torrent* tor;
		Tracker* tracker;
		ChunkManager* cman;
		PeerManager* pman;
		Downloader* down;
		Uploader* up;
		Choker* choke;
		QTimer tracker_update_timer,choker_update_timer,update_timer;
		QString datadir,trackerevent;
		Uint16 port;
		bool completed,running,started,saved;
		TorrentMonitor* tmon;
		int num_tracker_attempts;
		KURL last_tracker_url;
		
		static Uint16 initial_port;
	};

}

#endif
