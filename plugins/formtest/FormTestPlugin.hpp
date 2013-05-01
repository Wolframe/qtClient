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

#ifndef _FORM_TEST_PLUGIN_INCLUDED
#define _FORM_TEST_PLUGIN_INCLUDED

#include "FormPluginInterface.hpp"

#include <QWidget>
#include <QPushButton>

class FormTestPlugin : public QObject, public FormPluginInterface
{
	Q_OBJECT
	Q_INTERFACES( FormPluginInterface )
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA( IID "org.wolframe.qtClient.FormPluginInterface/1.0"  )
#endif // QT_VERSION >= 0x050000
	
	public:
		FormTestPlugin( );
		
		virtual QString name( ) const;
		virtual QString windowTitle( ) const;
		virtual QWidget *initialize( DataLoader *m_dataLoader, QWidget *_parent );
		virtual void gotAnswer( const QString& _tag, const QByteArray& _data );

	private:
		QWidget *m_widget;
		DataLoader *m_dataLoader;
		QPushButton *m_pushButton;
		int m_tagCounter;
		QString m_tag;

	private slots:
		void handleButtonPress( );
};

#endif // _FORM_TEST_PLUGIN_INCLUDED
