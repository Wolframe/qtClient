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
					// the attribute into the elemend
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



