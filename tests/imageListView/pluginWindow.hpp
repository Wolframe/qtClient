#ifndef _WINDOW_HPP_INCLUDED
#define _WINDOW_HPP_INCLUDED

#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QGridLayout;
class QGroupBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QListView;
class QListWidget;
QT_END_NAMESPACE

class PluginWindow : public QWidget
{
	Q_OBJECT

public:
	PluginWindow();

private slots:
	void localeChanged( int index );
	void addImagesFromFiles();
	void removeSelectedImages();
	void selectorClicked();
	void itemSelected();

private:
	void createPluginGroupBox();
	void createOperationsGroupBox();
	void createSelectedGroupBox();
	void createPropertiesGroupBox();

	QGroupBox	*pluginGroupBox;
	QGridLayout	*pluginLayout;
	QListView	*selector;

	QGroupBox	*operationsGroupBox;
	QLabel		*localeLabel;
	QComboBox	*localeCombo;
	QPushButton	*addImage;
	QPushButton	*removeImage;

	QGroupBox	*selectedGroupBox;
	QListWidget	*selectedList;

	QGroupBox	*propertiesGroupBox;
	QLabel		*selectedItems;
	QLabel		*totalItems;

	QStringList	imageFiles;
};

#endif	// _WINDOW_HPP_INCLUDED
