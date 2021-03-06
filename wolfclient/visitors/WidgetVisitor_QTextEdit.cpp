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
#include "WidgetVisitor_QTextEdit.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QTextEdit::WidgetVisitorState_QTextEdit( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_textEdit(qobject_cast<QTextEdit*>(widget_)){}

void WidgetVisitorState_QTextEdit::clear()
{
	m_textEdit->clear();
}

QVariant WidgetVisitorState_QTextEdit::property( const QString& name)
{
	if (name == "text" || name.isEmpty())
	{
		return QVariant( m_textEdit->toPlainText());
	}
	if (name == "html")
	{
		return QVariant( m_textEdit->toHtml());
	}
	return QVariant();
}

bool WidgetVisitorState_QTextEdit::setProperty( const QString& name, const QVariant& data)
{
	if (name == "text" || name.isEmpty())
	{
		m_textEdit->setPlainText( data.toString());
		m_textLoaded = m_textEdit->toHtml();
		return true;
	}
	if (name == "html")
	{
		m_textEdit->setHtml( data.toString());
		m_textLoaded = m_textEdit->toHtml();
		return true;
	}
	return false;
}

void WidgetVisitorState_QTextEdit::setState( const QVariant& state)
{
	qDebug() << "set state for text edit" << m_textEdit->objectName();
	if (state.isValid()) m_textEdit->setHtml( state.toString());
}

QVariant WidgetVisitorState_QTextEdit::getState() const
{
	QString content = m_textEdit->toHtml();
	return (content != m_textLoaded)?QVariant( content):QVariant();
}

void WidgetVisitorState_QTextEdit::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_textEdit, SIGNAL( currentCharFormatChanged(const QTextCharFormat&)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_textEdit, SIGNAL( cursorPositionChanged()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_textEdit, SIGNAL( selectionChanged()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_textEdit, SIGNAL( textChanged()), &listener, SLOT( changed()), Qt::UniqueConnection);
			break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigPressed:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_textEdit->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QTextEdit::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_textEdit, SIGNAL( selectionChanged()), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_textEdit, SIGNAL( textChanged()), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


