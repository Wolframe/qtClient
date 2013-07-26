#include "DataFormatXML.hpp"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QStringList>

QByteArray getDataXML( const QString& docType, const QString& rootElement, bool isStandalone, const QList<DataSerializeItem>& elements, bool debugmode)
{
	QByteArray rt;
	QXmlStreamWriter xml( &rt);
	if (debugmode)
	{
		xml.setAutoFormatting( true);
		xml.setAutoFormattingIndent( 2);
	}
	xml.writeStartDocument( "1.0", isStandalone);
	if (!isStandalone)
	{
		if (rootElement == docType)
		{
			xml.writeDTD( QString( "<!DOCTYPE %1>").arg( docType));
		}
		else
		{
			xml.writeDTD( QString( "<!DOCTYPE %1 SYSTEM '%2'>").arg( rootElement).arg( docType));
		}
		xml.writeStartElement( rootElement);
	}
	else
	{
		xml.writeStartElement( docType);
	}
	QList<DataSerializeItem>::const_iterator ie = elements.begin(), ee = elements.end();
	for (; ie != ee; ++ie)
	{
AGAIN:
		QVariant attribute;
		switch (ie->type())
		{
			case DataSerializeItem::OpenTag:
				xml.writeStartElement( ie->value().toString());
				break;

			case DataSerializeItem::CloseTag:
				xml.writeEndElement();
				break;

			case DataSerializeItem::Attribute:
				attribute = ie->value();
				++ie;
				if (ie == ee || ie->type() != DataSerializeItem::Value)
				{
					// not having a value for a key is not fatal, we just don't produce
					// the attribute into the element
					goto AGAIN;
				}
				else
				{
					xml.writeAttribute( attribute.toString(), ie->value().toString());
				}
				break;

			case DataSerializeItem::Value:
				xml.writeCharacters( ie->value().toString());
				break;
		}
	}
	xml.writeEndElement();
	xml.writeEndDocument();
	return rt;
}

static void getXMLAttributes( QList<DataSerializeItem>& list, const QXmlStreamAttributes& attributes)
{
	QXmlStreamAttributes::const_iterator ai = attributes.begin(), ae = attributes.end();
	for (; ai != ae; ++ai)
	{
		list.push_back( DataSerializeItem( DataSerializeItem::Attribute, ai->name().toString()));
		list.push_back( DataSerializeItem( DataSerializeItem::Value, ai->value().toString()));
	}
}

#define NEW_XML_PARSER
#ifdef NEW_XML_PARSER

QList<DataSerializeItem> getXMLSerialization( const QString& /* docType */, const QString& rootElement, const QByteArray& content )
{
	QList<DataSerializeItem> rt;
	QXmlStreamReader xml( content );
	int tagLevel = 0;
	QString value;

	while( !xml.atEnd( ) && !xml.hasError( ) ) {
		xml.readNext( );
		
		if( xml.isStartElement( ) ) {
			++tagLevel;

// check root element
			QString tagname = xml.name().toString();
			if( tagLevel == 1 ) {
				// TODO Check document type (!DOCTYPE) really?
				if( !rootElement.isEmpty( ) && tagname != rootElement ) {
					qCritical( ) << "XML root element" << tagname << "does not match to defined:" << rootElement;
					return rt;
				}
			} else {
				rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, tagname ) );
			}

// push attributes (TODO: really, the attributes of the root element have to appear here!?)
			getXMLAttributes( rt, xml.attributes( ) );

// push value
			if( !value.isEmpty( ) ) {
				rt.push_back( DataSerializeItem( DataSerializeItem::Value, value ) );
				value.clear( );
			}
			
		} else if( xml.isEndElement( ) ) {
// output content
			if( !value.isEmpty( ) ) {
				rt.push_back( DataSerializeItem( DataSerializeItem::Value, value ) );
				value.clear( );
			}
			
// close tag
			--tagLevel;
			
			if( tagLevel > 0 ) {
				rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, "" ) );
			}

		} else if( xml.isCDATA( ) || xml.isCharacters( ) || xml.isWhitespace( ) ) {
			value.append( xml.text( ) );
		} else if ( xml.isEntityReference( ) ) {
			qCritical() << "unexpected entity reference in XML: no entity references supported, seen entity" << xml.name( ).toString( );
			rt.clear( );
			return rt;
		}
	}
	
	if( xml.hasError( ) ) {
// don't return errornous deserializations
		qCritical( ) << "XML error occurred " << xml.errorString( );
		rt.clear( );
		return rt;
	} else if( !xml.atEnd( ) ) {
		qCritical( ) << "XML premature end of file";
		rt.clear( );
		return rt;
	} else if( tagLevel != 0 ) {
		qCritical( ) << "XML is not balanced";
		rt.clear( );
		return rt;
	}

	return rt;
}

#else

// Disabled old parser, not quite sure if I cover all the cases..
// the value is serialized before the attributes?
// I also don't like the value value is adding insignificant spaces over series of start
// and end tags just to trim them away. This way there is no way to keep significant spaces.
// the code tries to handle mixed content, but this can't exist as we have a structure.
// an empty list or some error handling is missing when the root element doesn't match
// general remark: closetag should have the same tagname as opentag, so checks can be made and
// debugging code looks nicer.
// Should the root element really be ignored here and the attributes be passed blank?

QList<DataSerializeItem> getXMLSerialization( const QString& /*docType*/, const QString& rootElement, const QByteArray& content)
{
	QList<DataSerializeItem> rt;
	QXmlStreamReader xml( content);
	int taglevel = 0;
	QString value;

	for (xml.readNext(); !xml.atEnd(); xml.readNext())
	{
		if (xml.isStartElement())
		{
			QString::const_iterator ti = value.begin(), te = value.end();
			for (; ti != te && ti->isSpace(); ++ti);
			if (ti != te)
			{
				if (taglevel > 0) rt.push_back( DataSerializeItem( DataSerializeItem::Value, value.trimmed()));
#ifdef WOLFRAME_LOWLEVEL_DEBUG
				qDebug() << "XML: parsed data" << value;
#endif
				value.clear();
			}
			++taglevel;
			QString tagname = xml.name().toString();
			if (taglevel == 1)
			{
				//TODO Check document type (!DOCTYPE)
				if (!rootElement.isEmpty() && tagname != rootElement)
				{
					qCritical() << "XML root element" << tagname << "does not match to defined:" << rootElement;
				}
			}
			else
			{
				rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, tagname));
			}
			getXMLAttributes( rt, xml.attributes());
		}
		else if (xml.isEndElement())
		{
			QString::const_iterator ti = value.begin(), te = value.end();
			for (; ti != te && ti->isSpace(); ++ti);
			if (ti != te)
			{
				rt.push_back( DataSerializeItem( DataSerializeItem::Value, value.trimmed()));
#ifdef WOLFRAME_LOWLEVEL_DEBUG
				qDebug() << "XML: parsed data" << value;
#endif
				value.clear();
			}
			--taglevel;
			if (taglevel == 0)
			{
				//... root element ignored. so is the end element belonging to root element.
			}
			else
			{
				rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, ""));
			}
		}
		else if (xml.isEntityReference())
		{
			qCritical() << "unexpected entity reference in XML: no entity references supported";
		}
		else if (xml.isCDATA() || xml.isCharacters() || xml.isWhitespace())
		{
			value.append( xml.text());
		}
	}
	return rt;
}

#endif


