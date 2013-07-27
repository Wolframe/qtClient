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

#include "DataFormatXML.hpp"
#include "testXmlSerialization.hpp"

#include <QtTest/QtTest>

void testXmlSerialization::testDataTree_data( )
{
	QTest::addColumn< QString >( "docType" );
	QTest::addColumn< QString >( "rootElement" );
	QTest::addColumn< QString >( "content" );
	QTest::addColumn< QString >( "result" );
	
	QTest::newRow( "simple form" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1>value1</elem1></form>"
		<< "OpenTag 'elem1', Value 'value1', CloseTag ''";
}

void testXmlSerialization::testDataTree( )
{
	QFETCH( QString, docType );
	QFETCH( QString, rootElement );
	QFETCH( QString, content );
	QFETCH( QString, result );

	QCOMPARE( serToString( getXMLSerialization( docType, rootElement, content.toUtf8( ) ) ), result );
}

QString testXmlSerialization::serToString( QList< DataSerializeItem > ser )
{
	QString s;
	bool first = true;
	
	foreach( DataSerializeItem item, ser ) {
		if( first ) {
			first = false;
		} else {
			s.append( ", " );
		}
		s.append( item.toString( ) );
	}
	return s;
}

QTEST_MAIN( testXmlSerialization )
