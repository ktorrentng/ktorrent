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
#include "dht.h"
#include "rpcmsg.h"
#include "rpccall.h"
#include "rpcserver.h"

namespace dht
{

	RPCCall::RPCCall(RPCServer* rpc,MsgBase* msg) : msg(msg),rpc(rpc),listener(0)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		timer.start(20*1000,true);
	}


	RPCCall::~RPCCall()
	{
		delete msg;
	}
	
	void RPCCall::onTimeout()
	{
		rpc->timedOut(msg->getMTID());
	}
	
	void RPCCall::response(MsgBase* rsp)
	{
		if (listener)
			listener->onResponse(rsp);
	}
	
	Method RPCCall::getMsgMethod() const
	{
		return msg->getMethod();
	}

}
#include "rpccall.moc"
