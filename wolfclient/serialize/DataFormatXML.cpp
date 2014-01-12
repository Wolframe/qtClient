#include "serialize/DataFormatXML.hpp"
#include "debugview/DebugHelpers.hpp"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QQueue>
#include <QDebug>
#include <QStringList>

#undef WOLFRAME_LOWLEVEL_DEBUG

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
				else if (ie->value().isValid())
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

static QString getDocid( const QByteArray& content)
{
	int idx = content.indexOf( "<!DOCTYPE");
	if (idx < 0) return QString();

	int endidx = content.indexOf( '>', idx);
	if (endidx < 0)
	{
		qCritical() << "Cannot parse <!DOCTYPE ..> definition";
		return QString();
	}
	QString doctypedef( content.mid( idx, endidx-idx));

	int sysidx = doctypedef.indexOf( "SYSTEM");
	if (sysidx < 0)
	{
		qCritical() << "Cannot parse <!DOCTYPE ..> definition (SYSTEM declaration missing)";
		return QString();
	}
	int sbidx = doctypedef.indexOf( '\'', sysidx);
	char br = '\'';
	if (sbidx < 0)
	{
		br = '\"';
		sbidx = doctypedef.indexOf( '\"', sysidx);
	}
	if (sbidx < 0)
	{
		qCritical() << "Cannot parse <!DOCTYPE ..> definition (SYSTEM declaration not a string)";
		return QString();
	}
	++sbidx;
	int ebidx = doctypedef.indexOf( '.', sbidx);
	if (ebidx < 0) ebidx = doctypedef.indexOf( br, sbidx);
	if (ebidx < 0) 
	{
		qCritical() << "Cannot parse <!DOCTYPE ..> definition (SYSTEM string not terminated)";
		return QString();
	}
	int slidx = doctypedef.lastIndexOf( '/', sbidx);
	if (slidx >= 0)
	{
		sbidx = slidx+1;
	}
	return doctypedef.mid( sbidx, ebidx-sbidx);
}

// In a non-DTD/schema model we don't know anything about mixed
// content. As we have structs we can assume that only leaves can contain
// text, hence significant whitespaces. All other sequences of whitespaces
// get eliminated. If we compare to JSON then there is no such thing as
// mixed content in non-leaf elements, so the assumption should be sane.

QList<DataSerializeItem> getXMLSerialization( const QString& docType, const QString& rootElement, const QByteArray& content )
{
	QList<DataSerializeItem> rt;
	QXmlStreamReader xml( content );
	int tagLevel = 0;
	QString value;
	bool lastElementWasOpenTag = true;

	if (!docType.isEmpty())
	{
		QString xmlDocType = getDocid( content);
		if (xmlDocType != docType)
		{
			if (xmlDocType.isEmpty())
			{
				qCritical() << "Document type is not defined but expected to be equal to " << docType;
			}
			else
			{
				qCritical() << "Document type does not match: (" << xmlDocType << "!=" << docType << ")";
			}
		}
	}
	while( !xml.atEnd( ) ) {
		xml.readNext( );
		
		if( xml.isStartElement( ) ) {

			// push value:
			QString::const_iterator ti = value.begin( ), te = value.end( );
			for( ; ti != te && ti->isSpace( ); ti++ ) ;
			if( ti != te ) {
				rt.push_back( DataSerializeItem( DataSerializeItem::Value, value ) );
#ifdef WOLFRAME_LOWLEVEL_DEBUG
				qDebug() << "XML Content (start elem)" << value;
#endif
			}

			value.clear( );
						
			++tagLevel;

			QString tagname = xml.name().toString();
			if (tagLevel == 1)
			{
				if (!rootElement.isEmpty( ) && tagname != rootElement)
				{
					// check root element:
					qCritical( ) << "XML root element" << tagname << "does not match to defined:" << rootElement;
					return rt;
				}
			} else {
#ifdef WOLFRAME_LOWLEVEL_DEBUG
				qDebug() << "XML OpenTag" << tagname;
#endif
				rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, tagname ) );
			}

			// push attributes:
			getXMLAttributes( rt, xml.attributes( ) );
			
			lastElementWasOpenTag = true;
			
		}
		else if (xml.isEndElement())
		{
			// output non empty content gathered:
			if (value.size())
			{
				QString::const_iterator ti = value.begin( ), te = value.end( );
				for( ; ti != te && ti->isSpace( ); ti++ ) ;
				if (ti != te || lastElementWasOpenTag)
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, value ) );
#ifdef WOLFRAME_LOWLEVEL_DEBUG
					qDebug() << "XML Content (end elem)" << shortenDebugMessageArgument( value);
#endif
				}
				value.clear( );
			}
			// close tag
			--tagLevel;
			
#ifdef WOLFRAME_LOWLEVEL_DEBUG
			qDebug() << "XML CloseTag";
#endif

			//... root element ignored. so is the end element belonging to root element.
			if( tagLevel > 0 ) {
				rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, "" ) );
			}

			lastElementWasOpenTag = false;
			
		} else if( xml.isCDATA( ) || xml.isCharacters( ) || xml.isWhitespace( ) ) {
			// gather content
			value.append( xml.text( ) );
		}
	}
	
	if( xml.hasError( ) ) {
		// don't return errornous deserializations
		qCritical( ) 	<< "XML error occurred in line"
				<< xml.lineNumber( ) << ", column"
				<< xml.columnNumber( ) << ":"
				<< xml.errorString( );
		rt.clear( );
		return rt;
	}
	
	return rt;
}
