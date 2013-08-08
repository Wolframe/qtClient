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

#include "serialize/DataStructMap.hpp"
#include "serialize/DataPathTree.hpp"
#include "DataPathTreeVisitor.hpp"
#include "serialize/DataStructDescription.hpp"
#include "testDataStructMap.hpp"

#include <QtTest/QtTest>

class ObjTree
{
public:
	ObjTree(){}
	ObjTree( const ObjTree& o)
		:m_object(o.m_object){}

	struct Node
	{
		Node( const QString& path_, const QVariant& value_)
			:m_path(path_),m_value(value_){}
		QString m_path;
		QVariant m_value;
	};

	ObjTree& operator << (const Node& node)
	{
		m_object.assign( node.m_path, node.m_value);
		return *this;
	}

private:
	DataPathTreeVisitor m_object;
};

Q_DECLARE_METATYPE( ObjTree);

void DataStructMapTest::testDataStructMap_data( )
{
	QTest::addColumn< ObjTree >( "objtree");
	QTest::addColumn< QString >( "description");
	QTest::addColumn< QString >( "data");
	QTest::addColumn< QString >( "errmsg");

	QTest::newRow( "simple assignment test" )
		<< (ObjTree() << ObjTree::Node( "person.name", QVariant( "hugo")))
		<< "person {name={person.name}}"
		<< "person {name='hugo'}"
		<< "OK";
}

void DataStructMapTest::testDataStructMap( )
{
	QFETCH( ObjTree, objtree );
	QFETCH( QString, description );
	QFETCH( QString, data );
	QFETCH( QString, errmsg );

	DataStructDescription* dd = new DataStructDescription();
	QList<QString> errlist;
	bool success = (errmsg == "OK");
	bool parse_rt = dd->parse( description, errlist);
	QCOMPARE( parse_rt, success);
	if (!success)
	{
		QString first_error = errlist.size()?errlist.at(0):QString();
		QCOMPARE( first_error, errmsg);
	}
	DataStruct* st = new DataStruct( dd);
	QString st_toString = st->toString();
	QCOMPARE( st_toString, data);
	delete st;
	delete dd;
}

QTEST_MAIN( DataStructMapTest )

