/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "httpconnection.h"
#include <QTimer>
#include <QtAlgorithms>
#include <kurl.h>
#include <klocale.h>
#include <net/socketmonitor.h>
#include <util/log.h>

#include "btversion.h"

namespace bt
{

	HttpConnection::HttpConnection() : sock(0),state(IDLE),mutex(QMutex::Recursive),using_proxy(false)
	{
		status = i18n("Not connected");
		connect(&reply_timer,SIGNAL(timeout()),this,SLOT(replyTimeout()));
		connect(&connect_timer,SIGNAL(timeout()),this,SLOT(connectTimeout()));
		up_gid = down_gid = 0;
	}


	HttpConnection::~HttpConnection()
	{
		if (sock)
		{
			net::SocketMonitor::instance().remove(sock);
			delete sock;
		}
		
		qDeleteAll(requests);
	}
	
	void HttpConnection::setGroupIDs(Uint32 up,Uint32 down)
	{
		up_gid = up;
		down_gid = down;
		if (sock)
		{
			sock->setGroupID(up_gid,true);
			sock->setGroupID(down_gid,false);
		}
	}
	
	const QString HttpConnection::getStatusString() const
	{
		QMutexLocker locker(&mutex);
		return status;
	}
	
	bool HttpConnection::ok() const 
	{
		QMutexLocker locker(&mutex);
		return state != ERROR;
	}
		
	bool HttpConnection::connected() const 
	{
		QMutexLocker locker(&mutex);
		return state == ACTIVE;
	}
	
	bool HttpConnection::closed() const
	{
		QMutexLocker locker(&mutex);
		return state == CLOSED || (sock && !sock->ok());
	}
	
	void HttpConnection::connectToProxy(const QString & proxy,Uint16 proxy_port)
	{
		using_proxy = true;
		KNetwork::KResolver::resolveAsync(this, SLOT(hostResolved(KNetwork::KResolverResults)), 
										  proxy, QString::number(proxy_port == 0 ? 8080 : proxy_port));
		state = RESOLVING;
		status = i18n("Resolving proxy %1:%2",proxy,proxy_port);
	}
	
	void HttpConnection::connectTo(const KUrl & url)
	{
		using_proxy = false;
		KNetwork::KResolver::resolveAsync(this, SLOT(hostResolved(KNetwork::KResolverResults)), 
										url.host(), QString::number(url.port() <= 0 ? 80 : url.port()));
		state = RESOLVING;
		status = i18n("Resolving hostname %1",url.host());
	}

	void HttpConnection::onDataReady(Uint8* buf,Uint32 size)
	{
		QMutexLocker locker(&mutex);
		
		if (state != ERROR && requests.count() > 0)
		{
			if (size == 0)
			{
				 // connection closed
				state = CLOSED;
				status = i18n("Connection closed");
			}
			else
			{
				HttpGet* g = requests.front();
				if (!g->onDataReady(buf,size))
				{
					state = ERROR;
					status = i18n("Error: request failed: %1",g->failure_reason);
				}
				else if (g->response_header_received)
					reply_timer.stop();
			}
		}
	}
	
	Uint32 HttpConnection::onReadyToWrite(Uint8* data,Uint32 max_to_write)
	{
		QMutexLocker locker(&mutex);
		if (state == CONNECTING)
		{
			if (sock->connectSuccesFull())
			{
				//Out(SYS_CON|LOG_DEBUG) << "HttpConnection: connected "  << endl;
				state = ACTIVE;
				status = i18n("Connected");
			}
			else
			{
				Out(SYS_CON|LOG_IMPORTANT) << "HttpConnection: failed to connect to webseed "  << endl;
				state = ERROR;
				status = i18n("Error: Failed to connect to webseed");
			}
			connect_timer.stop();
		}
		else if (state == ACTIVE)
		{
			HttpGet* g = requests.front();
			if (g->request_sent)
				return 0;
			
			Uint32 len = g->buffer.size() - g->bytes_sent;
			if (len > max_to_write)
				len = max_to_write;
			
			memcpy(data,g->buffer.data() + g->bytes_sent,len);
			g->bytes_sent += len;
			if (len == g->buffer.size())
			{
				g->buffer.clear();
				g->request_sent = true;
				// wait 60 seconds for a reply
				reply_timer.start(60 * 1000);
			}
			return len;
		}
		
		return 0;
	}
	
	bool HttpConnection::hasBytesToWrite() const
	{
		QMutexLocker locker(&mutex);
		if (state == CONNECTING)
			return true;
		
		if (state == ERROR || requests.count() == 0)
			return false;
		
		HttpGet* g = requests.front();
		return !g->request_sent;
	}

