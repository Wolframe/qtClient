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

class WImageListWidget;


class PluginWindow : public QWidget
{
	Q_OBJECT

public:
	PluginWindow();

private slots:
	void addImagesFromFiles();
	void removeSelectedImages();
	void selectorClicked();
	void itemSelected();

private:
	void createPluginGroupBox();
	void createOperationsGroupBox();
	void createSelectedGroupBox();
	void createPropertiesGroupBox();

	QGroupBox		*m_pluginGroupBox;
	QGridLayout		*m_pluginLayout;
	WImageListWidget	*m_selector;

	QGroupBox		*m_operationsGroupBox;
	QPushButton		*m_addImage;
	QPushButton		*m_removeImage;

	QGroupBox		*m_selectedGroupBox;
	QListWidget		*m_selectedList;

	QGroupBox		*m_propertiesGroupBox;
	QLabel			*m_selectedItems;
	QLabel			*m_totalItems;
};

#endif	// _WINDOW_HPP_INCLUDED
