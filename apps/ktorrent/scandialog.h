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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <datachecker/datacheckerlistener.h>
#include "scandlgbase.h"


namespace kt
{
	class TorrentInterface;
}

class ScanDialog : public ScanDlgBase, public bt::DataCheckerListener
{
	Q_OBJECT
public:
	ScanDialog(bool auto_import,QWidget* parent = 0, const char* name = 0, bool modal = true, WFlags fl = 0 );
	virtual ~ScanDialog();

	void execute(kt::TorrentInterface* tc,bool silently);

protected:
	virtual void progress(bt::Uint32 num,bt::Uint32 total);
	virtual void status(bt::Uint32 num_failed,bt::Uint32 num_downloaded);
	

protected slots:
	virtual void reject();
	virtual void accept();
	void scan();

private:
	kt::TorrentInterface* tc;
	bool silently;
	bool auto_import;
};


#endif

