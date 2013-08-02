#include "DataStructDescription.hpp"
#include "DataStruct.hpp"
#include <QDebug>

DataStructDescription::Element::Element( const QString& name_, const DataStructDescription* substruct_, bool pointer_)
	:type(pointer_?indirection_:struct_),name(name_),substruct(const_cast<DataStructDescription*>(substruct_))
{
	if (!pointer_)
	{
		substruct = new DataStructDescription( *substruct_);
		initvalue = new DataStruct( substruct);
	}
}

DataStructDescription::Element::Element( const QString& name_, const QVariant& initvalue_)
	:type(atomic_),name(name_),initvalue(0)
{
	initvalue = new DataStruct( initvalue_);
}

DataStructDescription::Element::Element( const QString& name_, const QString& varname, const QVariant& defaultvalue)
	:type(variableref_),name(name_),variableref(varname)
{
	initvalue = new DataStruct( defaultvalue);
}

QString DataStructDescription::Element::variablename() const
{
	return (type == variableref_)?variableref:QString();
}

bool DataStructDescription::Element::makeArray()
{
	if (type == indirection_) return false;
	if (array()) return false;
	if (!initvalue->makeArray()) return false;
	flags |= (unsigned char)Array;
	return true;
}

DataStructDescription::Element::Element( const Element& o)
	:type(o.type),name(o.name),variableref(o.variableref),initvalue(0),substruct(o.substruct)
{
	if (substruct)
	{
		if (type != indirection_)
		{
			substruct = new DataStructDescription( *o.substruct);
			initvalue = new DataStruct( *o.initvalue);
			initvalue->setDescription( substruct);
		}
	}
	else
	{
		initvalue = new DataStruct( *o.initvalue);
	}
}

DataStructDescription::Element::~Element()
{
	if (initvalue) delete initvalue;
	if (substruct && type != indirection_) delete substruct;
}

int DataStructDescription::addAtomVariable( const QString& name_, const QString& variblename, const QVariant& defaultvalue)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_, variblename, defaultvalue));
	return m_ar.size()-1;
}

int DataStructDescription::addAtom( const QString& name_, const QVariant& initvalue_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_, initvalue_));
	return m_ar.size()-1;
}

int DataStructDescription::addAttributeVariable( const QString& name_, const QString& variblename, const QVariant& defaultvalue)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.insert( m_nofattributes++, Element( name_, variblename, defaultvalue));
	return m_nofattributes-1;
}

int DataStructDescription::addAttribute( const QString& name_, const QVariant& initvalue_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.insert( m_nofattributes++, Element( name_, initvalue_));
	return m_nofattributes-1;
}

int DataStructDescription::addStructure( const QString& name_, const DataStructDescription& substruct_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_, &substruct_));
	return m_ar.size()-1;
}

int DataStructDescription::addIndirection( const QString& name_, const DataStructDescription* descr)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_, descr, true));
	return m_ar.size()-1;
}

int DataStructDescription::addElement( const Element& elem)
{
	if (findidx( elem.name) >= 0)  return -1;
	m_ar.push_back( elem);
	return m_ar.size()-1;
}

void DataStructDescription::inherit( const DataStructDescription& parent)
{
	const_iterator pi = parent.begin(), pe = parent.end();
	for (; pi != pe; ++pi) addElement( *pi);
}

int DataStructDescription::findidx( const QString& name_) const
{
	int ii = 0, nn = m_ar.size();
	for (; ii<nn; ++ii) if (name_ == m_ar[ii].name) return (int)ii;
	return -1;
}

DataStructDescription::const_iterator DataStructDescription::find( const QString& name_) const
{
	int findidx_ = findidx( name_);
	if (findidx_ < 0) return DataStructDescription::end();
	return m_ar.begin() + findidx_;
}

DataStructDescription::iterator DataStructDescription::find( const QString& name_)
{
	int findidx_ = findidx( name_);
	if (findidx_ < 0) return DataStructDescription::end();
	return m_ar.begin() + findidx_;
}

template <typename TP>
static int compareArithmentic( const TP& aa, const TP& bb)
{
	if (aa < bb) return -1;
	if (aa > bb) return +1;
	return 0;
}

static int compareDouble( const double& aa, const double& bb)
{
	if (aa - bb < 0.00000001 && bb - aa > 0.00000001) return 0;
	if (aa > bb) return +1;
	return -1;
}

