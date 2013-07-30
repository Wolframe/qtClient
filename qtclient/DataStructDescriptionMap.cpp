#include "DataStructDescriptionMap.hpp"

DataStructDescriptionMap::DataStructDescriptionMap()
	:m_curidx(0)
{
	m_ar.push_back( Node());
}

void DataStructDescriptionMap::printNode( QString& out, int root, int level)
{
	const Node& nd = m_ar.at( root);
	int ii = 0, nn = nd.childar.size();
	QString indent;
	indent.fill( "\t", level);
	out.append( indent);
	out.append( nd.name);
	out.append( "\n");
	for (; ii<nn; ++ii)
	{
		printNode( out, nd.childar.at(ii), level+1);
	}
}

void DataStructDescriptionMap::start()
{
	m_curidx = 0;
}

void DataStructDescriptionMap::enter( const QString& name)
{
	QList<int>::const_iterator ni = m_ar.at(m_curidx).childar.begin(), ne = m_ar.at(m_curidx).childar.end();
	for (; ni != ne; ++ni)
	{
		if (name == m_ar.at(*ni).name)
		{
			m_curidx = *ni;
			return;
		}
	}
	int newchld = m_ar.size();
	m_ar.push_back( Node( name, m_curidx));
	m_ar.at(m_curidx).childar.push_back( newchld);
	m_curidx = newchld;
}

void DataStructDescriptionMap::leave()
{
	if (m_curidx == 0) return;
	m_curidx = m_ar.at(m_curidx).parent;
}





