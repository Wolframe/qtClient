/************************************************************************

 Copyright (C) 2011 - 2014 Project Wolframe.
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
///\brief Header for widget requests to the server
#ifndef _WIDGET_REQUEST_IDENTIFIER_HPP_INCLUDED
#define _WIDGET_REQUEST_IDENTIFIER_HPP_INCLUDED
#include <QWidget>
#include <QString>
#include <QVariant>

class WidgetRequestHeader
{
public:
	enum Type {Undefined, Load, Action, Plugin};

	WidgetRequestHeader() :type(Undefined){}
	WidgetRequestHeader( Type type_, QWidget* recipient_widgetid);
	WidgetRequestHeader( const QString& str);
	WidgetRequestHeader( const WidgetRequestHeader& o);

	struct
	{
		QVariant widgetid;
	} recipient;

	QVariant actionid;
	QVariant followform;
	QVariant command;
	Type type;

	QString toString() const;
	QString toLogIdString() const;

	static QVariant getActionId( const QString& hdrstr);
	static QVariant getFollowForm( const QString& hdrstr);
	static QVariant getWidgetId( const QString& hdrstr);
	static QVariant getCommand( const QString& hdrstr);
	static Type getType( const QString& hdrstr);
};

#endif
