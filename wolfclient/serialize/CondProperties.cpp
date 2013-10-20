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
#include "serialize/CondProperties.hpp"
#include <QDebug>

static void skipSpaces( QString::const_iterator& itr, const QString::const_iterator& end)
{
	for (; itr != end && itr->isSpace(); ++itr);
}

QList<QString> getConditionProperties( const QString& str)
{
	bool isValue = false;
	QList<QString> rt;

	QString::const_iterator itr = str.begin(), end = str.end();
	QString::const_iterator start = end;
	for (; itr != end; ++itr)
	{
		if (*itr == ';')
		{
			++itr;
			skipSpaces( itr, end);
			if (itr == end) break;

			if (*itr == '{')
			{
				isValue = true;
				start = itr+1;
			}
		}
		else if (*itr == '{')
		{
			++itr;
			skipSpaces( itr, end);
			if (itr == end)
			{
				qCritical() << "Syntax error in request answer specification (brackets not balanced):" << str;
				break;
			}
			if (*itr == '{')
			{
				isValue = true;
				start = itr+1;
			}
			else
			{
				start = itr;
			}
		}
		else if (*itr == '}')
		{
			if (start != end && isValue)
			{
				QString var = QString( start, itr-start).trimmed();
				if (var.indexOf(':') < 0)
				{
					rt.push_back( var);
				}
				else
				{
					// ... definition { var : defaultvalue } is always fulfilled
				}
			}
			isValue = false;
		}
		else if (*itr == '=')
		{
			start = end;
			isValue = true;
		}
	}
	return rt;
}



