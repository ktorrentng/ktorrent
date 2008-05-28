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
#include <klocale.h>
#include <kicon.h>
#include "routermodel.h"
#include "upnprouter.h"

namespace kt 
{

	RouterModel::RouterModel(QObject* parent)
		: QAbstractTableModel(parent)
	{
	}
	
	
	RouterModel::~RouterModel()
	{
	}
	
	void RouterModel::addRouter(UPnPRouter* r)
	{
		routers.append(r);
		insertRow(routers.count() - 1);
	}
	
	int RouterModel::rowCount(const QModelIndex & parent) const
	{
		if (!parent.isValid())
			return routers.count();
		else
			return 0;
	}
	
	int RouterModel::columnCount(const QModelIndex & parent) const
	{
		if (!parent.isValid())
			return 3;
		else
			return 0;
	}
	
	QVariant RouterModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("Device");
			case 1: return i18n("Port Forwarded");
			case 2: return i18n("WAN Connection");
			default: return QVariant();
		}
	}

	UPnPRouter* RouterModel::routerForIndex(const QModelIndex & index)
	{
		if (!index.isValid())
			return 0;
		else
			return routers.at(index.row());
	}
	
	QVariant RouterModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid())
			return QVariant();
		
		const UPnPRouter* r = routers.at(index.row());
		if (role == Qt::DisplayRole)
		{
			switch (index.column())
			{
				case 0: return r->getDescription().friendlyName;
				case 1: return ports(r);
				case 2: return connections(r);
			}
		}
		else if (role == Qt::DecorationRole)
		{
			if (index.column() == 0)
				return KIcon("modem");
		}
		else if (role == Qt::ToolTipRole)
		{
			if (index.column() == 0)
			{
				const UPnPDeviceDescription & d = r->getDescription();
				return i18n(
					"Model Name: <b>%1</b><br/>"
					"Model Number: <b>%2</b><br/>"
					"Manufacturer: <b>%3</b><br/>"
					"Model Description: <b>%4</b><br/>",d.modelName,d.modelNumber,d.manufacturer,d.modelDescription);
			}
		}
		
		return QVariant();
	}
	
	bool RouterModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool RouterModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	QString RouterModel::ports(const UPnPRouter* r) const
	{
		QString ret;
		QList<UPnPRouter::Forwarding>::const_iterator j = r->beginPortMappings();
		while (j != r->endPortMappings())
		{
			const UPnPRouter::Forwarding & f = *j;
			if (!f.pending_req)
			{
				ret += QString::number(f.port.number) + " (";
				QString prot = (f.port.proto == net::UDP ? "UDP" : "TCP");
				ret +=  prot + ")";
			}
			j++;
			
			if (j != r->endPortMappings())
				ret += "\n";
		}
		return ret;
	}
	
	QString RouterModel::connections(const UPnPRouter* r) const
	{
		QString ret;
		QList<UPnPRouter::Forwarding>::const_iterator j = r->beginPortMappings();
		while (j != r->endPortMappings())
		{
			const UPnPRouter::Forwarding & f = *j;
			if (!f.pending_req)
			{
				if (f.service->servicetype.contains("WANPPPConnection"))
					ret += "PPP";
				else
					ret += "IP";
			}
			j++;
			
			if (j != r->endPortMappings())
				ret += "\n";
		}
		return ret;
	}

	void RouterModel::update()
	{
		emit dataChanged(index(0,0),index(rowCount(QModelIndex()) - 1,columnCount(QModelIndex()) - 1));
	}
}
