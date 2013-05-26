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

#ifndef _WIDGET_VISIOR_QTreeWidget_HPP_INCLUDED
#define _WIDGET_VISIOR_QTreeWidget_HPP_INCLUDED
#include "WidgetVisitorObject.hpp"
#include <QTreeWidget>

class WidgetVisitorState_QTreeWidget
	:public WidgetVisitorObject
{
public:
	WidgetVisitorState_QTreeWidget( QWidget* widget_);

	virtual bool enter( const QString& name, bool writemode);
	virtual bool leave( bool writemode);
	virtual void clear();
	virtual QVariant property( const QString& name);
	virtual bool setProperty( const QString& name, const QVariant& data);
	virtual bool isArrayElement( const QString& name);
	virtual void setState( const QVariant& state);
	virtual QVariant getState() const;
	virtual void endofDataFeed();
	virtual void connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener);
	virtual void connectWidgetEnabler( WidgetEnabler& enabler);

private:
	void initSelected( const QVariant& selected);

	struct StackElement
	{
		int readpos;
		QTreeWidgetItem* item;
		StackElement() :readpos(0),item(0){}
		StackElement( const StackElement& o) :readpos(o.readpos),item(o.item){}
		StackElement( QTreeWidgetItem* item_) :readpos(0),item(item_){}
	};

	QTreeWidget* m_treeWidget;
	QStack<StackElement> m_stk;
	QList<QString> m_headers;
	QString m_elementname;
	enum Mode {Init,Tree,List};
	static const char* modeName( Mode i)
	{
		static const char* ar[] = {"Init","Tree","List"};
		return ar[(int)i];
	}
	Mode m_mode;
};

#endif
