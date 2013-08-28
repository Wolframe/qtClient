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
#ifndef _WIDGET_XML_HPP_INCLUDED
#define _WIDGET_XML_HPP_INCLUDED
#include "WidgetVisitor.hpp"
#include <QWidget>
#include <QByteArray>

struct WidgetRequest
{
	enum Type {Action,DomainLoad};
	QString cmd;
	QString tag;
	QByteArray content;

	Type type() const;
	QString recipientid() const;
	QString followform() const;

	static QString actionWidgetRequestTag( QString recipientid_);
	static QString actionWidgetRequestTag( QString recipientid_, QString followform_);
	static QString domainLoadWidgetRequestTag( QString recipientid_);

	WidgetRequest(){}
	WidgetRequest( QString tag_)
		:tag(tag_){}
	WidgetRequest( QString cmd_, QString tag_, QByteArray content_)
		:cmd(cmd_),tag(tag_),content(content_){}
	WidgetRequest( const WidgetRequest& o)
		:cmd(o.cmd),tag(o.tag),content(o.content){}
};

WidgetRequest getWidgetRequest( WidgetVisitor& visitor, bool debugmode);
WidgetRequest getWidgetRequest( WidgetVisitor& visitor, const QString& actiondef, bool debugmode, const QString& menuitem);
WidgetRequest getActionRequest( WidgetVisitor& visitor, const QVariant& actiondef, bool debugmode, const QString& menuitem=QString());
bool isActionRequest( const QString& tag);
bool setWidgetAnswer( WidgetVisitor& visitor, const QByteArray& answer);

QList<QString> getMenuActionRequestProperties( WidgetVisitor& visitor, const QString& menuitem);
QList<QString> getActionRequestProperties( WidgetVisitor& visitor);

#endif

