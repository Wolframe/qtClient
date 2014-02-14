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
///\brief Identifier for widgets
#ifndef _WIDGET_IDENTIFIER_HPP_INCLUDED
#define _WIDGET_IDENTIFIER_HPP_INCLUDED
#include <QWidget>
#include <QString>
#include <QVariant>
#include <QDataStream>
#include <QMetaType>

#define WIDGETID_MIMETYPE "text/widgetid"

class WidgetId
{
public:
	const QString& objectName() const	{return m_objectName;}
	int cnt() const				{return m_cnt;}

	static QString objectName( const QString& id);
	static int cnt( const QString& id);
	static bool isValid( const QString& id);

	WidgetId() :m_cnt(0){}
	explicit WidgetId( const QWidget* wdg);
	explicit WidgetId( const QString& str);
	WidgetId( const QString& objectName_, int cnt_);
	WidgetId( const WidgetId& o);

	QString toString() const;

private:
	QString m_objectName;
	int m_cnt;
};

QDataStream &operator<<(QDataStream &out, const WidgetId& myObj);
QDataStream &operator>>(QDataStream &in, WidgetId& myObj);

QVariant getWidgetId( const QWidget* wdg);
QString askWidgetId( QWidget* wdg);
void setWidgetId( QWidget* wdg);
bool widgetIdMatches( const QString& id, const QWidget* wdg);

#endif