static int compareVariant( const QVariant& aa, const QVariant& bb)
{
	if (aa.type() != bb.type())
	{
		return (aa.type() < bb.type())?-1:+1;
	}
	switch (aa.type())
	{
		case QVariant::Int: return compareArithmentic( aa.toInt(), bb.toInt());
		case QVariant::UInt: return compareArithmentic( aa.toUInt(), bb.toUInt());
		case QVariant::Double: return compareDouble( aa.toDouble(), bb.toDouble());
		case QVariant::String: return compareArithmentic( aa.toString(), bb.toString());
		case QVariant::List:
		{
			QVariantList la = aa.toList();
			QVariantList lb = bb.toList();
			if (la.size() != lb.size()) return compareArithmentic( la.size(), lb.size());
			QVariantList::const_iterator ai = la.begin(), ae = la.end();
			QVariantList::const_iterator bi = lb.begin();
			for (; ai != ae; ++ai,++bi)
			{
				int cmp = compareVariant( *ai, *bi);
				if (cmp) return cmp;
			}
			return 0;
		}
		default: return compareArithmentic( aa.toString(), bb.toString());
	}
}

int DataStructDescription::compare( const DataStructDescription& o) const
{
	if (this == &o) return 0;
	const_iterator ai = begin(), ae = end();
	const_iterator bi = o.begin(), be = o.end();
	for (; ai != ae && bi != be; ++ai,++bi)
	{
		int cmp = ai->name.compare( bi->name);
		if (cmp) return cmp;
		if (ai->substruct && bi->substruct)
		{
			cmp = ai->substruct->compare( *bi->substruct);
			if (cmp) return cmp;
		}
		else if (ai->substruct)
		{
			return -1;
		}
		else if (bi->substruct)
		{
			return +1;
		}
		if (ai->initvalue && bi->initvalue)
		{
			cmp = ai->initvalue->compare( *bi->initvalue);
		}
		else if (ai->initvalue)
		{
			return -1;
		}
		else if (bi->initvalue)
		{
			return +1;
		}
		if (cmp) return cmp;
	}
	if (ai != ae) return -1;
	if (bi != be) return +1;
	return 0;
}

QString DataStructDescription::names( const QString& sep) const
{
	QString rt;
	const_iterator ni = begin(), ne = end();
	for (; ni != ne; ++ni)
	{
		if (rt.size()) rt.append( sep);
		rt.append( ni->name);
	}
	return rt;
}

static void printElementHeader( QString& out, const DataStructDescription::Element& elem)
{
	out.append( elem.name);
	if (elem.array()) out.append( "[]");
	out.append( " ");
}

void DataStructDescription::print( QString& out, const QString& indent, const QString& newitem, int level) const
{
	const_iterator di = begin(), de = end();
	for (; di != de; ++di)
	{
		out.append( newitem);
		for (int ll=0; ll<level; ++ll) out.append( indent);

		switch (di->type)
		{
			case atomic_:
				if (di->initvalue->value().isValid())
				{
					printElementHeader( out, *di);
					if (di->attribute())
					{
						out.append( "='");
						out.append( di->initvalue->value().toString());
						out.append( "'");
					}
					else
					{
						out.append( "{'");
						out.append( di->initvalue->value().toString());
						out.append( "'}");
					}
				}
				break;

			case variableref_:
				if (di->initvalue->value().isValid())
				{
					printElementHeader( out, *di);
					if (di->attribute())
					{
						out.append( "={");
						out.append( di->initvalue->value().toString());
						out.append( "}");
					}
					else
					{
						out.append( "{{");
						out.append( di->initvalue->value().toString());
						out.append( "}}");
					}
				}
				break;

			case struct_:
				if (di->substruct)
				{
					printElementHeader( out, *di);
					out.append( "{ ");
					di->substruct->print( out, indent, newitem, level+1);
					out.append( " }");
				}
				break;

			case indirection_:
				break;

		}
		out.append( ";");
	}
}

QString DataStructDescription::tostring() const
{
	QString out;
	out.append( "{");
	print( out, "", " ", 0);
	out.append( "}");
	return out;
}

DataStruct* DataStructDescription::createDataInstance() const
{
	return new DataStruct( this);
}




