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
#include "WidgetEnabler.hpp"
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <algorithm>

WidgetVisitorState_QLabel::WidgetVisitorState_QLabel( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_label(qobject_cast<QLabel*>(widget_))
{}

void WidgetVisitorState_QLabel::clear()
{
	m_label->setProperty( "_w_pictures", QVariant());
	QVariant origtext = m_label->property( "_w_origtext");
	if (!origtext.isValid())
	{
		m_label->setProperty( "_w_origtext", origtext = m_label->text());
	}
	m_label->clear();
	m_label->setText( origtext.toString());
}

QVariant WidgetVisitorState_QLabel::property( const QString& name)
{
	if (name.isEmpty() || name == "text")
	{
		return QVariant( m_label->text());
	}
	else if (name == "tooltip")
	{
		return QVariant( m_label->toolTip());
	}
	else if (name == "id")
	{
		return property( "_w_id");
	}
	return QVariant();
}

bool WidgetVisitorState_QLabel::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty() || name == "text")
	{
		m_label->setText( data.toString());
		m_label->adjustSize();
		return true;
	}
	if (name == "addtext")
	{
		QString sepstr = " ";
		QVariant sep = m_label->property( "separator");
		if (sep.isValid())
		{
			sepstr = sep.toString();
		}
		m_label->setText( m_label->text() + sepstr + data.toString());
		m_label->adjustSize();
		return true;
	}
	else if (name.size() == 1 && name.at(0) >= '1' && name.at(0) <= '9')
	{
		QString subst("%");
		subst.push_back( name.at(0));
		if (!m_label->property( "_w_origtext").isValid())
		{
			m_label->setProperty( "_w_origtext", m_label->text());
		}
		QString newText = m_label->text().replace( subst, data.toString());
		m_label->setText( newText);
		m_label->adjustSize();
		return true;
	}
	else if( name == "base64" ) {
		QPixmap p;
		p.loadFromData( QByteArray::fromBase64( data.toByteArray( ) ) );
		if( !p.isNull( ) ) {
			int w = std::min( m_label->width( ), p.width( ) );
			int h = std::min( m_label->height( ), p.height( ) );							
			m_label->setPixmap( p.scaled( QSize( w, h ), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
			m_label->adjustSize();
		}
	}
	else if (name == "addbase64")
	{
		QVariant pictures = m_label->property( "_w_pictures");
		QList<QVariant> lst;
		if (pictures.isValid() && pictures.type() == QVariant::List)
		{
			lst = pictures.toList();
		}
		lst.push_back( data);
		m_label->setProperty( "_w_pictures", QVariant(lst));
		m_label->adjustSize();
	}
	else if (name == "tooltip")
	{
		m_label->setToolTip( data.toString());
	}
	else if (name == "id")
	{
		m_label->setProperty( "_w_id", data);
	}
	return false;
}

void WidgetVisitorState_QLabel::setState( const QVariant& state)
{
	qDebug() << "set state for label" << m_label->objectName();
	if (state.isValid())
	{
		m_label->setText( state.toString());
		m_label->adjustSize();
	}
}

QVariant WidgetVisitorState_QLabel::getState() const
{
	return QVariant();
}

void WidgetVisitorState_QLabel::endofDataFeed()
{
	QVariant pictures = m_label->property( "_w_pictures");
	if (pictures.isValid() && pictures.type() == QVariant::List)
	{
		QList<QPixmap> pl;
		foreach (QVariant pic, pictures.toList())
		{
			QPixmap px;
			px.loadFromData( QByteArray::fromBase64( pic.toByteArray( ) ) );
			if (!px.isNull())
			{
				pl.push_back( px);
			}
		}
		if (pl.size() > 0)
		{
			QPixmap pic;
			QPainter painter( &pic);

			int ii = 0;
			int ww = m_label->width();
			int hh = m_label->height();
			double dd = (ww / 2) / pl.size();
			for (ii=0; ii<pl.size()-1; ++ii)
			{
				QRectF target( dd*ii, 0, dd*ii+dd, hh);
				QRectF source( 0.0, 0.0, pl.at(ii).width()/pl.size(), pl.at(ii).height());

				painter.drawPixmap( target, pl.at(ii), source);
			}
			QRectF target( ww/2, 0, ww, hh);
			QRectF source( 0.0, 0.0, pl.at(ii).width(), pl.at(ii).height());

			painter.drawPixmap( target, pl.at(ii), source);

			ww = std::min( m_label->width( ), pic.width( ) );
			hh = std::min( m_label->height( ), pic.height( ) );
			m_label->setPixmap( pic.scaled( QSize( ww, hh ), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
			m_label->adjustSize( );
		}
	}
}

void WidgetVisitorState_QLabel::connectWidgetEnabler( WidgetEnabler& /*enabler*/)
{
}

