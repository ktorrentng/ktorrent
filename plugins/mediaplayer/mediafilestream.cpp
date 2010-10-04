/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "mediafilestream.h"
#include <torrent/torrentfilestream.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
	MediaFileStream::MediaFileStream(bt::TorrentFileStream::WPtr stream, QObject* parent)
		: AbstractMediaStream(parent), stream(stream), waiting_for_data(false)
	{
		TorrentFileStream::Ptr s = stream.toStrongRef();
		if (s)
		{
			s->open(QIODevice::ReadOnly);
			setStreamSize(s->size());
			setStreamSeekable(!s->isSequential());
			connect(s.data(),SIGNAL(readyRead()),this,SLOT(dataReady()));
		}
	}
	
	MediaFileStream::~MediaFileStream()
	{
	}

	void MediaFileStream::dataReady()
	{
		if (waiting_for_data)
		{
			TorrentFileStream::Ptr s = stream.toStrongRef();
			if (s)
			{
				const QByteArray data = s->read(4096);
				if (!data.isEmpty()) 
				{
					writeData(data);
					waiting_for_data = false;
				}
			}
			else
				endOfData();
		}
	}

	void MediaFileStream::needData()
	{
		TorrentFileStream::Ptr s = stream.toStrongRef();
		if (!s || s->atEnd())
		{
			endOfData();
			return;
		}
		
		QByteArray data = s->read(4096);
		if (data.isEmpty()) 
		{
			waiting_for_data = true;
		}
		else
		{
			writeData(data);
		}
	}

	void MediaFileStream::reset()
	{
		TorrentFileStream::Ptr s = stream.toStrongRef();
		if (s)
			s->reset();
	}

	void MediaFileStream::enoughData()
	{
		waiting_for_data = false;
	}

	void MediaFileStream::seekStream(qint64 offset)
	{
		TorrentFileStream::Ptr s = stream.toStrongRef();
		if (s)
			s->seek(offset);
	}
}