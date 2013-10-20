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
#ifndef _WIDGET_ACTION_RESULT_DEFINITION_HPP_INCLUDED
#define _WIDGET_ACTION_RESULT_DEFINITION_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QSharedPointer>
#include "serialize/DataStructDescription.hpp"

class ActionResultDefinition
{
public:
	ActionResultDefinition( const QString& content);
	ActionResultDefinition( const ActionResultDefinition& o);
	QString toString() const;

	const QString& doctype() const			{return m_doctype;}
	const QString& rootelement() const		{return m_rootelement;}
	const DataStructDescription& structure() const	{return m_structure;}

private:
	QString m_doctype;
	QString m_rootelement;
	DataStructDescription m_structure;
};

#endif

