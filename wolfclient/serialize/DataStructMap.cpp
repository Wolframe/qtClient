#include "serialize/DataStructMap.hpp"
#include "serialize/DataStructDescriptionMap.hpp"
#include "DebugHelpers.hpp"
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
		qCritical() << "internal: wrong common ancestor calculation in variable reference (size):" << pred.join("/") << variableRef;
		return QString();
	}
	QStringList::const_iterator pi = pred.begin(), pe = pred.end();
	QStringList::const_iterator vi = variablePath.begin(), ve = variablePath.end();
	for (; pi!=pe; ++pi,++vi)
	{
		if (*pi != *vi)
		{
			qCritical() << "internal: wrong common ancestor calculation in variable reference (node values)";
			return QString();
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
static bool readDataStruct( const DataPath& pred, DataStruct* data, const DataStructDescriptionMap& descrmap, VisitorInterface* vi);
static bool writeDataStruct( const DataPath& pred, const DataStruct* data, const DataStructDescriptionMap& descrmap, VisitorInterface* vi);


static bool readDataGroupElement( const DataPath& pred, const DataPath& group, DataStruct* elem, const DataStructDescription::Element* elemdescr, const DataStructDescriptionMap& descrmap, VisitorInterface* vi)
{
	DataPath totpath( pred);

	enum ElemCategory {AnyStruct,Atomic,AtomicInSEStruct,ArrayInSEStruct,SEStructInArray};
	ElemCategory catg = AnyStruct;

	if (elemdescr->substruct && elemdescr->substruct->size() == 1 && !elemdescr->substruct->at(0).substruct)
	{
		//... special case with structs with one atomic element.
		//	there the grouping addresses the element in the structure
		if (elemdescr->array())
		{
			catg = SEStructInArray;
		}
		else if (elemdescr->substruct->at(0).array())
		{
			catg = ArrayInSEStruct;
		}
		else
		{
			catg = AtomicInSEStruct;
		}
		elemdescr = &elemdescr->substruct->at(0);
	}
	else if (elemdescr->type == DataStructDescription::variableref_)
	{
		catg = Atomic;
	}
	if (catg == AnyStruct)
	{
		totpath.append( group);
		int ptsize = group.size()-1;
		if (ptsize < 0)
		{
			qCritical() << "array elements not grouped (no common ancestor path)";
			return false;
		}
		if (!enterDataPath( pred, group, ptsize, vi, false))
		{
			return false;
		}
		QString itemname = group.at( ptsize);
		bool vi_enter = vi->enter( itemname, false);
		if (vi_enter)
		{
			if (elemdescr->array())
			{
				QWidget* arraywidget = vi->widget();
				for (int ai = 0; vi_enter; ++ai)
				{
					elem->push();
					if (elemdescr->substruct)
					{
						if (!readDataStruct( totpath, elem->back(), descrmap, vi))
						{
							qCritical() << "failed to read" << itemname << "[" << ai << "] at" << dataPathString( totpath, DataPath(), totpath.size()-1);
							return false;
						}
						elem->back()->setInitialized();
					}
					else if (elemdescr->type == DataStructDescription::variableref_)
					{
						// ... in an atomic array the ancessor path is always complete. So the property name is here the empty string always !
						elem->back()->setValue( vi->property( ""));
						elem->back()->setInitialized();
					}
					else if (elemdescr->type == DataStructDescription::atomic_)
					{
						// ... in case of atomic value array we just have to take the default and mark it as initialized 
						elem->back()->setInitialized();
					}
					else
					{
						qCritical() << "cannot handle element of this type in group" << group << "at" << pred;
						return false;
					}
					vi->leave( false);
					if (arraywidget != vi->widget()) break;

					vi_enter = vi->enter( itemname, false);
					while (!vi_enter && ptsize && arraywidget == vi->widget())
					{
						vi->leave( false);
						--ptsize;
						vi_enter = vi->enter( group.at( ptsize), false);
						itemname = group.at( ptsize) + "." + itemname;
					}
					if (!vi_enter && (ptsize == 0 || arraywidget != vi->widget())) break;
				}
			}
			else
			{
				if (elemdescr->substruct)
				{
					if (!readDataStruct( totpath, elem, descrmap, vi))
					{
						qCritical() << "failed to read structure at" << dataPathString( totpath, DataPath(), totpath.size());
						return false;
					}
					elem->setInitialized();
				}
				else if (elemdescr->type == DataStructDescription::variableref_)
				{
					// ... in an atomic array the ancessor path is always complete. So the property name is here the empty string always !
					elem->setValue( vi->property( ""));
					elem->setInitialized();
				}
				else if (elemdescr->type == DataStructDescription::atomic_)
				{
				// ... in case of atomic value we just have to take the default and mark it as initialized 
					elem->setInitialized();
				}
				else
				{
					qCritical() << "cannot handle element of this type in group" << group << "at" << pred;
					return false;
				}
				vi->leave( false);
			}
			elem->setInitialized();
		}
		leaveDataPath( ptsize, vi, false);
	}
	else
	{
		if (elemdescr->type == DataStructDescription::variableref_)
		{
			if (!elemdescr->variableref.isEmpty())
			{
				QString itemname = relativeVariableRef( totpath, elemdescr->variableref);
				QVariant ar = vi->property( itemname);
	
				switch (catg)
				{
					case AnyStruct: break;
					case SEStructInArray:
					{
						if (ar.type() == QVariant::List)
						{
							foreach (const QVariant& vv, ar.toList())
							{
								elem->push();
								elem->back()->at(0)->setValue( vv);
								elem->back()->at(0)->setInitialized();
								elem->back()->setInitialized();
							}
						}
						else
						{
							elem->push();
							elem->back()->at(0)->setValue( ar);
							elem->back()->at(0)->setInitialized();
							elem->back()->setInitialized();
						}
						break;
					}
					case ArrayInSEStruct:
					{
						if (ar.type() == QVariant::List)
						{
							foreach (const QVariant& vv, ar.toList())
							{
								elem->at(0)->push();
								elem->at(0)->back()->setValue( vv);
								elem->at(0)->back()->setInitialized();
								elem->at(0)->setInitialized();
							}
						}
						else
						{
							elem->at(0)->push();
							elem->at(0)->back()->setValue( ar);
							elem->at(0)->back()->setInitialized();
							elem->at(0)->setInitialized();
						}
						break;
					}
					case Atomic:
					{
						elem->setValue( ar);
						break;
					}
					case AtomicInSEStruct:
					{
						elem->at(0)->setValue( ar);
						elem->at(0)->setInitialized();
						break;
					}
				}
				elem->setInitialized();
			}
		}
		else if (elemdescr->type == DataStructDescription::atomic_)
		{
			if (elemdescr->array())
			{
				qCritical() << "reading arrays of constant values in group" << group << "at" << pred;
				return false;
			}
			else
			{
				elem->setInitialized();
			}
		}
		else
		{
			qCritical() << "cannot handle element of this type in group" << group << "at" << pred;
			return false;
		}
	}
	return true;
}

static bool writeDataGroupElement( const DataPath& pred, const DataPath& group, const DataStruct* elem, const DataStructDescription::Element* elemdescr, const DataStructDescriptionMap& descrmap, VisitorInterface* vi)
{
	DataPath totpath( pred);

	enum ElemCategory {AnyStruct,Atomic,AtomicInSEStruct,ArrayInSEStruct,SEStructInArray};
	ElemCategory catg = AnyStruct;

	if (elemdescr->substruct && elemdescr->substruct->size() == 1 && !elemdescr->substruct->at(0).substruct)
	{
		//... special case with structs with one atomic element.
		//	there the grouping addresses the element in the structure
		if (elemdescr->array())
		{
			catg = SEStructInArray;
		}
		else if (elemdescr->substruct->at(0).array())
		{
			catg = ArrayInSEStruct;
		}
		else
		{
			catg = AtomicInSEStruct;
		}
		elemdescr = &elemdescr->substruct->at(0);
	}
	else if (elemdescr->type == DataStructDescription::variableref_)
	{
		catg = Atomic;
	}
	if (catg == AnyStruct)
	{
		if (elemdescr->array() && elem->size() == 0)
		{
			// ... avoid writing of an array with one NULL element in case of an empty array
			return true;
		}
		totpath.append( group);

		int ptsize = group.size()-1;
		if (ptsize < 0)
		{
			qCritical() << "array elements not grouped (no common ancestor path)";
			return false;
		}
		if (!enterDataPath( pred, group, ptsize, vi, true))
		{
			return false;
		}
		QString itemname = group.at( ptsize);
		bool vi_enter = vi->enter( itemname, true);
		if (vi_enter)
		{
			if (elemdescr->array())
			{
				DataStruct::const_iterator ai = elem->arraybegin(), ae = elem->arrayend();
				while (vi_enter)
				{
					if (!vi_enter)
					{
						qCritical() << "failed to write all elements of array" << itemname << "at" << dataPathString( pred, group, ptsize);
						return false;
					}
					if (elemdescr->substruct)
					{
						if (!writeDataStruct( totpath, &*ai, descrmap, vi))
						{
							qCritical() << "failed to write" << itemname << "[" << ((ae-ai)+1) << "] at" << dataPathString( totpath, DataPath(), totpath.size()-1);
							return false;
						}
					}
					else if (elemdescr->type == DataStructDescription::variableref_
						|| elemdescr->type == DataStructDescription::atomic_)
					{
						// ... in an atomic array the ancessor path is always complete. So the property name is here the empty string always !
						if (!vi->setProperty( "", ai->value()))
						{
							qCritical() << "failed to write element of atomic array" << itemname << "at" << dataPathString( pred, group, ptsize);
							return false;
						}
					}
					else
					{
						qCritical() << "cannot handle element of this type" << itemname << "at" << dataPathString( pred, group, ptsize);
						return false;
					}
					vi->leave( true);
					if (++ai == ae) break;

					vi_enter = vi->enter( itemname, true);
					while (!vi_enter && ptsize)
					{
						vi->leave( true);
						--ptsize;
						vi_enter = vi->enter( group.at( ptsize), true);
						itemname = group.at( ptsize) + "." + itemname;
					}
				}
			}
			else
			{
				if (elemdescr->substruct)
				{
					if (!writeDataStruct( totpath, elem, descrmap, vi))
					{
						qCritical() << "failed to write" << itemname << "at" << dataPathString( totpath, DataPath(), totpath.size()-1);
						return false;
					}
				}
				else if (elemdescr->type == DataStructDescription::variableref_
					 || elemdescr->type == DataStructDescription::atomic_)
				{
					// ... in an atomic array the ancessor path is always complete. So the property name is here the empty string always !
					if (!vi->setProperty("", elem->value()))
					{
						qCritical() << "failed to set property '' at" << dataPathString( totpath, DataPath(), totpath.size());
						return false;
					}
				}
				else
				{
					qCritical() << "cannot handle element of this type at" << dataPathString( pred, group, ptsize);
					return false;
				}
			}
		}
		leaveDataPath( ptsize, vi, true);
	}
	else
	{
		if (elemdescr->type == DataStructDescription::variableref_)
		{
			if (!elemdescr->variableref.isEmpty())
			{
				QString itemname = relativeVariableRef( totpath, elemdescr->variableref);
	
				QVariant vv;
				switch (catg)
				{
					case AnyStruct: break;
					case SEStructInArray:
					{
						QList<QVariant> ar;
						DataStruct::const_iterator ai = elem->arraybegin(), ae = elem->arrayend();
						for (; ai != ae; ++ai)
						{
							ar.push_back( ai->at(0)->value());
						}
						vv = ar;
						break;
					}
					case ArrayInSEStruct:
					{
						QList<QVariant> ar;
						DataStruct::const_iterator ai = elem->at(0)->arraybegin(), ae = elem->at(0)->arrayend();
						for (; ai != ae; ++ai)
						{
							ar.push_back( ai->value());
						}
						vv = ar;
						break;
					}
					case Atomic:
					{
						vv = elem->value();
						break;
					}
					case AtomicInSEStruct:
					{
						vv = elem->at(0)->value();
						break;
					}
				}
				if  (!vi->setProperty( itemname, vv))
				{
					qCritical() << "failed to set property for element" << itemname << "at" << totpath;
					return false;
				}
			}
		}
		else
		{
			qCritical() << "cannot handle element of this type in write group" << group << "at" << pred;
			return false;
		}
	}
	return true;
}

static bool readDataStruct( const DataPath& pred, DataStruct* data, const DataStructDescriptionMap& descrmap, VisitorInterface* vi)
{
	bool initialized = false;
	const DataStructDescription* descr = data->description();
	if (data->array())
	{
		qCritical() << "cannot handle as root node of a structure";
		return false;
	}
	DataStruct::iterator si = data->structbegin(), se = data->structend();
	DataStructDescription::const_iterator di = descr->begin(), de = descr->end();
	for (; si != se && di != de; ++si,++di)
	{
		if (di->substruct)
		{
			DataStructDescriptionMap::const_iterator mi = descrmap.find( di->substruct);
			if (mi != descrmap.end())
			{
				if (mi.value().size())
				{
					if (!readDataGroupElement( pred, mi.value(), &*si, &*di, descrmap, vi))
					{
						return false;
					}
				}
				else
				{
					qCritical() << "skipping case of grouping without common ancestor path at" << pred;
				}
			}
			else
			{
				if (!readDataStruct( pred, &*si, descrmap, vi))
				{
					qCritical() << "failed to read substructure" << di->name << "from" << pred;
					return false;
				}
			}
			if (si->initialized())
			{
				initialized = true;
			}
		}
		else if (di->type == DataStructDescription::variableref_)
		{
			if (!di->variableref.isEmpty())
			{
				QString relvar = relativeVariableRef( pred, di->variableref);
				if (!si->setValue( vi->property( relvar)))
				{
					qCritical() << "failed to read element" << di->name << "to" << relvar << "in" << pred;
					return false;
				}
				si->setInitialized();
				initialized = true;
			}
		}
		else if (di->type == DataStructDescription::atomic_)
		{
			si->setInitialized();
		}
		else
		{
			qCritical() << "cannot handle data structure elements of this type" << "in" << pred;
		}
	}
	if (di != de || si != se)
	{
		qCritical() << "internal: structures do not match" << pred;
		return false;
	}
	if (initialized)
	{
		data->setInitialized();
	}
	return true;
}

static bool writeDataStruct( const DataPath& pred, const DataStruct* data, const DataStructDescriptionMap& descrmap, VisitorInterface* vi)
{
	const DataStructDescription* descr = data->description();
	if (data->array())
	{
		qCritical() << "cannot handle as root node of a structure";
		return false;
	}
	DataStruct::const_iterator si = data->structbegin(), se = data->structend();
	DataStructDescription::const_iterator di = descr->begin(), de = descr->end();
	for (; si != se && di != de; ++si,++di)
	{
		if (di->substruct)
		{
			DataStructDescriptionMap::const_iterator mi = descrmap.find( di->substruct);
			if (mi != descrmap.end())
			{
				if (mi.value().size())
				{
					if (!writeDataGroupElement( pred, mi.value(), &*si, &*di, descrmap, vi))
					{
						return false;
					}
				}
				else
				{
					qCritical() << "skipping case of grouping without common ancestor path at" << pred;
				}
			}
			else
			{
				if (!writeDataStruct( pred, &*si, descrmap, vi))
				{
					qCritical() << "failed to write substructure" << di->name << "to" << pred;
					return false;
				}
			}
		}
		else if (di->type == DataStructDescription::variableref_)
		{
			if (!di->variableref.isEmpty())
			{
				QString relvar = relativeVariableRef( pred, di->variableref);
				if (!vi->setProperty( relvar, si->value()))
				{
					qCritical() << "failed to write element" << di->name << "to" << relvar << "in" << pred;
					return false;
				}
			}
		}
		else
		{
			qCritical() << "cannot handle data structure elements of this type" << "in" << pred;
		}
	}
	if (di != de || si != se)
	{
		qCritical() << "internal: structures do not match" << pred;
		return false;
	}
	return true;
}

static bool readDataStruct( DataStruct* data, VisitorInterface* vi)
{
	bool rt;
	const DataStructDescription* descr = data->description();
	if (!descr)
	{
		qCritical() << "feeding data structure without description";
		return false;
	}
	DataStructDescriptionMap descrmap;
	if (!getDataStructDescriptionMap( descrmap, *descr) && descrmap.size() > 0)
	{
		qCritical() << "failed to feed data structure";
		return false;
	}
	DataStructDescriptionMap::const_iterator mi = descrmap.find( descr);
	if (mi != descrmap.end())
	{
		if (!enterDataPath( DataPath(), mi.value(), mi.value().size(), vi, false))
		{
			qCritical() << "failed get to data object" << mi.value();
			return false;
		}
		rt = readDataStruct( mi.value(), data, descrmap, vi);
		leaveDataPath( mi.value().size(), vi, false);
	}
	else
	{
		rt = readDataStruct( DataPath(), data, descrmap, vi);
	}
	return rt;
}

static bool writeDataStruct( const DataStruct* data, VisitorInterface* vi)
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
		if (!enterDataPath( DataPath(), mi.value(), mi.value().size(), vi, true))
		{
			qCritical() << "failed get to data object" << mi.value();
			return false;
		}
		rt = writeDataStruct( mi.value(), data, descrmap, vi);
		leaveDataPath( mi.value().size(), vi, true);
	}
	else
	{
		rt = writeDataStruct( DataPath(), data, descrmap, vi);
	}
	return rt;
}

bool putDataStruct( const DataStruct& data, VisitorInterface* vi)
{
	qDebug() << "write data struct" << data.toString( 60);
	return writeDataStruct( &data, vi);
}

bool getDataStruct( DataStruct& data, VisitorInterface* vi)
{
	bool rt = readDataStruct( &data, vi);
	qDebug() << "read data struct" << data.toString( 60);
	return rt;
}



