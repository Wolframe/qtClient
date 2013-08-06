#include "serialize/DataStructMap.hpp"
#include "serialize/DataStructDescriptionMap.hpp"
#include <QDebug>
#include <QStringList>

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

static bool enterDataPath( const DataPath& pred, const DataPath& pt, int ptsize, VisitorInterface* vi, bool writemode)
{
	int idx = 0;
	DataPath::const_iterator pi = pt.begin(), pe = pt.end();
	for (; pi != pe && idx < ptsize; ++pi,idx++)
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

static void leaveDataPath( int ptsize, VisitorInterface* vi, bool writemode)
{
	for (int idx=0; idx<ptsize; ++idx)
	{
		vi->leave( writemode);
	}
}

static QString relativeVariableRef( const DataPath& pred, const QString& variableRef)
{
	QStringList variablePath = variableRef.split( '.');
	if (variablePath.size() < pred.size())
	{
		qCritical() << "internal: wrong common ancestor calculation in variable reference (size)";
	}
	QStringList::const_iterator pi = pred.begin(), pe = pred.end();
	QStringList::const_iterator vi = variablePath.begin(), ve = variablePath.end();
	for (; pi!=pe; ++pi,++vi)
	{
		if (*pi != *vi)
		{
			qCritical() << "internal: wrong common ancestor calculation in variable reference (node values)";
		}
	}
	QString rt;
	if (vi != ve)
	{
		rt.append( *vi);
		++vi;
	}
	for (; vi != ve; ++vi)
	{
		rt.append( ".");
		rt.append( *vi);
	}
	return rt;
}

// forward declarations:
static bool assignDataStruct( const DataPath& pred, DataStruct* data, const DataStructDescriptionMap& descrmap, VisitorInterface* vi, bool writemode);
static bool assignDataGroupElement( const DataPath& pred, const DataPath& group, DataStruct* elem, const DataStructDescription::Element* elemdescr, const DataStructDescriptionMap& descrmap, VisitorInterface* vi, bool writemode);
static bool assignDataStruct( DataStruct* data, VisitorInterface* vi, bool writemode);
static bool assignElement( DataStruct* data, VisitorInterface* vi, bool writemode);
static bool assignElementVariable( const DataPath& pred, const QString& group, DataStruct* elem, const DataStructDescription::Element* elemdescr, VisitorInterface* vi, bool writemode);


static bool assignElementVariable( const DataPath& pred, const QString& elemname, DataStruct* elem, const DataStructDescription::Element* elemdescr, VisitorInterface* vi, bool writemode)
{
	if (elemdescr->type == DataStructDescription::variableref_)
	{
		// ... special handling of an array of atomic values
		QString relvar = relativeVariableRef( pred, elemdescr->variableref);
		QString var = elemname;
		if (var.size() && relvar.size()) var.append( ".");
		var.append( relvar);

		if (writemode)
		{
			if (!vi->setProperty( var, elem->value()))
			{
				qCritical() << "failed to set property" << var;
				return false;
			}
		}
		else
		{
			if (!elem->setValue( vi->property( var)))
			{
				qCritical() << "failed to get property" << var;
				return false;
			}
		}
	}
	return true;
}

static bool assignDataGroupElement( const DataPath& pred, const DataPath& group, DataStruct* elem, const DataStructDescription::Element* elemdescr, const DataStructDescriptionMap& descrmap, VisitorInterface* vi, bool writemode)
{
	DataPath totpath( pred);
	totpath.append( group);

	if (elemdescr->array())
	{
		int ptsize = group.size()-1;
		// ... in case of an array, the grouping repeated
		// element of the array is considered to be the
		// last node of the common ancestor path.
		// with the predecessor elements we select the array
		// and with the last element the array elements.
		// This is an implicit assuption that makes sense.
		// For other cases we would need a construct in the
		// structure description to explicitely define the
		// loop grouping element of the array.
		if (ptsize < 0)
		{
			qCritical() << "array elements not grouped (no common ancestor path)";
			return false;
		}

		if (!enterDataPath( pred, group, ptsize, vi, writemode))
		{
			return false;
		}
		QString arrayname = group.at( ptsize);
		int ai = 0, ae = elem->size();
		for (; ai != ae; ++ai)
		{
			bool vi_enter = vi->enter( arrayname, writemode);
			if (!vi_enter)
			{
				if (arrayname.size())
				{
					// ... special handling of an array of atomic values
					if (!assignElementVariable( totpath, arrayname, elem, elemdescr, vi, writemode))
					{
						qCritical() << "failed to assign array element data element" << arrayname << "[" << ai << "] at" << dataPathString( pred, group, ptsize);
						return false;
					}
					continue;
				}
				else
				{
					qCritical() << "failed to enter array data element" << arrayname << "[" << ai << "] at" << dataPathString( pred, group, ptsize);
				}
				return false;
			}
			if (elemdescr->substruct)
			{
				if (!assignDataStruct( totpath, elem->at( ai), descrmap, vi, writemode))
				{
					qCritical() << "failed to assign" << arrayname << "[" << ai << "] at" << dataPathString( totpath, DataPath(), totpath.size()-1);
				}
			}
			else if (elemdescr->type == DataStructDescription::variableref_)
			{
				QString var = relativeVariableRef( totpath, elemdescr->variableref);
				if (!vi->setProperty( var, elem->at( ai)->value()))
				{
					qCritical() << "failed to set property" << var << "in" << arrayname << "[" << ai << "] at" << dataPathString( pred, group, ptsize);
					return false;
				}
			}
			if (vi_enter)
			{
				vi->leave( writemode);
			}
		}
		leaveDataPath( ptsize, vi, true);
	}
	else
	{
	}
	return true;
}


static bool assignDataStruct( const DataPath& pred, DataStruct* data, const DataStructDescriptionMap& descrmap, VisitorInterface* vi, bool writemode)
{
	const DataStructDescription* descr = data->description();

	DataStruct::iterator si = data->begin(), se = data->end();
	DataStructDescription::const_iterator di = descr->begin();
	for (; si != se; ++si,++di)
	{
		if (di->substruct)
		{
			DataStructDescriptionMap::const_iterator mi = descrmap.find( di->substruct);
			if (mi != descrmap.end())
			{
				if (!assignDataGroupElement( pred, mi.value(), &*si, &*di, descrmap, vi, writemode))
				{
					return false;
				}
			}
		}
		else
		{
		}
	}
	return false;
}

static bool assignDataStruct( DataStruct* data, VisitorInterface* vi, bool writemode)
{
	bool rt;
	const DataStructDescription* descr = data->description();
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
	DataStructDescriptionMap::const_iterator mi = descrmap.find( descr);
	if (mi != descrmap.end())
	{
		if (!enterDataPath( DataPath(), mi.value(), mi.value().size(), vi, writemode))
		{
			qCritical() << "failed get to data object" << mi.value();
			return false;
		}
		rt = assignDataStruct( mi.value(), data, descrmap, vi, writemode);
		leaveDataPath( mi.value().size(), vi, writemode);
	}
	else
	{
		rt = assignDataStruct( mi.value(), data, descrmap, vi, writemode);
	}
	return rt;
}

bool putDataStruct( const DataStruct& data, VisitorInterface* vi)
{
	return assignDataStruct( const_cast<DataStruct*>( &data), vi, true);
}

bool getDataStruct( DataStruct& data, VisitorInterface* vi)
{
	return assignDataStruct( &data, vi, true);
}



