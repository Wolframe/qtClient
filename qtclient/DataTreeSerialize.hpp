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
#ifndef _WIDGET_DATATREE_SERIALIZE_HPP_INCLUDED
#define _WIDGET_DATATREE_SERIALIZE_HPP_INCLUDED
#include "DataSerializeItem.hpp"
#include "WidgetVisitor.hpp"
#include "DataTree.hpp"
#include <QList>
#include <QWidget>

QList<DataSerializeItem> getWidgetDataSerialization( const DataTree& datatree, WidgetVisitor& visitor);

struct WidgetDataAssignmentInstr
{
	enum Type {Enter, Leave, Assign};
	static const char* typeName( Type i)
	{
		const char* ar[] = {"Enter", "Leave", "Assign"};
		return ar[ (int)i];
	}
	Type type;
	int arraysize;
	QString name;
	QVariant value;

	WidgetDataAssignmentInstr()
		:type(Leave),arraysize(0){}
	WidgetDataAssignmentInstr( int arraysize_, const QString& name_)
		:type(Enter),arraysize(arraysize_),name(name_){}
	WidgetDataAssignmentInstr( const QString& name_, const QVariant& value_)
		:type(Assign),arraysize(0),name(name_),value(value_){}
	WidgetDataAssignmentInstr( const WidgetDataAssignmentInstr& o)
		:type(o.type),arraysize(o.arraysize),name(o.name),value(o.value){}

public:
	QString toString() const {
		return QString( "type: %1, arrsize: %2, name: %3, value: %4" )
			.arg( typeName( type ) )
			.arg( arraysize ).arg( name ).arg( value.toString( ) );
	}
};

QList<WidgetDataAssignmentInstr> getWidgetDataAssignments( const DataTree& schematree, const QList<DataSerializeItem>& answer);

#endif

