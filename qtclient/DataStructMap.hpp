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
#ifndef _WOLFRAME_DATA_STRUCT_MAP_HPP_INCLUDED
#define _WOLFRAME_DATA_STRUCT_MAP_HPP_INCLUDED
///\brief Module with function to build a DataStruct from a description and the widget data
#include "VisitorInterface.hpp"
#include "DataStruct.hpp"

///\brief Fill the object accessed over the visitor interface with content from the
//	data structure 'data'. Use the addressed by variable references in the data
//	description for grouping elements together.
bool putDataStruct( const DataStruct& data, VisitorInterface* vi);

///\brief Build the data structure 'data' from content addressed by variable
//	references in the data description retrieved over the visitor interface.
bool getDataStruct( DataStruct& data, VisitorInterface* vi);

#endif


