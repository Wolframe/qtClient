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
#ifndef _WIDGET_DATA_STRUCT_HPP_INCLUDED
#define _WIDGET_DATA_STRUCT_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QSharedPointer>

///\brief Forward declaration for DataStruct
class DataStructDescription;

///\class DataStruct
///\brief Data structure to represent the data unit processed by Wolframe
class DataStruct
{
public:
	DataStruct();
	DataStruct( const DataStructDescription* description_);
	DataStruct( const QVariant& value);
	DataStruct( const DataStruct& o);

	DataStruct& operator = ( const DataStruct& o);
	~DataStruct();

	const QVariant& value() const;
	bool atomic() const;
	bool array() const;

	int compare( const DataStruct& o);

	const DataStruct* at( int idx) const;
	DataStruct* at( int idx);

	const DataStruct* get( const QString& name) const;
	DataStruct* get( const QString& name);

private:
	friend class DataStructDescription;
	void setDescription( const DataStructDescription* description_);
	void assign( const DataStruct& o);
	void release();

	int m_size;
	union
	{
		QVariant* elem;
		DataStruct* ref;
	}
	m_data;
	const DataStructDescription* m_description;
};

#endif

