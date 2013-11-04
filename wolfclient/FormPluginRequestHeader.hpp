/************************************************************************

 Copyright (C) 2011 - 2013 Project Wolframe.
 All rights reserved.

 This file is part of Project Wolframe.

 Commercial Usage
    Licensees holding valid Project Wolframe Commercial licenses may
    use this file in accordance with the Project Wolframe
    Commercial License Agreement provided with the Software or,
    alternatively, in accordance with the terms contained
    in a written agreement between the licensee and Project Wolframe.

 GNU General Public License Usage
    Alternatively, you can redistribute this file and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wolframe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wolframe.  If not, see <http://www.gnu.org/licenses/>.

 If you have questions regarding the use of this file, please contact
 Project Wolframe.

************************************************************************/
///\brief Header for plugin requests to the server
#ifndef _FORM_PLUGIN_REQUEST_HEADER_HPP_INCLUDED
#define _FORM_PLUGIN_REQUEST_HEADER_HPP_INCLUDED
#include <QString>
#include <QVariant>

class FormPluginRequestHeader
{
public:
	FormPluginRequestHeader()
		:recipientid(0),cnt(0){}

	FormPluginRequestHeader( const QString& str)
	{
		recipientid = getRecipientId( str);
		cnt = getCnt( str);
		command = getCommand( str);
		pluginName = getPluginName( str);
	}

	FormPluginRequestHeader( const FormPluginRequestHeader& o)
		:recipientid(o.recipientid),cnt(o.cnt),command(o.command),pluginName(o.pluginName){}

	FormPluginRequestHeader( WId recipientid_, QString command_, QString pluginName_)
		:recipientid(recipientid_),cnt(0),command(command_),pluginName(pluginName_)
	{
		static int g_cnt = 0;
		cnt = ++g_cnt;
	}

	WId recipientid;
	int cnt;
	QString command;
	QString pluginName;

	QString toString() const
	{
		return QString( "t=P:w=%1:n=%2:c=%3:f=%4" ).arg(recipientid).arg(cnt).arg(command).arg(pluginName);
	}

	static bool isValidFormPluginRequestHeader( const QString& str)
	{
		return str.startsWith( "t=P:");
	}

	static QVariant getId( const QString& hdrstr, const char* id)
	{
		int idx = hdrstr.indexOf( id);
		if (idx >= 0)
		{
			idx += 2;
			int end = hdrstr.indexOf( ':', idx);
			if (end >= 0)
			{
				return hdrstr.mid( idx, end-idx);
			}
			else
			{
				return hdrstr.mid( idx);
			}
		}
		return QVariant();
	}

	static WId getRecipientId( const QString& hdrstr)
	{
		return getId( hdrstr, "w=").toInt();
	}

	static int getCnt( const QString& hdrstr)
	{
		return getId( hdrstr, "n=").toInt();
	}

	static QString getCommand( const QString& hdrstr)
	{
		return getId( hdrstr, "c=").toString();
	}

	static QString getPluginName( const QString& hdrstr)
	{
		return getId( hdrstr, "f=").toString();
	}
};

#endif
