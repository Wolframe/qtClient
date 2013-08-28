#include "serialize/DataStructDescription.hpp"
#include "serialize/DataStruct.hpp"
#include <QDebug>

DataStructDescription::Element::Element( const QString& name_)
	:type(atomic_),name(name_),initvalue(0),substruct(0),flags(0)
{
}

void DataStructDescription::Element::initStructure( const DataStructDescription* substruct_, bool pointer_)
{
	type = pointer_?indirection_:struct_;
	if (!pointer_)
	{
		substruct = new DataStructDescription( *substruct_);
		initvalue = new DataStruct( substruct);
	}
	else
	{
		substruct = substruct_;
	}
}

void DataStructDescription::Element::initAtom( const QVariant& initvalue_)
{
	initvalue = new DataStruct( initvalue_);
}

void DataStructDescription::Element::initAtomVariable( const QString& varname, const QVariant& defaultvalue)
{
	type = variableref_;
	variableref = varname;
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
	:type(o.type),name(o.name),variableref(o.variableref),initvalue(0),substruct(o.substruct),flags(o.flags)
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
	else if (o.initvalue)
	{
		initvalue = new DataStruct( *o.initvalue);
	}
}

DataStructDescription::Element::~Element()
{
	if (initvalue) delete initvalue;
	if (substruct && type != indirection_) delete substruct;
}

namespace {
struct StkElement
{
	DataStructDescription::iterator ti;
	DataStructDescription::iterator te;
	DataStructDescription::const_iterator oi;
	DataStructDescription::const_iterator oe;

	StkElement( DataStructDescription* ths, const DataStructDescription* oth)
		:ti(ths->begin()),te(ths->end()),oi(oth->begin()),oe(oth->end()){}
	StkElement( const StkElement& o)
		:ti(o.ti),te(o.te),oi(o.oi),oe(o.oe){}

	void exchangeIndirection( const DataStructDescription* ths, const DataStructDescription* oth)
	{
		if (ti->type == DataStructDescription::indirection_ && ti->substruct == ths)
		{
			ti->substruct = oth;
		}
	}
	bool finished()
	{
		return (ti == te);
	}
	void next()
	{
		ti++;
		oi++;
	}
};
}//end anonymous namespace

void DataStructDescription::setIndirectionDescription( const DataStructDescription* o)
{
	QList<StkElement> stk;
	stk.push_back( StkElement( this, o));
	if (stk.back().finished()) stk.pop_back();

	while (!stk.isEmpty())
	{
		if (stk.back().ti->type == DataStructDescription::indirection_)
		{
			stk.back().exchangeIndirection( o, this);
			stk.back().next();
		}
		else if (stk.back().ti->substruct && stk.back().oi->substruct)
		{
			const DataStructDescription* dd = stk.back().oi->substruct;
			const DataStructDescription* aa = stk.back().ti->substruct;
			const_cast<DataStructDescription*>( aa)->setIndirectionDescription( dd);
			stk.push_back( StkElement( const_cast<DataStructDescription*>( stk.back().ti->substruct), stk.back().oi->substruct));
		}
		else
		{
			stk.back().next();
		}
		while (stk.back().finished())
		{
			stk.pop_back();
			if (stk.empty()) break;
			stk.back().next();
		}
	}
}

int DataStructDescription::addAtomVariable( const QString& name_, const QString& variblename, const QVariant& defaultvalue)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_));
	m_ar.back().initAtomVariable( variblename, defaultvalue);
	return m_ar.size()-1;
}

int DataStructDescription::addAtom( const QString& name_, const QVariant& initvalue_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_));
	m_ar.back().initAtom( initvalue_);
	return m_ar.size()-1;
}

int DataStructDescription::addAttributeVariable( const QString& name_, const QString& variblename, const QVariant& defaultvalue)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.insert( m_nofattributes, Element( name_));
	m_ar[ m_nofattributes].initAtomVariable( variblename, defaultvalue);
	m_ar[ m_nofattributes].setAttribute();
	return m_nofattributes++;
}

int DataStructDescription::addAttribute( const QString& name_, const QVariant& initvalue_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.insert( m_nofattributes, Element( name_));
	m_ar[ m_nofattributes].initAtom( initvalue_);
	m_ar[ m_nofattributes].setAttribute();
	return m_nofattributes++;
}

int DataStructDescription::addStructure( const QString& name_, const DataStructDescription& substruct_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_));
	m_ar.back().initStructure( &substruct_, false);
	return m_ar.size()-1;
}

int DataStructDescription::addIndirection( const QString& name_, const DataStructDescription* substruct_)
{
	if (findidx( name_) >= 0) return -1;
	m_ar.push_back( Element( name_));
	m_ar.back().initStructure( substruct_, true);
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
}

static void print_newitem( QString& out, const QString& indent, const QString& newitem, int level)
{
	out.append( newitem);
	while (level--) out.append( indent);
}

void DataStructDescription::print( QString& out, const QString& indent, const QString& newitem, int level) const
{
	const_iterator di = begin(), de = end();
	for (int idx=0; di != de; ++di,++idx)
	{
		if (idx)
		{
			print_newitem( out, indent, newitem, level);
			out.append( ";");
		}
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
				printElementHeader( out, *di);
				if (di->attribute())
				{
					out.append( "={");
					out.append( di->variableref);
					out.append( "}");
				}
				else
				{
					out.append( "{{");
					out.append( di->variableref);
					out.append( "}}");
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
	}
}

QString DataStructDescription::toString() const
{
	QString out;
	print( out, "", " ", 0);
	return out;
}

DataStruct* DataStructDescription::createDataInstance() const
{
	return new DataStruct( this);
}

bool DataStructDescription::check() const
{
	const_iterator di = begin(), de = end();
	for (; di != de; ++di)
	{
		if (di->initvalue)
		{
			if (!di->initvalue->check())
			{
				qCritical() << "initvalue check of" << di->name << "failed";
				return false;
			}
		}
	}
	return true;
}





