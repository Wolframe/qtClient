#ifndef MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

protected:
	void changeEvent(QEvent* event);

private slots:
	void setLanguage(QAction* action);

private:
	void fillLanguages();
	void retranslate();

	Ui::MainWindow ui;

	// store translated widgets as members
	QMenu* languages;
};

#endif // MAINWINDOW_H
