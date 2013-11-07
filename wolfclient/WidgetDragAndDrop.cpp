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
///\brief Implement drag and drop for widgets
#include "WidgetVisitor.hpp"
#include "WidgetDragAndDrop.hpp"
#include "WidgetId.hpp"
#include <QDrag>
#include <QMimeData>

static bool containsPoint( const QWidget* widget, const QVariant& cond)
{
	return widget->rect().contains( cond.toPoint(), true);
}

void mousePressEventHandleDrag( QWidget* this_, QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && this_->rect().contains(event->pos()))
	{
		WidgetVisitor visitor( this_);

		foreach (QWidget* subnode, visitor.findSubNodes( containsPoint, event->pos()))
		{
			WidgetVisitor subvisitor( subnode);
			if (subvisitor.hasDrag())
			{
				QDrag *drag = new QDrag(this_);
				QMimeData *mimeData = new QMimeData;
				mimeData->setData( WIDGETID_MIMETYPE, subvisitor.widgetid().toLatin1());
				drag->setMimeData( mimeData);
		
				Qt::DropAction dropAction = drag->exec( Qt::CopyAction | Qt::MoveAction);
				switch (dropAction)
				{
					case Qt::CopyAction: qDebug() << "Handle event copy action drag on widget" << subvisitor.widgetid(); break;
					case Qt::MoveAction: qDebug() << "Handle event move action drag on widget" << subvisitor.widgetid(); break;
					default: qCritical() << "internal: illegal state in handle drag event"; break;
				}
				return;
			}
		}
	}
}

