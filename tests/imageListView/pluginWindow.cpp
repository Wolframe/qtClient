
#include <QtGui>

#include "pluginWindow.hpp"

PluginWindow::PluginWindow()
{
	createPluginGroupBox();
	createOperationsGroupBox();
	createSelectedGroupBox();
	createPropertiesGroupBox();

	QGridLayout *layout = new QGridLayout;
	layout->addWidget( pluginGroupBox, 0, 0 );
	layout->addWidget( selectedGroupBox, 0, 1 );
	layout->addWidget( propertiesGroupBox, 1, 0 );
	layout->addWidget( operationsGroupBox, 1, 1 );

	layout->setSizeConstraint( QLayout::SetFixedSize );
	setLayout( layout );

	pluginLayout->setRowMinimumHeight( 0, selector->sizeHint().height() );
	pluginLayout->setColumnMinimumWidth( 0, selector->sizeHint().width() );

	setWindowTitle( tr( "Image List View/Widget Test" ));
}

void PluginWindow::createPluginGroupBox()
{
	pluginGroupBox = new QGroupBox( tr( "ImageList" ));
//!!!!!!!!!
	selector = new QListView;
	connect( selector, SIGNAL( clicked( QModelIndex )), this, SLOT( selectorClicked()) );
	connect( selector, SIGNAL( doubleClicked( QModelIndex )), this, SLOT( itemSelected()) );
//!!!!!!!!!
	pluginLayout = new QGridLayout;
	pluginLayout->addWidget( selector, 0, 0, Qt::AlignCenter );
	pluginGroupBox->setLayout( pluginLayout );
}

void PluginWindow::createOperationsGroupBox()
{
	operationsGroupBox = new QGroupBox( tr( "Operations" ));
// Build locale combo
	localeCombo = new QComboBox;
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
			localeCombo->addItem( label, locale );
			index++;
		}
	}
	if ( curLocaleIndex != -1 )
		localeCombo->setCurrentIndex( curLocaleIndex );
	localeLabel = new QLabel( tr( "&Locale" ));
	localeLabel->setBuddy( localeCombo );

	connect( localeCombo, SIGNAL( currentIndexChanged( int )),
		 this, SLOT( localeChanged( int )) );
// End of locale combo

// Buttons
	addImage = new QPushButton( tr( "&Add image(s) from file(s)..." ));
	connect( addImage, SIGNAL( clicked( bool )), this, SLOT( addImagesFromFiles()) );

	removeImage = new QPushButton( tr( "&Remove selected image(s)" ));
	connect( removeImage, SIGNAL( clicked( bool )), this, SLOT( removeSelectedImages()) );

// Set elements into the grid
	QGridLayout *opLayout = new QGridLayout;
	opLayout->addWidget( localeLabel, 0, 0 );
	opLayout->addWidget( localeCombo, 0, 1 );
	opLayout->addWidget( addImage, 1, 1 );
	opLayout->addWidget( removeImage, 2, 1 );

	operationsGroupBox->setLayout( opLayout );
}

void PluginWindow::createSelectedGroupBox()
{
	selectedGroupBox = new QGroupBox( tr( "Selected" ));

	selectedList = new QListWidget();

	QGridLayout *selectedLayout = new QGridLayout;
	selectedLayout->addWidget( selectedList, 0, 0 );

	selectedGroupBox->setLayout( selectedLayout );
}

void PluginWindow::createPropertiesGroupBox()
{
	propertiesGroupBox = new QGroupBox( tr( "Widget properties" ));

	selectedItems = new QLabel( tr( "Number of selected items: 0" ));
	totalItems = new QLabel( tr( "Total number of items: 0" ));

	QGridLayout *propLayout = new QGridLayout;
	propLayout->addWidget( selectedItems, 0, 0 );
	propLayout->addWidget( totalItems, 1, 0 );

	propertiesGroupBox->setLayout( propLayout );
}

// Slots
void PluginWindow::localeChanged( int index )
{
	selector->setLocale( localeCombo->itemData( index ).toLocale() );
}

void PluginWindow::addImagesFromFiles()
{
	QFileDialog	fileDialog( this );
	fileDialog.setFileMode( QFileDialog::ExistingFiles );
	fileDialog.setAcceptMode( QFileDialog::AcceptOpen );
	fileDialog.setNameFilter( tr( "Images (*.png *.xpm *.jpg);;All files (*.*)" ));

	QStringList	fileNames;
	if ( fileDialog.exec() )	{
		fileNames = fileDialog.selectedFiles();
		imageFiles += fileNames;
	}
	QMessageBox::critical( this, tr( "Plugin tester" ), tr( "Not implemented yet" ) );
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
