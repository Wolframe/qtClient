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
#include "serialize/ActionDefinition.hpp"
#include "serialize/CondProperties.hpp"
#include <QDebug>

static bool isAlphaNum( QChar ch)
{
	return ch.isLetter() || ch == '_' || ch.isDigit();
}

static void skipSpaces( QString::const_iterator& itr, const QString::const_iterator& end)
{
	for (; itr != end && itr->isSpace(); ++itr);
}

ActionDefinition::ActionDefinition( const QString& str)
{
	m_condProperties = getConditionProperties( str);

	QString::const_iterator itr = str.begin(), end = str.end();
	skipSpaces( itr, end);
	for (; itr != end && isAlphaNum(*itr); ++itr);
	m_doctype = QString( str.begin(), itr-str.begin());
	skipSpaces( itr, end);
	if (itr != end && *itr == ':')
	{
		m_command = m_doctype;
		m_doctype.clear();
		++itr;
		skipSpaces( itr, end);
	}
	if (itr != end && isAlphaNum(*itr))
	{
		QString::const_iterator rootstart = itr;
		for (; itr != end && isAlphaNum(*itr); ++itr);
		m_rootelement = QString( rootstart, itr-rootstart);
		skipSpaces( itr, end);
	}
	if (itr != end && *itr == '{')
	{
		QString::const_iterator start = itr;
		QList<QString> errlist;
		if (!m_structure.parse( start, end, errlist))
		{
			qCritical() << "invalid action definition:";
			foreach (const QString& err, errlist)
			{
				qCritical() << "error: " << err;
			}
		}
	}
}

ActionDefinition::ActionDefinition( const ActionDefinition& o)
	:m_condProperties(o.m_condProperties)
	,m_command(o.m_command)
	,m_doctype(o.m_doctype)
	,m_rootelement(o.m_rootelement)
	,m_structure(o.m_structure){}

QString ActionDefinition::toString() const
{
	QString rt;
	if (!m_command.isEmpty())
	{
		rt.append( m_command);
		rt.append( ": ");
	}
	rt.append( m_doctype);
	rt.push_back( ' ');
	rt.append( m_rootelement);
	rt.push_back( ' ');
	rt.push_back( '{');
	rt.append( m_structure.toString());
	rt.push_back( '}');
	return rt;
}

