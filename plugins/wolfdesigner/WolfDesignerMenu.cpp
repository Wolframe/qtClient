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

#include "WolfDesignerMenu.hpp"

#include <QAction>
#include <QMessageBox>

#include <QDesignerFormWindowInterface>

#include "WolfDesignerDialog.hpp"

WolfDesignerMenu::WolfDesignerMenu( QWidget *_widget, QObject *_parent )
	: QObject( _parent ), m_widget( _widget )
{
	QAction *action = new QAction( tr( "Wolframe Controls..." ), this );
	connect( action, SIGNAL( triggered( ) ), this, SLOT( edit( ) ) );
	m_actions.append( action );
}

QList<QAction *> WolfDesignerMenu::taskActions( ) const
{
	return m_actions;
}

QAction *WolfDesignerMenu::preferredEditAction( ) const
{
	return m_actions[0];
}

void WolfDesignerMenu::edit( )
{
	WolfDesignerDialog d( m_widget, m_widget );
	d.exec( );
}

WolfDesignerMenuFactory::WolfDesignerMenuFactory( QExtensionManager *_parent )
	: QExtensionFactory( _parent )
{
}

QObject *WolfDesignerMenuFactory::createExtension( QObject *_object, const QString &_iid, QObject *_parent ) const
{
	if( _iid != Q_TYPEID( QDesignerTaskMenuExtension ) ) {
		return 0;
	}
	
	if( qobject_cast<QWidget *>( _object ) ) {

		QWidget *w = qobject_cast<QWidget *>( _object );

		// the menu is requested once per widget and lifetime,
		// so we can't make things depend on properties or states!

		//~ QDesignerFormWindowInterface *formWindow;
		//~ formWindow = QDesignerFormWindowInterface::findFormWindow( w );

//~ 
     //~ formWindow->cursor()->setProperty(myWidget, myProperty, newValue);
				
		QString className = w->metaObject( )->className( );
		
		// request/answer menues just for the top level widget in
		// the form editor, otherwise we loose default edit actions
		// (F2, double click) for example for text fields, push buttons
		//~ if( className == "QDesignerWidget" ) {
			return new WolfDesignerMenu( w, _parent );
		//~ }
	}
	
	return 0;
}
