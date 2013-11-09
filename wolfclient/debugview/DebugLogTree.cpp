/************************************************************************

 Copyright (C) 2011 - 2013 Project Wolframe.
 All rights reserved.

 This file is part of Project Wolframe.

 Commercial Usage
    Licensees holding valid Project Wolframe Commercial licenses may
    use this file in accordance with the Project Wolframe
    Commercial License Agreement provided with the Software or,
    alternatively, in accordance with the terms contained
    in a written agreement between the licensee and Project Wolframe.

 GNU General Public License Usage
    Alternatively, you can redistribute this file and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wolframe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wolframe.  If not, see <http://www.gnu.org/licenses/>.

 If you have questions regarding the use of this file, please contact
 Project Wolframe.

************************************************************************/
#include "debugview/DebugLogTree.hpp"
#include <QDebug>

#define WOLFRAME_LOWLEVEL_DEBUG

DebugLogTree::DebugLogTree()
	:m_prefixcnt(0),m_msgcnt(0)
{
	m_prefixstk.push_back(0);
}

DebugLogTree::DebugLogTree( const DebugLogTree& o)
	:m_prefixmap(o.m_prefixmap)
	,m_prefixinv(o.m_prefixinv)
	,m_prefixcnt(o.m_prefixcnt)
	,m_uniquecnt(o.m_uniquecnt)
	,m_childmap(o.m_childmap)
	,m_msgcnt(o.m_msgcnt)
	,m_msgmap(o.m_msgmap)
	,m_msginv(o.m_msginv)
	,m_prefixstk(o.m_prefixstk)
	,m_msglist(o.m_msglist){}


void DebugLogTree::clear()
{
	m_prefixmap.clear();
	m_prefixinv.clear();
	m_prefixcnt = 0;
	m_uniquecnt.clear();
	m_childmap.clear();
	m_msgcnt = 0;
	m_msgmap.clear();
	m_msginv.clear();
	m_msglist.clear();
	m_prefixstk.clear();
	m_prefixstk.push_back(0);
}

void DebugLogTree::insert( LogLevel level, const QString& msg)
{
	struct Message msgrec;
	msgrec.prefix = m_prefixstk.back();
	msgrec.level = level;
	QMap<QString, int>::const_iterator mi = m_msgmap.find( msg);
	if (mi == m_msgmap.end())
	{
		m_msgmap.insert( msg, msgrec.stridx = ++m_msgcnt);
		m_msginv.insert( msgrec.stridx, msg);
	}
	else
	{
		msgrec.stridx = mi.value();
	}
	m_msglist.push_back( msgrec);
}

QString DebugLogTree::pushPrefix( const QString& prefix, bool unique)
{
	QString logid;
	QString pp;
	if (m_prefixstk.back())
	{
		pp.append( m_prefixinv[ m_prefixstk.back()]);
		pp.append(".");
	}
	logid.append( prefix);
	if (unique)
	{
		logid.append("#");
		logid.append( QString::number( ++m_uniquecnt[ pp]));
	}
	pp.append( logid);
	QMap<QString,int>::const_iterator pi = m_prefixmap.find( pp);
	if (pi == m_prefixmap.end())
	{
		++m_prefixcnt;
		m_prefixmap.insert( pp, m_prefixcnt);
		m_prefixinv.insert( m_prefixcnt, pp);
		m_childmap[ m_prefixstk.back()].push_back( m_prefixcnt);
		m_prefixstk.push_back( m_prefixcnt);
	}
	else
	{
		m_prefixstk.push_back( pi.value());
	}
	return logid;
}

void DebugLogTree::popPrefix()
{
	if (m_prefixstk.size() > 1)
	{
		m_prefixstk.pop_back();
	}
}

static DebugLogTree* g_logtree = 0;
static bool g_garbageCollect = true;

void setDebugGarbageCollect( bool enabled)
{
	g_garbageCollect = enabled;
}

void setDebugLogTree( DebugLogTree* dbglogtree)
{
	if (g_logtree && !dbglogtree)
	{
		g_logtree->clear();
	}
	g_logtree = dbglogtree;
}

const DebugLogTree* getDebugLogTree()
{
	return g_logtree;
}


QString openLogStruct( const QString& name, bool unique)
{
	if (g_logtree)
	{
		if (g_garbageCollect && g_logtree->isTopLevel() == 1)
		{
			g_logtree->clear();
		}
		return g_logtree->pushPrefix( name, unique);
	}
	return QString();
}

void closeLogStruct( int cnt)
{
	if (g_logtree)
	{
		while (cnt-- > 0) g_logtree->popPrefix();
	}
}

const char* logLevelName( LogLevel i)
{
	static const char* ar[] = {"Debug","Warning","Critical","Fatal"};
	return ar[ (int)i];
}

