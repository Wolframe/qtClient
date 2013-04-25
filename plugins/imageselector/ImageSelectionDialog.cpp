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
//
//
//

#include <QGridLayout>
#include <QtConcurrentMap>

#include "ImageSelectionDialog.hpp"

static const int imageSize = 60;

static QImage scale( const QString &imageFileName )
{
	QImage image( imageFileName );
	return image.scaled( QSize( imageSize, imageSize ),
			     Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

ImageSelectionDialog::ImageSelectionDialog( QStringList imageNamesList, QWidget *parent )
	: QWidget( parent ), m_imageNamesList( imageNamesList )
{

	QGridLayout* m_gridLayout = new QGridLayout( this );
	m_imageListView = new QListView( this );
	m_gridLayout->addWidget( m_imageListView );

	m_imageListView->setViewMode( QListView::IconMode );
	m_imageListView->setUniformItemSizes( true );
	m_imageListView->setSelectionRectVisible( true );
	m_imageListView->setMovement( QListView::Static );
	m_imageListView->setSelectionMode( QListView::SingleSelection );
	m_imageListView->setEditTriggers( QAbstractItemView::NoEditTriggers );

	m_imageListView->setResizeMode( QListView::Adjust );

	m_standardModel = new QStandardItemModel( this );
	m_imageListView->setModel( m_standardModel );

	m_imageScaler = new QFutureWatcher<QImage>( this );
	connect( m_imageScaler, SIGNAL( resultReadyAt( int ) ), SLOT( showImage( int ) ) );
	connect( m_imageScaler, SIGNAL( finished() ), SLOT( finished() ) );

	m_imageScaler->setFuture( QtConcurrent::mapped( m_imageNamesList, scale ) );

	connect( m_imageListView, SIGNAL( doubleClicked( QModelIndex ) ), SLOT( imageClicked( QModelIndex ) ) );
}

ImageSelectionDialog::~ImageSelectionDialog()
{
	m_imageScaler->cancel();
	m_imageScaler->waitForFinished();
}

void ImageSelectionDialog::showImage( int num )
{
	QStandardItem* imageitem = new QStandardItem();
	imageitem->setIcon( QIcon( QPixmap::fromImage( m_imageScaler->resultAt( num ) ) ) );
	m_standardModel->appendRow( imageitem );
}

void ImageSelectionDialog::finished()
{
}

void ImageSelectionDialog::imageClicked(QModelIndex index)
{
	if ( index.row() < m_imageNamesList.count() )	{
		qDebug() << "image selected " << m_imageNamesList.at( index.row() );
		close();
	}
}
