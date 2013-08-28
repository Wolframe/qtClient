#include "serialize/DataStruct.hpp"
#include "serialize/DataStructDescription.hpp"
#include <QDebug>

static int arrayAllocSize( int size)
{
	return (size | 0x7) + 1;
}

DataStruct::DataStruct()
	:m_size(-1),m_description(0),m_initialized(false)
{
	m_data.elem = 0;
}

DataStruct::DataStruct( const QVariant& value)
	:m_size(-1),m_description(0),m_initialized(false)
{
	m_data.elem = new QVariant(value);
}

void DataStruct::release()
{
	if (m_size < 0)
	{
		if (m_description)
		{
			if (m_data.ref) delete [] m_data.ref;
		}
		else
		{
			if (m_data.elem) delete m_data.elem;
		}
	}
	else
	{
		if (m_data.ref) delete [] m_data.ref;

	}
	m_size = -1;
	m_description = 0;
	m_data.elem = 0;
}

void DataStruct::assign( const DataStruct& o)
{
	release();
	if (o.m_size < 0)
	{
		// single
		if (o.m_description)
		{
			if (o.m_data.ref)
			{
				// single struct
				m_data.ref = new DataStruct[ arrayAllocSize( o.m_description->size())];
				DataStructDescription::const_iterator di = o.m_description->begin(), de = o.m_description->end();
				std::size_t idx = 0;
				for (; di != de; ++di,++idx)
				{
					m_data.ref[ idx] = o.m_data.ref[ idx];
				}
			}
			else
			{
				// single indirection
				m_data.ref = 0;
			}
		}
		else if (o.m_data.elem)
		{
			// single atomic
			m_data.elem = new QVariant( *o.m_data.elem);
		}
		else
		{
			m_data.elem = 0;
		}
	}
	else
	{
		// array
		m_data.ref = new DataStruct[ arrayAllocSize( o.m_size+1)];
		int ii = 0;
		for (; ii <= o.m_size; ++ii)
		{
			m_data.ref[ ii] = o.m_data.ref[ ii];
		}
	}
	m_size = o.m_size;
	m_description = o.m_description;
	m_initialized = o.m_initialized;
}

DataStruct::DataStruct( const DataStruct& o)
	:m_size(-1),m_description(0),m_initialized(o.m_initialized)
{
	m_data.elem = 0;
	assign( o);
}

DataStruct::~DataStruct()
{
	release();
}

DataStruct::DataStruct( const DataStructDescription* description_)
	:m_size(-1),m_description(description_),m_initialized(false)
{
	m_data.ref = new DataStruct[ arrayAllocSize( m_description->size())];

	DataStructDescription::const_iterator di = m_description->begin(), de = m_description->end();
	for (int idx=0; di != de; ++di,++idx)
	{
		m_data.ref[ idx] = *(di->initvalue);
	}
}

bool DataStruct::makeArray()
{
	if (m_size >= 0)
	{
		qCritical() << "internal: bad call of make array";
		return false;
	}
	int allocsize = arrayAllocSize( 1);
	DataStruct* ref = new DataStruct[ allocsize];
	ref[0] = *this;
	release();
	m_description = ref[0].m_description;
	m_data.ref = ref;
	m_size = 0;
	return true;
}

void DataStruct::expandIndirection()
{
	if (m_size < 0 && m_description && m_data.ref == 0)
	{
		m_data.ref = new DataStruct( m_description);
	}
}

const QVariant& DataStruct::value() const
{
	static const QVariant nullvalue;
	return (!m_description && m_size < 0)?(*m_data.elem):nullvalue;
}

bool DataStruct::setValue( const QVariant& val)
{
	if (m_size >= 0) return false;
	if (m_description)
	{
		int idx = m_description->findidx("");
		if (idx < 0) return false;
		DataStruct* elem = m_data.ref + idx;
		if (!elem->atomic()) return false;
		elem->setValue( val);
	}
	else
	{
		*m_data.elem = val;
	}
	return true;
}

bool DataStruct::atomic() const
{
	return (!m_description && m_size < 0);
}

bool DataStruct::array() const
{
	return (m_size >= 0);
}

bool DataStruct::indirection() const
{
	return (m_size < 0 && m_description && m_data.ref == 0);
}

