/************************************************************************

 Copyright (C) 2011 - 2014 Project Wolframe.
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
#ifndef _WOLFRAME_DATA_PATH_TREE_TEST_VISITOR_HPP_INCLUDED
#define _WOLFRAME_DATA_PATH_TREE_TEST_VISITOR_HPP_INCLUDED
///\brief Visitor implementation for test purposes

#include "serialize/VisitorInterface.hpp"
#include "serialize/DataPathTree.hpp"

class DataPathTreeVisitor
	:public VisitorInterface
{
public:
	DataPathTreeVisitor(){}
	DataPathTreeVisitor( const DataPathTreeVisitor& o)
		:m_tree(o.m_tree),m_valuemap(o.m_valuemap){}
	DataPathTreeVisitor( const DataPathTree& tree_, const QMap<int,QVariant>& valuemap_)
		:m_tree(tree_),m_valuemap(valuemap_){}

	virtual ~DataPathTreeVisitor(){}

	virtual bool enter( const QString& name, bool /*writemode*/);

	///\brief Set the current node to the parent that called enter to this node.
	virtual void leave( bool /*writemode*/);

	///\brief Get the property of the current node by 'name'
	///\param[in] name name of the property
	///\return property variant (any type)
	virtual QVariant property( const QString& name);

	///\brief Set the property of the current node
	///\param[in] name name of the property
	///\param[in] value property value as variant (any type)
	///\return true on success
	virtual bool setProperty( const QString& name, const QVariant& value);

	///\brief Clear data of the currently visited node
	virtual void clear();

	///\brief Declare the end of data initialization
	virtual void endofDataFeed();

	///\brief Assign a value to a variable 'path' in the tree.
	///\param[in] path variable with '.' as structure element selection operator
	///\param[in] value to assign
	bool assign( const QString& path, const QVariant& value);

private:
	DataPathTree m_tree;
	QMap<int,QVariant> m_valuemap;
};

#endif
