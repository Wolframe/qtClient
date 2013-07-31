#include "DataPathTree.hpp"
#include <QDebug>

DataPathTree::DataPathTree()
	:m_curidx(0)
{
	m_ar.push_back( Node());
}

void DataPathTree::printNode( QString& out, int root, int level)
{
	const Node& nd = m_ar.at( root);
	int ii = 0, nn = nd.childar.size();
	QString indent;
	indent.fill( '\t', level);
	out.append( indent);
	out.append( nd.name);
	out.append( "\n");
	for (; ii<nn; ++ii)
	{
		printNode( out, nd.childar.at(ii), level+1);
	}
}

void DataPathTree::print( QString& out)
{
	if (m_ar.isEmpty()) return;
	printNode( out, 0, 0);
}

void DataPathTree::start( int nodeidx)
{
	if (nodeidx >= m_ar.size())
	{
		qCritical() << "internal: array bound read in data path tree";
		return;
	}
	m_curidx = nodeidx;
}

int DataPathTree::enter( const QString& name)
{
	QList<int>::const_iterator ni = m_ar.at(m_curidx).childar.begin(), ne = m_ar.at(m_curidx).childar.end();
	for (; ni != ne; ++ni)
	{
		if (name == m_ar.at(*ni).name)
		{
			return m_curidx = *ni;
		}
	}
	int newchld = m_ar.size();
	m_ar.push_back( Node( name, m_curidx));
	m_ar[m_curidx].childar.push_back( newchld);
	return m_curidx = newchld;
}

int DataPathTree::visit( const QString& name)
{
	QList<int>::const_iterator ni = m_ar.at(m_curidx).childar.begin(), ne = m_ar.at(m_curidx).childar.end();
	for (; ni != ne; ++ni)
	{
		if (name == m_ar.at(*ni).name)
		{
			return m_curidx = *ni;
		}
	}
	return -1;
}

int DataPathTree::leave()
{
	if (m_curidx == 0) return -1;
	return m_curidx = m_ar.at(m_curidx).parent;
}

int DataPathTree::getPathNode( const QStringList& path) const
{
	int ci = m_curidx;
	foreach (const QString& pi, path)
	{
		QList<int>::const_iterator ni = m_ar.at(ci).childar.begin(), ne = m_ar.at(ci).childar.end();
		for (; ni != ne; ++ni)
		{
			if (pi == m_ar.at(*ni).name)
			{
				ci = *ni;
				break;
			}
		}
		if (ni == ne) return false;
	}
	return ci;
}

int DataPathTree::addPathNode( const QStringList& path)
{
	int rt = m_curidx;
	foreach (const QString& pi, path)
	{
		rt = enter( pi);
	}
	return rt;
}

QStringList DataPathTree::getPath( int nodeidx) const
{
	QList<QString> rt;
	while (nodeidx != -1)
	{
		rt.insert( 0, m_ar.at(nodeidx).name);
	}
	return rt;
}

int DataPathTree::getLowestCommonAncestor( const QList<int>& nodear) const
{
	if (nodear.size() == 0) return -1;
	if (nodear.size() == 1) return nodear.at(0);

	QList< QList<int> > pathar;
	foreach (int nd, nodear)
	{
		QList<int> path;
		int ni = nd;
		while (ni != -1)
		{
			path.insert( 0, ni);
			ni = m_ar.at(ni).parent;
		}
		if (path.size() == 0) return -1;
		pathar.push_back( path);
	}
	int pi = 0, pe = pathar.at(0).size();
	for (; pi < pe; ++pi)
	{
		int ii, nn = pathar.size();
		for (ii=1; ii<nn; ++ii)
		{
			if (pathar.at(ii).size() >= pi) break;
			if (pathar.at(ii).at(pi) != pathar.at(0).at(pi)) break;
		}
		if (ii != nn) break;
	}
	return (pi == 0)?-1:pathar.at(0).at(pi-1);
}