static int compareVariant( const QVariant& aa, const QVariant& bb)
{
	int at = (int)aa.type();
	int bt = (int)bb.type();
	if (at < bt) return -1;
	if (at > bt) return +1;
	int cmp = 0;
	switch (aa.type())
	{
		case QVariant::Int: cmp = (aa.toInt() >= bb.toInt()) + (aa.toInt() != bb.toInt()) - 1;
		case QVariant::UInt: cmp = (aa.toUInt() >= bb.toUInt()) + (aa.toUInt() != bb.toUInt()) - 1;
		case QVariant::Double: cmp = (aa.toDouble() >= bb.toDouble()) + (aa.toDouble() != bb.toDouble()) - 1;
		default:
		{
			QString as = aa.toString();
			QString bs = bb.toString();
			cmp = (as >= bs) + (as != bs) - 1;
		}
	}
	return cmp;
}

int DataStruct::compare( const DataStruct& o)
{
	if (m_size > o.m_size) return +1;
	if (m_size < o.m_size) return -1;
	if (m_size < 0)
	{
		if (m_description && o.m_description)
		{
			if (m_description != o.m_description)
			{
				int cmp = m_description->compare( *o.m_description);
				if (cmp) return cmp;
			}
			int ii=0, nn=m_description->size();
			for (; ii<nn; ++ii)
			{
				int cmp = m_data.ref[ ii].compare( o.m_data.ref[ ii]);
				if (cmp) return cmp;
			}
		}
		else if (!m_description && !o.m_description)
		{
			if (m_initialized != o.m_initialized)
			{
				return (int)m_initialized - (int)o.m_initialized;
			}
			int cmp = compareVariant( *m_data.elem, *o.m_data.elem);
			if (cmp) return cmp;
		}
		else if (m_description)
		{
			return +1;
		}
		else if (o.m_description)
		{
			return -1;
		}
	}
	else
	{
		if (m_description && o.m_description)
		{
			if (m_description != o.m_description)
			{
				int cmp = m_description->compare( *o.m_description);
				if (cmp) return cmp;
			}
			int ii=1, nn=m_size;
			for (; ii<=nn; ++ii)
			{
				int cmp = m_data.ref[ ii].compare( o.m_data.ref[ ii]);
				if (cmp) return cmp;
			}
		}
		else if (!m_description && !o.m_description)
		{
			int ii=1, nn=m_size;
			for (; ii<=nn; ++ii)
			{
				int cmp = m_data.ref[ ii].compare( o.m_data.ref[ ii]);
				if (cmp) return cmp;
			}
		}
		else if (m_description)
		{
			return +1;
		}
		else if (o.m_description)
		{
			return -1;
		}
	}
	return 0;
}

DataStruct& DataStruct::operator = ( const DataStruct& o)
{
	assign( o);
	return *this;
}

void DataStruct::setDescription( const DataStructDescription* description_)
{
	if (m_description && description_)
	{
		if (m_description->size() != description_->size())
		{
			qCritical() << "internal: illegal state (different size structure description set)";
		}
		if (m_size >= 0)
		{
			for (int ii=0; ii<m_size; ++ii)
			{
				m_data.ref[ ii].setDescription( description_);
			}
		}
		else
		{
			int ii=0;
			DataStructDescription::const_iterator di=description_->begin(), de=description_->end();
			for (; di!=de; ++di,++ii)
			{
				m_data.ref[ ii].setDescription( di->substruct);
			}
		}
		m_description = description_;
	}
	else if (m_description || description_)
	{
		qCritical() << "internal: illegal state (set description of structure)";
	}
}

bool DataStruct::check() const
{
	if (m_description && m_description->size() > 1000)
	{
		qCritical() << "internal: check description failed";
		return false;
	}
	if (m_size < 0)
	{
		if (m_description)
		{
			std::size_t si = 0, se = m_description->size();
			DataStructDescription::const_iterator di = m_description->begin(), de = m_description->end();
			for (; si != se && di != de; ++si,++di)
			{
				if (m_data.ref[ si].m_description != di->substruct)
				{
					qCritical() << "internal: check description failed (substruct)";
					return false;
				}
				if (!m_data.ref[ si].check()) return false;
			}
			if (si != se || di != de)
			{
				qCritical() << "internal: check description failed (substruct size)";
				return false;
			}
		}
	}
	else
	{
		std::size_t si = 0, se = m_size+1;
		for (; si != se; ++si)
		{
			if (!m_data.ref[ si].check()) return false;
		}
	}
	return true;
}

