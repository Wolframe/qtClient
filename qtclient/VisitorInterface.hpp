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
#ifndef _WOLFRAME_VISITOR_INTERFACE_HPP_INCLUDED
#define _WOLFRAME_VISITOR_INTERFACE_HPP_INCLUDED
///\brief Interface to a tree structure for extracting its data (request) and initializing
//	its data (answer).
#include <QString>
#include <QVariant>

struct VisitorInterface
{
	VisitorInterface(){}
	virtual ~VisitorInterface(){};

	///\brief Sets the current node to the child with name 'name'
	virtual bool enter( const QString& name, bool writemode)=0;

	///\brief Set the current node to the parent that called enter to this node.
	virtual void leave( bool writemode)=0;

	///\brief Get the property of the current node by 'name'
	///\param[in] name name of the property
	///\return property variant (any type)
	virtual QVariant property( const QString& name)=0;

	///\brief Set the property of the current node
	///\param[in] name name of the property
	///\param[in] value property value as variant (any type)
	///\return true on success
	virtual bool setProperty( const QString& name, const QVariant& value)=0;

	///\brief Clear data of the currently visited node
	virtual void clear()=0;

	///\brief Declare the end of data initialization
	virtual void endofDataFeed()=0;
};

#endif


