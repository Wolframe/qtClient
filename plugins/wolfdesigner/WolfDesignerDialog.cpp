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

#include "WolfDesignerDialog.hpp"

#include <QDesignerFormWindowInterface>
#include <QDesignerFormEditorInterface>
#include <QDesignerPropertyEditorInterface>
#include <QDesignerPropertySheetExtension>
#include <QExtensionManager>
#include <QDesignerDynamicPropertySheetExtension>
#include <QDesignerFormWindowCursorInterface>

#include <QMessageBox>

WolfDesignerDialog::WolfDesignerDialog( QWidget *_widget, QWidget *_parent )
	: QDialog( _parent ), m_widget( _widget )
{
	setupUi( this );

	connect( buttonBox, SIGNAL( accepted( ) ), this, SLOT( save( ) ) );
	connect( buttonBox, SIGNAL( rejected( ) ), this, SLOT( cancel( ) ) );
	
	initialize( );
}

void WolfDesignerDialog::initialize( )
{
	form->setPlainText( m_widget->property( "form" ).toString( ) );
	request->setPlainText( m_widget->property( "action" ).toString( ) );
	answer->setPlainText( m_widget->property( "answer" ).toString( ) );
}

void WolfDesignerDialog::saveProperty( QDesignerFormWindowInterface *_formWindow,
	QWidget *_widget, const QString &_name, const QString &_value )
{
	// funny functions, no return value, so lets try to set the property
	// over the form window cursor, then check if the value is around,
	// if not we go over QObject to create the property with the proper value
	// We also check if the value differs from the now value before overwriting
	// it (not to trouble the Qt designers history)
	
	// the history doesn't work that way, but at least the changed state of
	// the widget changes
	//~ cursor->setProperty( _name, _value );
	
	//~ cursor->setWidgetProperty( _widget, _name, _value );
	//~ QVariant newValue = _widget->property( _name.toLatin1( ) );
	//~ if( newValue.isNull( ) ) {
		//~ // gets added to widget, but not to designer property sheet!
		//~ _widget->setProperty( _name.toLatin1( ), _value.toLatin1( ) );
	//~ }
	
	//~ QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow( _widget );
	//~ Q_ASSERT( formWindow != 0 );
	
	//~ QDesignerPropertyEditorInterface *propertyEditor =
		//~ formWindow->core( )->propertyEditor( ); 
	//~ propertyEditor->setPropertyValue( _name, _value, true );
		
	//~ QDesignerPropertySheetExtension *propertySheet = 0;
	//~ QExtensionManager *manager = formWindow->core( )->extensionManager( );
//~ 
	//~ propertySheet = qt_extension<QDesignerPropertySheetExtension*>( manager, _widget );
	//~ int index = propertySheet->indexOf( _name );

	//~ // property not created, maybe we have to register it first?
	//~ 
	//~ propertySheet->setProperty( index, _value );
	//~ propertySheet->setChanged( index, true );
//~ 
	//~ delete propertySheet;	
	
	QDesignerFormWindowCursorInterface *cursor = _formWindow->cursor( );
	
	cursor->setWidgetProperty( _widget, _name, _value );
	QVariant newValue = _widget->property( _name.toLatin1( ) );
	if( newValue.isNull( ) && !newValue.toString( ).isEmpty( ) ) {
		// an internal interface! we should really not use it!		
		QDesignerDynamicPropertySheetExtension *propertySheet = 0;
		QExtensionManager *manager = _formWindow->core( )->extensionManager( );
		propertySheet = qt_extension<QDesignerDynamicPropertySheetExtension *>( manager, _widget );

		if( propertySheet->canAddDynamicProperty( _name ) ) {
			_widget->setProperty( _name.toLatin1( ), _value.toLatin1( ) );

#ifndef QT_NO_DEBUG 
			int idx = propertySheet->addDynamicProperty( _name, _value );
#endif
			Q_ASSERT( idx > 0 );
		}
		
		// no delete propertySheet ?! segfaults on undo and on several other
		// occasions
	}
}

void WolfDesignerDialog::save( )
{
	QDesignerFormWindowInterface *formWindow;
	
	formWindow = QDesignerFormWindowInterface::findFormWindow( m_widget );
	
	saveProperty( formWindow, m_widget, "form", form->toPlainText( ) );
	saveProperty( formWindow, m_widget, "action", request->toPlainText( ) );
	saveProperty( formWindow, m_widget, "answer", answer->toPlainText( ) );
		
	accept( );
}

void WolfDesignerDialog::cancel( )
{
	reject( );
}
