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

#ifndef _WIDGET_VISIOR_QTableWidget_HPP_INCLUDED
#define _WIDGET_VISIOR_QTableWidget_HPP_INCLUDED
#include "WidgetVisitor.hpp"
#include <QTableWidget>

class WidgetVisitorState_QTableWidget
	:public WidgetVisitor::State
{
public:
	WidgetVisitorState_QTableWidget( QWidget* widget_);
	~WidgetVisitorState_QTableWidget();

	virtual bool enter( const QByteArray& name, bool writemode);
	virtual bool leave( bool writemode);
	virtual void clear();
	virtual QVariant property( const QByteArray& name);
	virtual bool setProperty( const QByteArray& name, const QVariant& data);
	virtual const QList<QByteArray>& dataelements() const;
	virtual bool isRepeatingDataElement( const QByteArray& name);
	virtual void setState( const QVariant& state);
	virtual QVariant getState() const;

private:
	void fill_cell( int row, int col, int itemidx);
	void set_thumbnail( int itemidx, const QVariant& data);
	QVariant get_thumbnail( int row, int col) const;

private:
	struct StackElement
	{
		int readpos;
		QTableWidgetItem* item;
		StackElement() :readpos(0),item(0){}
		StackElement( const StackElement& o) :readpos(o.readpos),item(o.item){}
		StackElement( QTableWidgetItem* item_) :readpos(0),item(item_){}
	};

	QTableWidget* m_tableWidget;
	QHash<QByteArray,int> m_colheaders;
	QHash<QByteArray,int> m_rowheaders;
	enum Mode {Init,Row,Column,RowData,ColumnData};
	static const char* modeName( Mode i)
	{
		static const char* ar[] = {"Init","Row","Column","RowData","ColumnData"};
		return ar[(int)i];
	}
	Mode m_mode;
	QList<QVariant> m_items;
	QList<QWidget*> m_cellwidgets;
	int m_row;
	int m_column;
	int m_rowcount;
	int m_columncount;
	QList<QByteArray> m_dataelements_init;
	QList<QByteArray> m_dataelements_row;
	QList<QByteArray> m_dataelements_col;
};

#endif