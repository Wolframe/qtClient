#include "DataStructDescriptionMap.hpp"
#include "DataPathTree.hpp"
#include <QDebug>

struct StackElement
{
	const DataStructDescription* descr;
	DataStructDescription::const_iterator itr;

	bool next()
	{
		DataStructDescription::const_iterator xx = itr+1;
		if (xx == descr->end()) return false;
		++itr;
		return true;
	}

	StackElement( const DataStructDescription* descr_)
		:descr(descr_),itr(descr->begin()){}
	StackElement( const StackElement& o)
		:descr(o.descr),itr(o.itr){}
};

static void getDataStructDescriptionMap_( DataStructDescriptionMap& res, QList<QString>& lca, const DataStructDescription* descr)
{
	QList<StackElement> stk;
	QList<int> scopeReferences;
	DataPathTree tree;

	if (descr->size() == 0) return;
	stk.push_back( descr);

	while (!stk.isEmpty())
	{
		DataStructDescription::const_iterator di = stk.back().itr;
		if (di->type == DataStructDescription::variableref_)
		{
			scopeReferences.push_back( tree.addPathNode( di->variablename()));
		}
		if (di->array())
		{
			if (di->substruct)
			{
				DataStructDescriptionMap subres;
				QList<QString> sublca;
				getDataStructDescriptionMap_( subres, sublca, di->substruct);
				if (sublca.isEmpty())
				{
					qCritical() << "array elements do not have a common ancestor path";
				}
				scopeReferences.push_back( tree.addPathNode( sublca));
				DataStructDescriptionMap::const_iterator bi = subres.begin(), be = subres.end();
				for (; bi != be; ++bi)
				{
					if (sublca.size() > bi.value().size())
					{
						qCritical() << "logic error: common prefix of list bigger than a member";
						return;
					}
					QList<QString> relative_pt;
					QList<QString>::const_iterator ci = bi.value().begin() + sublca.size();
					for (; ci != bi.value().end(); ++ci)
					{
						relative_pt.push_back( *ci);
					}
					res.insert( bi.key(), relative_pt);
				}
			}
		}
		else if (di->type == DataStructDescription::struct_)
		{
			if (di->substruct->size())
			{
				stk.push_back( di->substruct);
			}
		}

		while (!stk.back().next())
		{
			stk.pop_back();
		}
	}
	int ancestor = tree.getLowestCommonAncestor( scopeReferences);
	if (ancestor > 0)
	{
		lca = tree.getPath( ancestor);
	}
	else if (ancestor < 0)
	{
		qCritical() << "illegal state in get data structure description map (no common ancestor in tree)";
	}
}

DataStructDescriptionMap getDataStructDescriptionMap( const DataStructDescription& descr)
{
	DataStructDescriptionMap rt;
	QList<QString> sublca;
	getDataStructDescriptionMap_( rt, sublca, &descr);
	return rt;
}



