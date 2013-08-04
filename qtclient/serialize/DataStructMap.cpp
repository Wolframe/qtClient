#include "serialize/DataStructMap.hpp"
#include "serialize/DataStructDescriptionMap.hpp"
#include <QDebug>

static QString dataPathString( const DataPath& pred, const DataPath& pt, int size)
{
	QString rt;
	foreach (const QString& ee, pred)
	{
		if (rt.size()) rt.append( "/");
		rt.append( ee);
	}
	int ii = 0;
	foreach (const QString& ee, pt)
	{
		if (ii < size)
		{
			if (rt.size()) rt.append( "/");
			rt.append( ee);
		}
		++ii;
	}
	return rt;
}

static bool enterDataPath( const DataPath& pred, const DataPath& pt, VisitorInterface* vi, bool writemode)
{
	int idx = 0;
	DataPath::const_iterator pi = pt.begin(), pe = pt.end();
	for (; pi != pe; ++pi,idx++)
	{
		if (!vi->enter( *pi, writemode))
		{
			qCritical() << "failed to enter data element" << dataPathString( pred, pt, idx);
			for (; idx>0; --idx) vi->leave( writemode);
			return false;
		}
	}
	return true;
}

static void leaveDataPath( const DataPath& pred, const DataPath& pt, VisitorInterface* vi, bool writemode)
{
	DataPath::const_iterator pi = pt.begin(), pe = pt.end();
	for (; pi != pe; ++pi)
	{
		vi->leave( writemode);
	}
}

static bool putDataStruct_( const DataPath& pred, const DataStruct* data, const DataStructDescription* descr, const DataStructDescriptionMap& descrmap, VisitorInterface* vi)
{
	DataPath totpath( pred);
	return false;
}

bool putDataStruct( const DataStruct& data, VisitorInterface* vi)
{
	bool rt;
	const DataStructDescription* descr = data.description();
	if (!descr)
	{
		qCritical() << "feeding data structure without description";
		return false;
	}
	DataStructDescriptionMap descrmap;
	if (!getDataStructDescriptionMap( descrmap, *descr))
	{
		qCritical() << "failed to feed data structure";
		return false;
	}
	DataStructDescriptionMap::const_iterator di = descrmap.find( descr);
	if (di != descrmap.end())
	{
		if (!enterDataPath( DataPath(), di.value(), vi, true))
		{
			qCritical() << "failed get to data object" << di.value();
			return false;
		}
		rt = putDataStruct_( di.value(), &data, descr, descrmap, vi);
		leaveDataPath( DataPath(), di.value(), vi, true);
	}
	else
	{
		rt = putDataStruct_( di.value(), &data, descr, descrmap, vi);
	}
	return rt;
}

bool getDataStruct( DataStruct& data, VisitorInterface* vi)
{
}


