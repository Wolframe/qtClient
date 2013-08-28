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
#ifndef _WOLFRAME_DATA_STRUCT_DESCRIPTION_MAP_HPP_INCLUDED
#define _WOLFRAME_DATA_STRUCT_DESCRIPTION_MAP_HPP_INCLUDED
///\brief Module to relate two hierarchical structures, one given by a
//	DataStructDescription and the other given by variable references in this
//	description to each other. The idea is that a map of elements in one
//	structure to elements in the other structure should preserve the grouping
//	of elements in the same substructure. The difficulty here is that elements
//	in the request/answer specification language of the Wolframe client
//	are adressed individually as atomic values or sets of atomic values. So the
//	grouping information is reconstructed by assuming that elements with a common
//	ancestor path belong to a group described by the longest common ancestor path
//	(see ->Model).

#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include "serialize/DataStructDescription.hpp"

/* Model
*
* Because widget element references in data structures are all atomic, the grouping of
* elements belonging to the same structure cannot be made explicitely. The grouping
* is done implicitely by grouping elements according their longest common ancestor
* element path. Elements "X.Y.A" and "X.Y.B" in the same structure are assumed
* to belong to a group "X.Y". This is important in arrays because there the individual
* addressing of the elements is not equivalent to addressing first the group and then
* the elements relatively. We have to assure that elements in the same group ("X.Y")
* are mapped to the same array element. This is done by opening the grouping element
* for every array element in the data and then addressing the sub elements of the group.
*
* The function 'getDataStructDescriptionMap( const DataStructDescription& )' returns
* a map that assigns a relative group path to every relevant grouping element
* in the description. Relevant are structure descriptions referenced by an self recursive
* indirection or descriptions pf array elements.
*/

typedef QList<QString> DataPath;
typedef QMap<const DataStructDescription*,DataPath> DataStructDescriptionMap;

///\brief Get a map of elements of a data structure description and its substructures to relative paths extracted from variable references.
bool getDataStructDescriptionMap( DataStructDescriptionMap& descrmap, const DataStructDescription& descr);

#endif

