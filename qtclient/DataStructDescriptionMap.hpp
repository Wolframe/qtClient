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
#ifndef _WIDGET_DATA_STRUCT_DESCRIPTION_MAP_HPP_INCLUDED
#define _WIDGET_DATA_STRUCT_DESCRIPTION_MAP_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include "DataStructDescription.hpp"

typedef QList<QString> DataPath;
typedef QMap<DataStructDescription::Element*,DataPath> DataStructDescriptionMap;

/* MODEL
*
* Because widget element references in data structures are all atomic, the grouping of
* elements belonging to the same structure cannot be made explicitely. The grouping
* is done implicitely by grouping elements according their lowest common ancestor
* in the element path. Elements "X.Y.A" and "X.Y.B" in the same structure are assumed
* to belong to a unit "X.Y". This is important in array because there the individual
* addressing of the elements is not equivalent. We have to assure that elements
* in the same group ("X.Y") are mapped to the same array element. This is done by
* opening the grouping element for every array element in the data and then addressing
* the sub elements of the group.
*
* The function 'getDataStructDescriptionMap( const DataStructDescription& )' returns
* a map that assigns a relative group path to every grouping element in the description.
*/

///\brief Get a map of elements of a data structure description and its substructures to relative paths extracted from variable references.
DataStructDescriptionMap
	getDataStructDescriptionMap( const DataStructDescription& descr);

#endif