	void HttpConnection::hostResolved(KNetwork::KResolverResults res)
	{
		if (res.count() > 0)
		{
			KNetwork::KInetSocketAddress addr = res.front().address();
			sock = new net::BufferedSocket(true,addr.ipVersion());
			sock->setNonBlocking();
			sock->setReader(this);
			sock->setWriter(this);
			sock->setGroupID(up_gid,true);
			sock->setGroupID(down_gid,false);
			
			if (sock->connectTo(addr))
			{
				status = i18n("Connected");
				state = ACTIVE;
				net::SocketMonitor::instance().add(sock);
				net::SocketMonitor::instance().signalPacketReady();
			}
			else if (sock->state() == net::Socket::CONNECTING)
			{
				status = i18n("Connecting");
				state = CONNECTING;
				net::SocketMonitor::instance().add(sock);
				net::SocketMonitor::instance().signalPacketReady();
				// 60 second connect timeout
				connect_timer.start(60000);
			}
			else 
			{
				Out(SYS_CON|LOG_IMPORTANT) << "HttpConnection: failed to connect to webseed" << endl;
				state = ERROR;
				status = i18n("Failed to connect to webseed");
			}
		}
		else
		{
			Out(SYS_CON|LOG_IMPORTANT) << "HttpConnection: failed to resolve hostname of webseed" << endl;
			state = ERROR;
			status = i18n("Failed to resolve hostname of webseed");
		}
	}
	
	bool HttpConnection::get(const QString & host,const QString & path,bt::Uint64 start,bt::Uint64 len)
	{
		QMutexLocker locker(&mutex);
		if (state == ERROR)
			return false;
			
		HttpGet* g = new HttpGet(host,path,start,len,using_proxy);
		requests.append(g);
		net::SocketMonitor::instance().signalPacketReady();
		return true;
	}
	
	bool HttpConnection::getData(QByteArray & data)
	{
		QMutexLocker locker(&mutex);
		if (requests.count() == 0)
			return false;
		
		HttpGet* g = requests.front();
		if (g->piece_data.size() == 0)
		{
			if (!g->request_sent)
				net::SocketMonitor::instance().signalPacketReady();
			return false;
		}
		
		data = g->piece_data;
		g->piece_data.clear();
		
		// if all the data has been received and passed on to something else
		// remove the current request from the queue
		if (g->piece_data.size() == 0 && g->finished())
		{
			delete g;
			requests.pop_front();
			if (requests.size() > 0)
				net::SocketMonitor::instance().signalPacketReady();
		}
		
		return true;
	}
	
	float HttpConnection::getDownloadRate() const
	{
		QMutexLocker locker(&mutex);
		if (sock)
			return sock->getDownloadRate();
		else
			return 0;
	}
	
	void HttpConnection::connectTimeout()
	{
		QMutexLocker locker(&mutex);
		if (state == CONNECTING)
		{
			status = i18n("Error: failed to connect, server not responding");
			state = ERROR;
		}
		connect_timer.stop();
	}
	
	void HttpConnection::replyTimeout()
	{
		QMutexLocker locker(&mutex);
		status = i18n("Error: request timed out");
		state = ERROR;
		reply_timer.stop();
	}
	
	////////////////////////////////////////////
	
	HttpConnection::HttpGet::HttpGet(const QString & host,const QString & path,bt::Uint64 start,bt::Uint64 len,bool using_proxy) : path(path),start(start),len(len),data_received(0),bytes_sent(0),response_header_received(false),request_sent(false)
	{
		QHttpRequestHeader request("GET",!using_proxy ? path : QString("http://%1/%2").arg(host).arg(path));
		request.setValue("Host",host);
		request.setValue("Range",QString("bytes=%1-%2").arg(start).arg(start + len - 1));
		request.setValue("User-Agent",bt::GetVersionString());
		request.setValue("Accept"," text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
		request.setValue("Accept-Language", "en-us,en;q=0.5");
		request.setValue("Accept-Charset","ISO-8859-1,utf-8;q=0.7,*;q=0.7");
		if (using_proxy)
		{
			request.setValue("Keep-Alive","300");
			request.setValue("Proxy-Connection","keep-alive");
		}
		else
			request.setValue("Connection","Keep-Alive");
		buffer = request.toString().toLocal8Bit();
	//	Out(SYS_CON|LOG_DEBUG) << "HttpConnection: sending http request:" << endl;
	//	Out(SYS_CON|LOG_DEBUG) << request.toString() << endl;
	}
	
	HttpConnection::HttpGet::~HttpGet()
	{}
	
	bool HttpConnection::HttpGet::onDataReady(Uint8* buf,Uint32 size)
	{
		if (!response_header_received)
		{
			// append the data
			buffer.append(QByteArray((char*)buf,size));
			// look for the end of the header 
			int idx = buffer.indexOf("\r\n\r\n");
			if (idx == -1) // haven't got the full header yet
				return true; 
			
			response_header_received = true;
			QHttpResponseHeader hdr(QString::fromLocal8Bit(buffer.mid(0,idx + 4)));
			
		//	Out(SYS_CON|LOG_DEBUG) << "HttpConnection: http reply header received" << endl;
		//	Out(SYS_CON|LOG_DEBUG) << hdr.toString() << endl;
			if (! (hdr.statusCode() == 200 || hdr.statusCode() == 206))
			{
				failure_reason = hdr.reasonPhrase();
				return false;
			}
			
			if (buffer.size() - (idx + 4) > 0)
			{
				// more data then the header has arrived so append it to piece_data
				data_received += buffer.size() - (idx + 4);
				piece_data.append(buffer.mid(idx + 4));
			}
		}
		else
		{
			// append the data to the list
			data_received += size;
			piece_data.append(QByteArray((char*)buf,size));
		}
		return true;
	}
}

#include "httpconnection.moc"