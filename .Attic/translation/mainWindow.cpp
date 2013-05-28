#include <QActionGroup>
#include <QLocale>
#include "mainWindow.hpp"
#include "application.hpp"

MainWindow::MainWindow(QWidget* parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	fillLanguages();
	retranslate();
}

MainWindow::~MainWindow()
{
}

void MainWindow::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		// apply the new language

		// all designed forms have retranslateUi() method
		ui.retranslateUi(this);

		// retranslate other widgets which weren't added in the designer
		retranslate();
	}
	QMainWindow::changeEvent(event);
}

void MainWindow::setLanguage(QAction* action)
{
	Application::setLanguage(action->data().toString());
}

void MainWindow::fillLanguages()
{
	languages = menuBar()->addMenu("");

	// make a group of language actions
	QActionGroup* actions = new QActionGroup(this);
	connect(actions, SIGNAL(triggered(QAction*)), this, SLOT(setLanguage(QAction*)));

	foreach (QString avail, Application::availableLanguages())
	{
		// figure out nice names for locales
		QLocale locale(avail);
		QString language = QLocale::languageToString(locale.language());
		QString country = QLocale::countryToString(locale.country());
		QString name = language + " (" + country + ")";

		// construct an action
		QAction* action = languages->addAction(name);
		action->setData(avail);
		action->setCheckable(true);
		actions->addAction(action);
	}
}

void MainWindow::retranslate()
{
	languages->setTitle(tr("Language"));
}
