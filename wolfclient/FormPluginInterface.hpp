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

#ifndef _FORM_PLUGIN_INTERFACE_INCLUDED
#define _FORM_PLUGIN_INTERFACE_INCLUDED

#include <QtPlugin>
#include <QString>
#include <QByteArray>
#include <QWidget>
#include <QHash>
#include <QVariant>

#include "DataLoader.hpp"
#include "FormCall.hpp"

class FormPluginInterface
{	
	public:
		virtual ~FormPluginInterface( ) { }
		
		virtual QString name( ) const = 0;
		virtual QString windowTitle( ) const = 0;
		virtual QWidget *createForm( const FormCall &formCall, DataLoader *_dataLoader, bool _debug, QHash<QString,QVariant>* _globals, QWidget *_parent ) = 0;
		virtual void gotAnswer( const QString& _tag, const QByteArray& _data ) = 0;
		virtual void gotError( const QString& tag_, const QByteArray& error_ ) = 0;
};

Q_DECLARE_INTERFACE( FormPluginInterface, "org.wolframe.wolfclient.FormPluginInterface/1.0" )

#endif // _FORM_PLUGIN_INTERFACE_INCLUDED
