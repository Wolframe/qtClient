
#include <QtGui>

#include "pluginWindow.hpp"

#include "ImageListViewDialog.hpp"

// Create the test window
PluginWindow::PluginWindow()
{
	createPluginGroupBox();
	createOperationsGroupBox();
	createSelectedGroupBox();
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
	m_selector = new ImageListViewDialog( 120, 90 );

//	connect( m_selector, SIGNAL( clicked( QModelIndex )), this, SLOT( selectorClicked()) );
//	connect( m_selector, SIGNAL( doubleClicked( QModelIndex )), this, SLOT( itemSelected()) );
//!!!!!!!!!
	m_pluginLayout = new QGridLayout;
	m_pluginLayout->addWidget( m_selector, 0, 0, Qt::AlignCenter );
	m_pluginGroupBox->setLayout( m_pluginLayout );
}

// Operations group (box)
void PluginWindow::createOperationsGroupBox()
{
	m_operationsGroupBox = new QGroupBox( tr( "Operations" ));
// Build locale combo
	m_localeCombo = new QComboBox;
	int curLocaleIndex = -1;
	int index = 0;
	for ( int langIdx = QLocale::C; langIdx <= QLocale::LastLanguage; langIdx++ )	{
		QLocale::Language lang = static_cast< QLocale::Language >( langIdx ) ;
		QList< QLocale::Country > countries = QLocale::countriesForLanguage( lang );
		for ( int i = 0; i < countries.count(); i++ )	{
			QLocale::Country country = countries.at( i );
			QString label = QLocale::languageToString( lang );
			label += QLatin1Char( '/' );
			label += QLocale::countryToString( country );
			QLocale locale( lang, country );
			if ( this->locale().language() == lang && this->locale().country() == country )
				curLocaleIndex = index;
			m_localeCombo->addItem( label, locale );
			index++;
		}
	}
	if ( curLocaleIndex != -1 )
		m_localeCombo->setCurrentIndex( curLocaleIndex );
	m_localeLabel = new QLabel( tr( "&Locale" ));
	m_localeLabel->setBuddy( m_localeCombo );

	connect( m_localeCombo, SIGNAL( currentIndexChanged( int )),
		 this, SLOT( localeChanged( int )) );
// End of locale combo

// Buttons
	m_addImage = new QPushButton( tr( "&Add image(s) from file(s)..." ));
	connect( m_addImage, SIGNAL( clicked( bool )), this, SLOT( addImagesFromFiles()) );

	m_removeImage = new QPushButton( tr( "&Remove selected image(s)" ));
	connect( m_removeImage, SIGNAL( clicked( bool )), this, SLOT( removeSelectedImages()) );

// Set elements into the grid
	QGridLayout *opLayout = new QGridLayout;
	opLayout->addWidget( m_localeLabel, 0, 0 );
	opLayout->addWidget( m_localeCombo, 0, 1 );
	opLayout->addWidget( m_addImage, 1, 1 );
	opLayout->addWidget( m_removeImage, 2, 1 );

	m_operationsGroupBox->setLayout( opLayout );
}

// Show selected items group (box)
void PluginWindow::createSelectedGroupBox()
{
	m_selectedGroupBox = new QGroupBox( tr( "Selected" ));

	m_selectedList = new QListWidget();

	QGridLayout *selectedLayout = new QGridLayout;
	selectedLayout->addWidget( m_selectedList, 0, 0 );

	m_selectedGroupBox->setLayout( selectedLayout );
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
}

// Slots
void PluginWindow::localeChanged( int index )
{
	m_selector->setLocale( m_localeCombo->itemData( index ).toLocale() );
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
		for ( QStringList::const_iterator i = fileNames.begin(); i != fileNames.end(); i++ )
			m_selector->addImage( *i, *i );
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