bool DataStruct::push()
{
	if (!array())
	{
		qCritical() << "illegal operation 'push' on non array structure";
		return false;
	}
	int allocsize = arrayAllocSize( m_size+1);
	int prev_allocsize = arrayAllocSize( m_size);
	if (allocsize > prev_allocsize)
	{
		DataStruct* ref = new DataStruct[ allocsize];
		for (int ii=0; ii<=m_size; ++ii)
		{
			ref[ ii] = m_data.ref[ ii];
		}
		delete [] m_data.ref;
		m_data.ref = ref;
	}
	m_data.ref[ m_size+1] = m_data.ref[ 0];
	++m_size;
	return true;
}

static void print_newitem( QString& out, const QString& indent, const QString& newitem, int level)
{
	out.append( newitem);
	while (level--) out.append( indent);
}

void DataStruct::print( QString& out, const QString& indent, const QString& newitem, int level) const
{
	if (atomic())
	{
		out.append("'");
		out.append( value().toString());
		out.append("'");
	}
	else if (array())
	{
		int ii = 1;
		for (; ii<m_size; ++ii)
		{
			if (ii>1) out.append( ", ");
			m_data.ref[ ii].print( out, indent, newitem, level+1);
		}
	}
	else if (indirection())
	{
	}
	else if (m_description)
	{
		DataStructDescription::const_iterator di = m_description->begin(), de = m_description->end();
		DataStruct::const_iterator ei = begin();
		for (int idx=0; di != de; ++di,++ei)
		{
			if (ei->initialized())
			{
				if (idx++)
				{
					out.append( ";");
					print_newitem( out, indent, newitem, level);
				}

				out.append( di->name);
				if (di->array()) out.append( "[]");
				if (di->attribute())
				{
					out.append( "=");
					out.append( ei->toString());
				}
				else
				{
					out.append( "{");
					ei->print( out, indent, newitem, level+1);
					out.append( "}");
				}
			}
		}
	}
}

QString DataStruct::toString() const
{
	QString out;
	print( out, "", " ", 0);
	return out;
}

const DataStruct* DataStruct::back() const
{
	if (!array() || m_size <= 0) return 0;
	return m_data.ref + m_size;
}

DataStruct* DataStruct::back()
{
	if (!array() || m_size <= 0) return 0;
	return m_data.ref + m_size;
}

const DataStruct* DataStruct::at( int idx) const
{
	if (!array() || m_size < idx || idx < 0) return 0;
	return m_data.ref + idx + 1;
}

DataStruct* DataStruct::at( int idx)
{
	if (!array() || m_size < idx || idx < 0) return 0;
	return m_data.ref + idx + 1;
}

const DataStruct* DataStruct::get( const QString& name) const
{
	if (array()) return 0;
	if (!m_description) return 0;

	int idx = m_description->findidx( name);
	if (idx < 0) return 0;
	return m_data.ref + idx;
}

DataStruct* DataStruct::get( const QString& name)
{
	if (array()) return 0;
	if (!m_description) return 0;

	int idx = m_description->findidx( name);
	if (idx < 0) return 0;
	return m_data.ref + idx;
}

const DataStruct* DataStruct::prototype() const
{
	return array()?m_data.ref:0;
}

DataStruct::const_iterator DataStruct::begin() const
{
	if (m_description) return const_iterator( m_data.ref);
	if (m_size) return const_iterator( m_data.ref+1);
	return 0;
}

DataStruct::const_iterator DataStruct::end() const
{
	if (m_description) return const_iterator( m_data.ref + m_description->size());
	if (m_size) return const_iterator( m_data.ref + m_size + 1);
	return 0;
}

DataStruct::iterator DataStruct::begin()
{
	if (m_description) return iterator( m_data.ref);
	if (m_size) return iterator( m_data.ref+1);
	return 0;
}

DataStruct::iterator DataStruct::end()
{
	if (m_description) return iterator( m_data.ref + m_description->size());
	if (m_size) return iterator( m_data.ref + m_size + 1);
	return 0;
}

DataStructIndirection::DataStructIndirection( const DataStructDescription* descr)
	:DataStruct()
{
	m_description = descr;
}


