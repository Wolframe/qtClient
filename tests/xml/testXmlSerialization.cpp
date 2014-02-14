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

#include "serialize/DataFormatXML.hpp"
#include "testXmlSerialization.hpp"

#include <QtTest/QtTest>

void testXmlSerialization::testXmlDeserialization_data( )
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
	QTest::newRow( "attributes" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1 attr1='val1' attr2='val2'>value1</elem1></form>"
		<< "OpenTag 'elem1', Attribute 'attr1', Value 'val1', Attribute 'attr2', Value 'val2', Value 'value1', CloseTag ''";
	QTest::newRow( "check root element" )
		<< "Form.simpleform"
		<< "form"
		<< "<otherform><elem1>value1</elem1></otherform>"
		<< "";
	QTest::newRow( "check unexpected end of data" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1>value1</elem1>"
		<< "";
	QTest::newRow( "entities are not supported" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1>&entity;</elem1>"
		<< "";
	QTest::newRow( "CDATA" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1><![CDATA[<test & test>]]></elem1></form>"
		<< "OpenTag 'elem1', Value '<test & test>', CloseTag ''";
	QTest::newRow( "significant white-space" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1>   test and  test   </elem1></form>"
		<< "OpenTag 'elem1', Value '   test and  test   ', CloseTag ''";	
	QTest::newRow( "insignificant white-space" )
		<< "Form.simpleform"
		<< "form"
		<< "<form>\n  <elem1 >   test and  test   </elem1  >\n</form>\n"
		<< "OpenTag 'elem1', Value '   test and  test   ', CloseTag ''";	
	QTest::newRow( "only significant white-space" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1>    </elem1></form>"
		<< "OpenTag 'elem1', Value '    ', CloseTag ''";	
	QTest::newRow( "empty content in tag" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1></elem1></form>"
		<< "OpenTag 'elem1', CloseTag ''";	
	QTest::newRow( "empty tag, is same as empty value" )
		<< "Form.simpleform"
		<< "form"
		<< "<form><elem1/></form>"
		<< "OpenTag 'elem1', CloseTag ''";	
	QTest::newRow( "deep tree from configurator" )
		<< "Tree.simpleform"
		<< "tree"
		<< "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
"<!DOCTYPE tree SYSTEM \"TagHierarchy.simpleform\">\n"
"<tree>\n"
"	<item id=\"20\">\n"
"		<tag>Portabile</tag>\n"
"		<description>Calculatoare portabile, tablete</description>\n"
"		<item id=\"21\">\n"
"			<tag>Notebook</tag></item>\n"
"		<item id=\"22\">\n"
"			<tag>Netbook</tag></item>\n"
"		<item id=\"23\">\n"
"			<tag>Tablet</tag></item>\n"
"		<item id=\"24\">\n"
"			<tag>Ultrabook</tag></item>\n"
"	</item>\n"
"    </tree>\n"
		<< "OpenTag 'item', Attribute 'id', Value '20', OpenTag 'tag', Value 'Portabile', CloseTag '', OpenTag 'description', Value 'Calculatoare portabile, tablete', CloseTag '', OpenTag 'item', Attribute 'id', Value '21', OpenTag 'tag', Value 'Notebook', CloseTag '', CloseTag '', OpenTag 'item', Attribute 'id', Value '22', OpenTag 'tag', Value 'Netbook', CloseTag '', CloseTag '', OpenTag 'item', Attribute 'id', Value '23', OpenTag 'tag', Value 'Tablet', CloseTag '', CloseTag '', OpenTag 'item', Attribute 'id', Value '24', OpenTag 'tag', Value 'Ultrabook', CloseTag '', CloseTag '', CloseTag ''";
}

void testXmlSerialization::testXmlDeserialization( )
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
