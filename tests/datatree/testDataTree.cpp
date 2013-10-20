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

#include "serialize/DataStructDescription.hpp"
#include "serialize/DataStruct.hpp"
#include "testDataTree.hpp"

#include <QtTest/QtTest>

void DataTreeTest::testDataTree_data( )
{
	QTest::addColumn< QString >( "tree" );
	QTest::addColumn< QString >( "result" );
	
	QTest::newRow( "attribute assignment of integer constant" )
		<< "{ xyz =1 }"
		<< "{ xyz = '1' }";
	QTest::newRow( "attribute assignment of float constant" )
		<< "{ aa= '1.0' }"
		<< "{ aa = '1.0' }";
	QTest::newRow( "attribute assignment of constant string" )
		<< "{ doc { a = b } }"
		<< "{ doc { a = 'b' } }";
	QTest::newRow( "sequence of attribute assignment" )
		<< "{ doc { bli = '1.0'; bla='723974' } }"
		<< "{ doc { bli = '1.0'; bla = '723974' } }";
	QTest::newRow( "substructures of attribute assignments" )
		<< "{ doc { substruct{ bla='72 3974'} } }"
		<< "{ doc { substruct { bla = '72 3974' } } }";
	QTest::newRow( "substructure and sequences of attribute assignments" )
		<< "{ doc { bli = '1.0'; substruct{ bla='72 3974'} } }"
		<< "{ doc { bli = '1.0'; substruct { bla = '72 3974' } } }";
	QTest::newRow( "escaping of quotes in strings" )
		<< "{ doc { bli = '1.0'; substruct{ bla='\"72 3974\"'}; oth { x {{abc}} }} }"
		<< "{ doc { bli = '1.0'; substruct { bla = '\"72 3974\"' }; oth { x{{abc}} } } }";
	QTest::newRow( "arrays of substructures" )
		<< "{ doc { a[] {'b'} } }"
		<< "{ doc { a[] { 'b' } } }";
	QTest::newRow( "parsing of spaces and curly brackets" )
		<< "{form1 {first{{first}}}}"
		<< "{ form1 { first{{first}} } }";
	QTest::newRow( "mapping of content" )
		<< "{ doc { bli={first}; {second}}}"
		<< "{ doc { bli = {first}; {second} } }";
	QTest::newRow( "real example from old configurator" )
		<< "{picture {id={main.id}; caption{{main.caption}}; info{{main.info}}; tagwrap { tag[] {id={main.tags.item.id:?}}}; image{size={main.image.size:?}; {main.image.base64:?}}}}"
		<< "{ picture { id = {main.id}; caption{{main.caption}}; info{{main.info}}; tagwrap { tag[] { id = {main.tags.item.id:?} } }; image { size = {main.image.size:?}; {main.image.base64:?} } } }";
	QTest::newRow( "string constant assignment as content" )
		<< "{ object {id { {users.userList.selected} }; type {'user'}} }"
		<< "{ object { id{{users.userList.selected}}; type { 'user' } } }";
}

void DataTreeTest::testDataTree( )
{
	QFETCH( QString, tree );
	QFETCH( QString, result );

	QCOMPARE( transformTree( tree ), result );
}

QString DataTreeTest::transformTree( const QString &s )
{
	DataStructDescription descr;
	QList<QString> errlist;
	if (descr.parse( s, errlist))
	{
		return descr.toString();
	}
	else
	{
		return "INVALID";
	}
}

QTEST_MAIN( DataTreeTest )
