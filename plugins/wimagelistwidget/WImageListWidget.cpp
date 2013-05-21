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

#include <QGridLayout>
//#include <QtConcurrentMap>

#include "WImageListWidget.hpp"

#include <QDebug>

static const int imageSize = 60;

static QImage scale( const QImage& image, const QSize size )
{
	return image.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}


WImageListWidget::WImageListWidget( QWidget *parent )
	: QListWidget( parent )
{
//	QGridLayout* m_gridLayout = new QGridLayout( this );
//	m_gridLayout->addWidget( this );

	setViewMode( QListView::IconMode );
	setUniformItemSizes( true );
	setSelectionRectVisible( true );
	setMovement( QListView::Static );
	setSelectionMode( QListView::SingleSelection );
	setEditTriggers( QAbstractItemView::NoEditTriggers );

	QListView::setIconSize( QSize( imageSize, imageSize ) );
	setResizeMode( QListView::Adjust );

//	m_imageScaler = new QFutureWatcher<QImage>( this );
//	connect( m_imageScaler, SIGNAL( resultReadyAt( int ) ), SLOT( addImage( int ) ) );
//	connect( m_imageScaler, SIGNAL( finished() ), SLOT( finished() ) );
}

WImageListWidget::~WImageListWidget()
{
//	m_imageScaler->cancel();
//	m_imageScaler->waitForFinished();
}


void WImageListWidget::setIconSize( QSize& size )
{
	QListView::setIconSize( size );
}

//void WImageListWidget::addImage( const QString imageFile, const QString toolTip, const QString statusTip )
//{
//	qDebug() << "show image " << imageFile;
////	m_imageScaler->setFuture( QtConcurrent::mapped( imageFile, scale ) );
//	QListWidgetItem* imageItem = new QListWidgetItem();
////	imageItem->setIcon( QIcon( QPixmap::fromImage( m_imageScaler->result() ) ) );
//	imageItem->setIcon( QIcon( QPixmap::fromImage( scale( imageFile, QListView::iconSize() ) ) ) );
//	if ( ! toolTip.isEmpty() )
//		imageItem->setToolTip( toolTip );
//	if ( ! statusTip.isEmpty() )
//		imageItem->setToolTip( statusTip );

//	addItem( imageItem );
//	qDebug() << "image '" << imageFile << "' available";
//}

void WImageListWidget::addImage( const QImage image, const QString toolTip, const QString statusTip )
{
	QListWidgetItem* imageItem = new QListWidgetItem();
	imageItem->setIcon( QIcon( QPixmap::fromImage( scale( image, QListView::iconSize() ) ) ) );
	if ( ! toolTip.isEmpty() )
		imageItem->setToolTip( toolTip );
	if ( ! statusTip.isEmpty() )
		imageItem->setToolTip( statusTip );

	addItem( imageItem );
}

//void WImageListWidget::finished()
//{
//}

void WImageListWidget::imageClicked( QModelIndex index )
{
	qDebug() << "image clicked at " << index.row();
}

void WImageListWidget::imageDoubleClicked( QModelIndex index )
{
	qDebug() << "image double clicked at " << index.row();
}