void printLog( LogLevel level, const QString& msg)
{
	if (g_logtree)
	{
		g_logtree->insert( level, msg);
	}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	fprintf( stderr, "%s: %s\n", logLevelName( level), qPrintable( msg ) );
#endif
}

void clearLogStruct()
{
	if (g_logtree)
	{
		g_logtree->clear();
	}
}

struct StackElem
{
	int chldidx;
	DebugLogTree::NodeStructR node;

	StackElem( const DebugLogTree::NodeStructR& node_)
		:chldidx(-1),node(node_){}
	StackElem()
		:chldidx(-1){}
	StackElem( const StackElem& o)
		:chldidx(o.chldidx),node(o.node){}
	~StackElem(){}
};

DebugLogTree::NodeStructR DebugLogTree::getNodeStruct() const
{
	QList<Message>::const_iterator mi = m_msglist.begin(), me = m_msglist.end();
	QMap<int,LogLevel> lvmap;
	for (; mi != me; ++mi)
	{
		QMap<int,LogLevel>::iterator li = lvmap.find( mi->prefix);
		if (li != lvmap.end())
		{
			if ((int)li.value() < (int)mi->level)
			{
				li.value() = mi->level;
			}
		}
		else
		{
			lvmap[ mi->prefix] = mi->level;
		}
	}
	QMap<int,LogLevel>::iterator li = lvmap.find( 0);
	LogLevel maxlevel = LogDebug;
	if (li != lvmap.end())
	{
		maxlevel = li.value();
	}
	QList<StackElem> stk;
	DebugLogTree::NodeStructR cd( new DebugLogTree::NodeStruct( 0, "", maxlevel));
	stk.push_back( StackElem( cd));

	while (!stk.isEmpty())
	{
		if (stk.back().chldidx == -1)
		{
			int parentidx = stk.back().node->id;
			foreach (int cl, m_childmap[ parentidx])
			{
				QString prefix = m_prefixinv[ parentidx];
				int ofs = prefix.size()?(prefix.size()+1):0;
				QString name = m_prefixinv[ cl].mid( ofs);
				maxlevel = LogDebug;
				QMap<int,LogLevel>::iterator li = lvmap.find( cl);
				if (li != lvmap.end())
				{
					maxlevel = li.value();
				}
				DebugLogTree::NodeStructR cd( new DebugLogTree::NodeStruct( cl, name, maxlevel));
				stk.back().node->chld.push_back( cd);
			}
			++stk.back().chldidx;
		}
		else if (stk.back().chldidx >= stk.back().node->chld.size())
		{
			NodeStructR rt = stk.back().node;
			foreach (const NodeStructR& cd, stk.back().node->chld)
			{
				if ((int)cd->maxlevel > (int)rt->maxlevel)
				{
					rt->maxlevel = cd->maxlevel;
				}
			}
			stk.pop_back();
			if (stk.size() == 0)
			{
				return rt;
			}
		}
		else
		{
			stk.push_back( stk.back().node->chld[ stk.back().chldidx++]);
		}
		
	}
	return DebugLogTree::NodeStructR();
}

static void fillIdSet( QMap<int,bool>& idset, int id, const QMap<int,QList<int> >& childmap)
{
	foreach( int chldidx, childmap[ id])
	{
		if (idset.find( chldidx) == idset.end())
		{
			idset[ chldidx] = true;
			fillIdSet( idset, chldidx, childmap);
		}
	}
}

QList<DebugLogTree::MessageStruct> DebugLogTree::getMessages( int id, LogLevel level) const
{
	QList<MessageStruct> rt;
	QMap<int,bool> idset;
	idset[ id] = true;
	fillIdSet( idset, id, m_childmap);
	QList<Message>::const_iterator mi = m_msglist.begin(), me = m_msglist.end();
	int prevmsg = -1;
	int msgindex = 1;
	for (; mi != me; ++mi,++msgindex)
	{
		if ((int)level <= (int)mi->level && idset[ mi->prefix])
		{
			if (prevmsg != mi->stridx)
			{
				rt.push_back( MessageStruct( mi->level, msgindex, m_msginv[ mi->stridx]));
			}
			prevmsg = mi->stridx;
		}
	}
	return rt;
}


DebugLogTree::NodeStructR getLogNodeStruct()
{
	if (g_logtree)
	{
		return g_logtree->getNodeStruct();
	}
	else
	{
		return DebugLogTree::NodeStructR();
	}
}

QList<DebugLogTree::MessageStruct> getLogMessages( int id, LogLevel level)
{
	if (g_logtree)
	{
		return g_logtree->getMessages( id, level);
	}
	else
	{
		return QList<DebugLogTree::MessageStruct>();
	}
}


