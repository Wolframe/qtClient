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
#include "WidgetVisitor_QLineEdit.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QLineEdit::WidgetVisitorState_QLineEdit( QWidget* widget_)
	:WidgetVisitor::State(widget_)
	,m_lineEdit(qobject_cast<QLineEdit*>(widget_)){}

void WidgetVisitorState_QLineEdit::clear()
{
	m_lineEdit->clear();
}

QVariant WidgetVisitorState_QLineEdit::property( const QString& name)
{
	if (name.isEmpty())
	{
		return QVariant( m_lineEdit->text());
	}
	return QVariant();
}

bool WidgetVisitorState_QLineEdit::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_lineEdit->setText( data.toString());
		return true;
	}
	return false;
}

void WidgetVisitorState_QLineEdit::setState( const QVariant& state)
{
	qDebug() << "Restoring tree state for line edit" << m_lineEdit->objectName();
	if (state.isValid()) m_lineEdit->setText( state.toString());
}

QVariant WidgetVisitorState_QLineEdit::getState() const
{
	return QVariant( m_lineEdit->text());
}

void WidgetVisitorState_QLineEdit::connectDataSignals( WidgetVisitor::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetVisitor::SigChanged:
			QObject::connect( m_lineEdit, SIGNAL( cursorPositionChanged(int,int)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_lineEdit, SIGNAL( editingFinished()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_lineEdit, SIGNAL( returnPressed()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_lineEdit, SIGNAL( selectionChanged()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_lineEdit, SIGNAL( textChanged( const QString & text)), &listener, SLOT( changed()), Qt::UniqueConnection);
			// redundant (to textChanged):
			// QObject::connect( m_lineEdit, SIGNAL( textEdited( const QString & text)), &listener, SLOT( changed()));
			break;
		case WidgetVisitor::SigActivated:
		case WidgetVisitor::SigEntered:
		case WidgetVisitor::SigPressed:
		case WidgetVisitor::SigClicked:
		case WidgetVisitor::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_lineEdit->metaObject()->className() << WidgetVisitor::dataSignalTypeName(dt);
	}
}

void WidgetVisitorState_QLineEdit::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_lineEdit, SIGNAL( selectionChanged()), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_lineEdit, SIGNAL( textEdited( const QString&)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}

