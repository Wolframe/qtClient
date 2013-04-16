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
#include "WidgetVisitor_QLabel.hpp"
#include "WidgetListener.hpp"
#include <QDebug>

WidgetVisitorState_QLabel::WidgetVisitorState_QLabel( QWidget* widget_)
	:WidgetVisitor::State(widget_)
	,m_label(qobject_cast<QLabel*>(widget_)){}

void WidgetVisitorState_QLabel::clear()
{
	m_label->clear();
}

QVariant WidgetVisitorState_QLabel::property( const QString& name)
{
	if (name.isEmpty())
	{
		return QVariant( m_label->text());
	}
	return QVariant();
}

bool WidgetVisitorState_QLabel::setProperty( const QString& name, const QVariant& data)
{
	/*[-]*/qDebug() << "WidgetVisitorState_QLabel::setProperty" << name << data;
	if (name.isEmpty())
	{
		m_label->setText( data.toString());
		return true;
	}
	return false;
}

const QList<QString>& WidgetVisitorState_QLabel::dataelements() const
{
	static const DataElements ar( "", 0);
	return ar;
}

void WidgetVisitorState_QLabel::setState( const QVariant& state)
{
	qDebug() << "Restoring tree state for line edit" << m_label->objectName();
	if (state.isValid()) m_label->setText( state.toString());
}

QVariant WidgetVisitorState_QLabel::getState() const
{
	return QVariant( m_label->text());
}
