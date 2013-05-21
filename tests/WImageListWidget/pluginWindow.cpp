#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtGui>

#include <QDebug>

#include "pluginWindow.hpp"

#include "WImageListWidget.hpp"

// Create the test window
PluginWindow::PluginWindow()
{
	qDebug() << "Create Plugin Group Box";
	createPluginGroupBox();
	qDebug() << "Create Operations Group Box";
	createOperationsGroupBox();
	qDebug() << "Create Selected Group Box";
	createSelectedGroupBox();
	qDebug() << "Create Properties Group Box";
	createPropertiesGroupBox();

	QGridLayout *layout = new QGridLayout;
	layout->addWidget( m_pluginGroupBox, 0, 0 );
	layout->addWidget( m_selectedGroupBox, 0, 1 );
	layout->addWidget( m_propertiesGroupBox, 1, 0 );
	layout->addWidget( m_operationsGroupBox, 1, 1 );

	layout->setSizeConstraint( QLayout::SetFixedSize );
	setLayout( layout );

	m_pluginLayout->setRowMinimumHeight( 0, m_selector->sizeHint().height() );
	m_pluginLayout->setColumnMinimumWidth( 0, m_selector->sizeHint().width() );

	setWindowTitle( tr( "Image List View/Widget Test" ));
}

// Plugin group (box)
void PluginWindow::createPluginGroupBox()
{
	m_pluginGroupBox = new QGroupBox( tr( "ImageList" ));
//!!!!!!!!!
	qDebug() << "Create selector";
	m_selector = new WImageListWidget();
	qDebug() << "Selector created";

//	connect( m_selector, SIGNAL( clicked( QModelIndex )), this, SLOT( selectorClicked()) );
//	connect( m_selector, SIGNAL( doubleClicked( QModelIndex )), this, SLOT( itemSelected()) );
//!!!!!!!!!
	m_pluginLayout = new QGridLayout;
	m_pluginLayout->addWidget( m_selector, 0, 0, Qt::AlignCenter );
	m_pluginGroupBox->setLayout( m_pluginLayout );
	qDebug() << "Plugin Group Box created";
}

// Operations group (box)
void PluginWindow::createOperationsGroupBox()
{
	m_operationsGroupBox = new QGroupBox( tr( "Operations" ));

// Buttons
	m_addImage = new QPushButton( tr( "&Add image(s) from file(s)..." ));
	connect( m_addImage, SIGNAL( clicked( bool )), this, SLOT( addImagesFromFiles()) );

	m_removeImage = new QPushButton( tr( "&Remove selected image(s)" ));
	connect( m_removeImage, SIGNAL( clicked( bool )), this, SLOT( removeSelectedImages()) );

// Set elements into the grid
	QGridLayout *opLayout = new QGridLayout;
	opLayout->addWidget( m_addImage, 1, 1 );
	opLayout->addWidget( m_removeImage, 2, 1 );
	m_operationsGroupBox->setLayout( opLayout );

	qDebug() << "Operations Group Box created";
}

// Show selected items group (box)
void PluginWindow::createSelectedGroupBox()
{
	m_selectedGroupBox = new QGroupBox( tr( "Selected" ));

	m_selectedList = new QListWidget();

	QGridLayout *selectedLayout = new QGridLayout;
	selectedLayout->addWidget( m_selectedList, 0, 0 );

	m_selectedGroupBox->setLayout( selectedLayout );

	qDebug() << "Selected Group Box created";
}

// Show properties group (box)
void PluginWindow::createPropertiesGroupBox()
{
	m_propertiesGroupBox = new QGroupBox( tr( "Widget properties" ));

	m_selectedItems = new QLabel( tr( "Number of selected items: 0" ));
	m_totalItems = new QLabel( tr( "Total number of items: 0" ));

	QGridLayout *propLayout = new QGridLayout;
	propLayout->addWidget( m_selectedItems, 0, 0 );
	propLayout->addWidget( m_totalItems, 1, 0 );

	m_propertiesGroupBox->setLayout( propLayout );

	qDebug() << "Properties Group Box created";
}


// Add image file(s)
void PluginWindow::addImagesFromFiles()
{
	QFileDialog	fileDialog( this );
	fileDialog.setFileMode( QFileDialog::ExistingFiles );
	fileDialog.setAcceptMode( QFileDialog::AcceptOpen );
	fileDialog.setNameFilter( tr( "Images (*.png *.xpm *.jpg);;All files (*.*)" ));

	QStringList	fileNames;
	if ( fileDialog.exec() )	{
		fileNames = fileDialog.selectedFiles();
		for ( QStringList::const_iterator i = fileNames.begin(); i != fileNames.end(); i++ )	{
			QImage img( *i );
			m_selector->addImage( img, *i );
		}
	}
}

void PluginWindow::removeSelectedImages()
{
	QMessageBox::critical( this, tr( "Plugin tester" ), tr( "Not implemented yet" ) );
}

void PluginWindow::selectorClicked()
{
	QMessageBox::critical( this, tr( "Plugin tester" ), tr( "Not implemented yet" ) );
}

void PluginWindow::itemSelected()
{
	QMessageBox::critical( this, tr( "Plugin tester" ), tr( "Not implemented yet" ) );
}
