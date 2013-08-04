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
#ifndef _WIDGET_DATA_PATH_TREE_HPP_INCLUDED
#define _WIDGET_DATA_PATH_TREE_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QList>
#include <QStringList>

///\class DataPathTree
///\brief Data structure representing a tree with QString as path node names
///\remark Tree has an artificial root node with no name (node with index 0)
class DataPathTree
{
public:
	///\brief Constructor
	DataPathTree();
	///\brief Copy constructor
	DataPathTree( const DataPathTree& o)
		:m_curidx(o.m_curidx),m_ar(o.m_ar){}

	///\brief Prints the tree with TABs on new newline as indentiation to a string
	void print( QString& out);

	///\brief Set the current node index as 'nodeidx' (0 for the root node)
	void start( int nodeidx=0);

	///\brief Set the current node index to the child with name 'name' of the current node. Create the node, if it does not exist yet.
	///\return the new index of the current node
	int enter( const QString& name);
	///\brief Set the current node index to the child with name 'name' of the current node. Return false if it does not exist.
	///\return the new index of the current node or -1 if the function failed
	int visit( const QString& name);
	///\brief Set the current node index to the parent of the current node
	///\return the new index of the current node or -1 if there is no ancestor for this node (root)
	int leave();

	///\brief Return the current node index
	int currentNodeIdx() const				{return m_curidx;}

	///\brief Get the node index of the path 'path' relative to the current node index. Path element delimiter is '.'
	///\return the index of the node created.
	int getPathNode( const QString& path) const		{return getPathNode( path.split( '.'));}
	///\brief Add a node with the path 'path' relative to the current node index. Path element delimiter is '.'
	///\return the index of the node created.
	int addPathNode( const QString& path)			{return addPathNode( path.split( '.'));}

	///\brief Get the node index of the path 'path' relative to the current node index
	int getPathNode( const QStringList& path) const;
	///\brief Add a node with the path 'path' relative to the current node index
	int addPathNode( const QStringList& path);

	///\brief Get the absolute path string of the current node (empty string for root). Path element delimiter is '.'
	QString getPathString() const				{return getPathString(m_curidx);}
	///\brief Get the absolute path string of the node with index 'nodeidx'. Path element delimiter is '.'
	QString getPathString( int nodeidx) const		{return getPath( nodeidx).join(".");}

	///\brief Get the absolute path of the current node
	QStringList getPath() const				{return getPath(m_curidx);}
	///\brief Get the absolute path of the node with index 'nodeidx'
	QStringList getPath( int nodeidx) const;

	///\brief Returns the lowest common ancestor node of the argument node index list in the tree
	int getLowestCommonAncestor( const QList<int>& nodear) const;

private:
	///\brief Prints the node with index 'root' on the indentiation (TAB) level 'level'
	void printNode( QString& out, int root, int level);

	struct Node
	{
		QString name;
		int parent;
		QList<int> childar;

		Node()
			:parent(-1){}
		Node( const Node& o)
			:name(o.name),parent(o.parent),childar(o.childar){}
		Node( const QString& name_, int parent_)
			:name(name_),parent(parent_){}
	};

	int m_curidx;
	QList<Node> m_ar;
};

#endif
