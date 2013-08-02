#include "DataStruct.hpp"
#include "DataStructDescription.hpp"
#include <QDebug>

DataStruct::DataStruct()
	:m_size(-1),m_description(0)
{
	m_data.elem = 0;
}

DataStruct::DataStruct( const QVariant& value)
	:m_size(-1),m_description(0)
{
	m_data.elem = new QVariant(value);
}

void DataStruct::release()
{
	if (m_size < 0)
	{
		if (m_description)
		{
			delete m_data.ref;
		}
		else if (m_data.elem)
		{
			delete m_data.elem;
		}
	}
	else
	{
		if (m_description)
		{
			delete [] m_data.ref;
		}
		else if (m_data.elem)
		{
			delete [] m_data.elem;
		}
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
		if (o.m_description)
		{
			m_data.ref = new DataStruct[ o.m_description->size()];
			DataStructDescription::const_iterator di = o.m_description->begin(), de = o.m_description->end();
			std::size_t idx = 0;
			for (; di != de; ++di,++idx)
			{
				m_data.ref[ idx] = o.m_data.ref[ idx];
			}
		}
		else
		{
			m_data.elem = new QVariant( *m_data.elem);
		}
	}
	else
	{
		if (o.m_description)
		{
			m_data.ref = new DataStruct[ o.m_size+1];
			int ii = 0;
			for (; ii <= o.m_size; ++ii)
			{
				m_data.ref[ ii] = o.m_data.ref[ ii];
			}
		}
		else
		{
			m_data.elem = new QVariant[ o.m_size+1];
			int ii = 0;
			for (; ii <= o.m_size; ++ii)
			{
				m_data.elem[ ii] = o.m_data.elem[ ii];
			}
		}
	}
	m_size = o.m_size;
	m_description = o.m_description;
}

DataStruct::DataStruct( const DataStruct& o)
	:m_size(-1),m_description(0)
{
	m_data.elem = 0;
	assign( o);
}

DataStruct::~DataStruct()
{
	release();
}

DataStruct::DataStruct( const DataStructDescription* description_)
	:m_size(-1),m_description(description_)
{
	m_size = m_description->size();
	m_data.ref = new DataStruct[ m_size];

	DataStructDescription::const_iterator di = m_description->begin(), de = m_description->end();
	for (int idx=0; di != de; ++di,++idx)
	{
		m_data.ref[ idx] = *(di->initvalue);
	}
}

const QVariant& DataStruct::value() const
{
	static const QVariant nullvalue;
	return (!m_description && m_size < 0)?(*m_data.elem):nullvalue;
}

bool DataStruct::atomic() const
{
	return (!m_description && m_size < 0);
}

bool DataStruct::array() const
{
	return (m_size >= 0);
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
			int ii=0, nn=m_size;
			for (; ii<nn; ++ii)
			{
				int cmp = m_data.ref[ ii].compare( o.m_data.ref[ ii]);
				if (cmp) return cmp;
			}
		}
		else if (!m_description && !o.m_description)
		{
			int ii=0, nn=m_size;
			for (; ii<nn; ++ii)
			{
				int cmp = compareVariant( m_data.elem[ii], o.m_data.elem[ii]);
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
	}
	else if (m_description || description_)
	{
		qCritical() << "internal: illegal state (set description of structure)";
	}
}

const DataStruct* DataStruct::at( int /*idx*/) const
{
	if (!array()) return 0;
	return 0;
}

DataStruct* DataStruct::at( int /*idx*/)
{
	if (!array()) return 0;
	return 0;
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






