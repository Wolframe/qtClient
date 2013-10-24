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
#ifndef _WOLFRAME_DEBUG_LOG_TREE_HPP_INCLUDED
#define _WOLFRAME_DEBUG_LOG_TREE_HPP_INCLUDED
#include <QDebug>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QList>

enum LogLevel
{
	LogDebug,
	LogWarning,
	LogCritical,
	LogFatal
};

class DebugLogTree
{
public:
	DebugLogTree();
	DebugLogTree( const DebugLogTree& o);

	void clear();
	void insert( LogLevel level, const QString& msg);
	QString pushPrefix( const QString& prefix, bool unique=false);
	void popPrefix();

	struct NodeStruct;
	typedef QSharedPointer<NodeStruct> NodeStructR;

	struct NodeStruct
	{
		int id;
		QString name;
		QList<NodeStructR> chld;

		NodeStruct( int id_, QString name_)
			:id(id_),name(name_){}
		NodeStruct( const NodeStruct& o)
			:id(o.id),name(o.name),chld(o.chld){}
		~NodeStruct(){}
	};

	NodeStructR getNodeStruct() const;
	QString getMessages( int id, LogLevel level) const;

private:
	QMap<QString, int> m_prefixmap;
	QMap<int,QString> m_prefixinv;
	int m_prefixcnt;
	QMap<QString, int> m_uniquecnt;
	QMap<int,QList<int> > m_childmap;
	int m_msgcnt;
	QMap<QString, int> m_msgmap;
	QMap<int,QString> m_msginv;
	QList<int> m_prefixstk;

	struct Message
	{
		int prefix;
		LogLevel level;
		int stridx;
	};
	QList<Message> m_msglist;
};

void setDebugLogTree( DebugLogTree* dbglogtree);
const DebugLogTree* getDebugLogTree();

QString openLogStruct( const QString& name, bool unique=false);
void closeLogStruct( int cnt=1);
void clearLogStruct();
void printLog( LogLevel level, const QString& msg);

DebugLogTree::NodeStructR getLogNodeStruct();
QString getLogMessages( int id, LogLevel level);

#endif

