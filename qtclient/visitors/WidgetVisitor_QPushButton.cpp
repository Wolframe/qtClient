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
#include "WidgetVisitor_QPushButton.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QPushButton::WidgetVisitorState_QPushButton( QWidget* widget_)
	:WidgetVisitor::State(widget_)
	,m_pushButton(qobject_cast<QPushButton*>(widget_)){}

void WidgetVisitorState_QPushButton::clear()
{}

QVariant WidgetVisitorState_QPushButton::property( const QString& /*name*/)
{
	return QVariant();
}

bool WidgetVisitorState_QPushButton::setProperty( const QString& /*name*/, const QVariant& /*data*/)
{
	return false;
}

void WidgetVisitorState_QPushButton::setState( const QVariant& state)
{
	qDebug() << "set state for push button" << m_pushButton->objectName();
	int stateval = state.toInt();
	m_pushButton->setEnabled( ((stateval & 1) != 0));
	m_pushButton->setDown( ((stateval & 2) != 0));
}

QVariant WidgetVisitorState_QPushButton::getState() const
{
	int state = 0;
	if (m_pushButton->isEnabled()) state |= 1;
	if (m_pushButton->isDown()) state |= 2;
	return QVariant( state);
}

void WidgetVisitorState_QPushButton::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_pushButton, SIGNAL( clicked( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_pushButton, SIGNAL( released()), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_pushButton, SIGNAL( toggled( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_pushButton, SIGNAL( pressed()), &enabler, SLOT( changed()), Qt::UniqueConnection);
}

