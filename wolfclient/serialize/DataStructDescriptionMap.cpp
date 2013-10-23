#include "serialize/DataStructDescriptionMap.hpp"
#include "serialize/DataPathTree.hpp"
#include "debug/DebugLogTree.hpp"

struct StackElement
{
	bool start;
	const DataStructDescription* descr;
	DataStructDescription::const_iterator itr;

	bool next()
	{
		if (start)
		{
			start = false;
			return true;
		}
		DataStructDescription::const_iterator xx = itr+1;
		if (xx == descr->end()) return false;
		++itr;
		return true;
	}

	StackElement( const DataStructDescription* descr_)
		:start(true),descr(descr_),itr(descr->begin()){}
	StackElement( const StackElement& o)
		:start(o.start),descr(o.descr),itr(o.itr){}
};

static bool findIndirection( const DataStructDescription* descr, const DataStructDescription* indirection)
{
	DataStructDescription::const_iterator si = descr->begin(), se = descr->end();
	for (; si != se; ++si)
	{
		if (si->type == DataStructDescription::indirection_)
		{
			if (si->substruct == indirection) return true;
		}
		else if (si->substruct)
		{
			if (findIndirection( si->substruct, indirection)) return true;
		}
	}
	return false;
}

static bool findSelfRecursion( const DataStructDescription* descr)
{
	return findIndirection( descr, descr);
}

static bool getDataStructDescriptionMap_( DataStructDescriptionMap& res, QList<QString>& lca, const DataStructDescription* descr)
{
	QList<StackElement> stk;
	QList<int> scopeReferences;
	DataPathTree tree;

	if (descr->size() == 0) return true;
	stk.push_back( descr);

	for(;;)
	{
		while (!stk.isEmpty() && !stk.back().next())
		{
			stk.pop_back();
		}
		if (stk.isEmpty()) break;

		DataStructDescription::const_iterator di = stk.back().itr;
		if (di->type == DataStructDescription::variableref_ && !di->variableref.isEmpty())
		{
			scopeReferences.push_back( tree.addPathNode( di->variablename()));
		}
		const DataStructDescription* sync_substruct = 0;
		if (di->array())
		{
			if (di->substruct)
			{
				sync_substruct = di->substruct;
			}
		}
		else if (di->type == DataStructDescription::struct_)
		{
			if (findSelfRecursion( di->substruct))
			{
				sync_substruct = di->substruct;
			}
			if (di->substruct->size())
			{
				stk.push_back( di->substruct);
			}
		}
		if (sync_substruct)
		{
			DataStructDescriptionMap subres;
			QList<QString> sublca;
			if (getDataStructDescriptionMap_( subres, sublca, sync_substruct))
			{
				scopeReferences.push_back( tree.addPathNode( sublca));
	
				DataStructDescriptionMap::const_iterator bi = subres.begin(), be = subres.end();
				for (; bi != be; ++bi)
				{
					if (sublca.size() > bi.value().size())
					{
						qCritical() << "logic error: common prefix of list bigger than a member";
						return false;
					}
				}
				for (bi = subres.begin(); bi != be; ++bi)
				{
					QList<QString> relative_pt;
					QList<QString>::const_iterator ci = bi.value().begin() + sublca.size();
					for (; ci != bi.value().end(); ++ci)
					{
						relative_pt.push_back( *ci);
					}
					res.insert( bi.key(), relative_pt);
					//... insert relative path of substructures having a sync node
				}
				res.insert( sync_substruct, sublca);
				// ... insert absolute path of current sync node
			}
		}
	}
	if (scopeReferences.isEmpty())
	{
		return false;
	}

	int ancestor = tree.getLowestCommonAncestor( scopeReferences);
	if (ancestor > 0)
	{
		lca = tree.getPath( ancestor);
	}
	else if (ancestor < 0)
	{
		QString aa;
		int idx = 0;
		foreach (int ai, scopeReferences)
		{
			if (idx++) aa.append( ", ");
			aa.append( tree.getPathString( ai));
		}
		qCritical() << "illegal data structure description (no common ancestor in tree for {" << aa << "})";
		return false;
	}
	return true;
}

bool getDataStructDescriptionMap( DataStructDescriptionMap& descrmap, const DataStructDescription& descr)
{
	QList<QString> sublca;
	return getDataStructDescriptionMap_( descrmap, sublca, &descr);
}

