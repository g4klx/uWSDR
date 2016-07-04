/*
 *   Copyright (C) 2006-2008,2013 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	Version_H
#define	Version_H

#include <wx/wx.h>
#include <wx/datetime.h>

const wxString VERSION  = wxT("UWSDR 0.9.1");

const wxDateTime::wxDateTime_t REL_DATE_DAY   = 29;
const wxDateTime::Month        REL_DATE_MONTH = wxDateTime::Nov;
const unsigned int             REL_DATE_YEAR  = 2013;

enum UWSDRMODE {
	MODE_AM,
	MODE_CWLN,
	MODE_CWLW,
	MODE_CWUN,
	MODE_CWUW,
	MODE_DIGL,
	MODE_DIGU,
	MODE_FMN,
	MODE_FMW,
	MODE_LSB,
	MODE_USB
};

enum TUNINGHW {
	TUNINGHW_NONE = 0,
	TUNINGHW_POWERMATE
};

enum EXTERNALADDRS {
	EXTERNALADDRS_HOST = 0,
	EXTERNALADDRS_LOCAL,
	EXTERNALADDRS_ALL
};

const int JACK_API = -1;
const int JACK_DEV = -1;

#endif
