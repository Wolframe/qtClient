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

#ifndef _WIDGET_TREE_HPP_INCLUDED
#define _WIDGET_TREE_HPP_INCLUDED
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include "FormCall.hpp"
#include <QWidget>
#include <QPushButton>
#include <QSignalMapper>

class WidgetTree
{
public:
	WidgetTree( DataLoader *_dataLoader, QHash<QString,QVariant>* _globals, bool debug_);
	bool initialize( QWidget* ui_, QWidget* oldUi, const QString& formcall, const QString& logid);

	QVariant getWidgetStates() const;
	void setWidgetStates( const QVariant& state);

	QWidget* deliverAnswer( const QString& tag, const QByteArray& content, QString& followform);
	QWidget* deliverError( const QString& tag, const QByteArray& content);

	void saveVariables();
	void saveWidgetStates();

	QString popCurrentForm();	//< for _RESET_
	QString popPreviousForm();	//< for _CLOSE_

private:
	void setEnablers( QWidget* widget, const QList<WidgetEnablerImpl::Trigger>& trigger);
	void setPushButtonEnablers( QPushButton* pushButton);
	///\brief sets the enablers declared with 'state:..' properties
	void setDeclaredEnablers( QWidget* widget);
	void signalEnablers();
	void handleDatasignal( WidgetVisitor& visitor, const char* signame);

private:
	FormCall m_formCall;
	WidgetVisitor m_visitor;
	QHash<QString,QVariant>* m_globals;			// global variable assignments read in init form and written in switch/close form
	DataLoader *m_dataLoader;				// data loader reference
	QHash<QString,QList<WidgetListenerR> > m_listeners;	// widget signal listeners for this form
	QHash<QString,QList<WidgetEnablerR> > m_enablers;	// objects holding the logic for enabling buttons when conditions defined by properties referenced are met
	QVariant m_state;					// states of the widgets in the tree (used to restore state of previous window on the stack after _CLOSE_)
	bool m_debug;						// true, if in debug mode
};

#endif

