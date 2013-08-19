#include "DataPathTreeVisitor.hpp"
#include <QDebug>

#define WOLFRAME_LOWLEVEL_DEBUG

bool DataPathTreeVisitor::enter( const QString& name, bool /*writemode*/)
{
	bool rt = m_tree.visit( name) >= 0;
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "CALL enter(" << name << ") RETURNS" << rt;
#endif
	return rt;
}

void DataPathTreeVisitor::leave( bool /*writemode*/)
{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "CALL leave()";
#endif
	m_tree.leave();
}

QVariant DataPathTreeVisitor::property( const QString& name)
{
	QVariant rt;
	if (name.indexOf('.') >= 0)
	{
		int idx = 0;
		QStringList pt( name.split('.'));
		for (; idx+1 < pt.size(); ++idx)
		{
			if (m_tree.visit( pt.at(idx)) < 0)
			{
				while (idx-->0) m_tree.leave();
				return false;
			}
		}
		rt = property( pt.at( pt.size()-1));
		while (idx-->0) m_tree.leave();
	}
	else
	{
		if (!name.isEmpty()) if (m_tree.visit( name) < 0) return rt;

		QMap<int,QVariant>::const_iterator vi = m_valuemap.find( m_tree.currentNodeIdx());
		if (vi != m_valuemap.end())
		{
			rt = vi.value();
		}
		if (!name.isEmpty()) m_tree.leave();
	}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "CALL property(" << name << ") RETURNS" << rt;
#endif
	return rt;
}

bool DataPathTreeVisitor::setProperty( const QString& name, const QVariant& value)
{
	bool rt = false;
	if (name.indexOf('.') >= 0)
	{
		int idx = 0;
		QStringList pt( name.split('.'));
		for (; idx+1 < pt.size(); ++idx)
		{
			if (m_tree.visit( pt.at(idx)) < 0)
			{
				while (idx-->0) m_tree.leave();
				return false;
			}
		}
		rt = setProperty( pt.at( pt.size()-1), value);
		while (idx-->0) m_tree.leave();
	}
	else
	{
		if (!name.isEmpty()) if (m_tree.visit( name) < 0) return rt;
		m_valuemap[ m_tree.currentNodeIdx()] = value;
		if (!name.isEmpty()) m_tree.leave();
	}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "CALL setProperty(" << name << value << ")";
#endif
	return true;
}

void DataPathTreeVisitor::clear()
{
	QList<int> chld;
	chld.push_back( m_tree.currentNodeIdx());
	for (int idx=0; idx<chld.size(); ++idx)
	{
		chld.append( m_tree.getChildren( chld.at(idx)));
		QMap<int,QVariant>::iterator vi = m_valuemap.find( chld.at(idx));
		if (vi != m_valuemap.end()) *vi = QVariant();
	}
}

void DataPathTreeVisitor::endofDataFeed()
{
}

bool DataPathTreeVisitor::assign( const QString& path, const QVariant& value)
{
	int nodeidx = m_tree.addPathNode( path);
	QMap<int,QVariant>::iterator mi = m_valuemap.find( nodeidx);
	if (mi != m_valuemap.end())
	{
		qCritical() << "duplicate definition of node value of" << path;
		return false;
	}
	m_valuemap[ nodeidx] = value;
	return true;
}



