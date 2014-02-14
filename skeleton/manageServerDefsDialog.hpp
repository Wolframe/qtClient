/************************************************************************

 Copyright (C) 2011 - 2014 Project Wolframe.
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

#ifndef _MANAGE_SERVER_DEFS_DIALOG_HPP_INCLUDED
#define _MANAGE_SERVER_DEFS_DIALOG_HPP_INCLUDED

#include <QDialog>
#include <QAbstractListModel>

#include "serverDefinition.hpp"

namespace Ui {
class ManageServerDefsDialog;
}

class SKELETON_VISIBILITY ManageServerDefsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ManageServerDefsDialog( QVector< ServerDefinition >& params,
					 QString& defaultServer, QWidget *parent = 0 );
	~ManageServerDefsDialog();

	// override exec()
	int exec();

private slots:
	void done( int retCode );

	void newServerDefinition();
	void editServerDefinition();
	void delServerDefinition();
	void setDefaultServer();

	void updateUIstate();

private:
	class DefsListModel : public QAbstractListModel	{
	public:
		DefsListModel( QVector< ServerDefinition >& defsVector, QString& defEntry )
			: m_defsVector( defsVector ), m_defEntry( defEntry )	{}
		int rowCount( const QModelIndex& /*parent*/ ) const	{ return m_defsVector.count(); }
		QVariant data ( const QModelIndex& index, int role ) const;
		QVariant headerData ( int /*section*/, Qt::Orientation /*orientation*/, int /*role*/ ) const
									{ return QVariant(); }
		void rowChanged( const int row );
		bool appendServerDefinition( const ServerDefinition& def );
		bool removeServerDefinition( int position );
	private:
		QVector< ServerDefinition >&	m_defsVector;
		QString&			m_defEntry;
	};

private:
	Ui::ManageServerDefsDialog	*ui;
	QVector< ServerDefinition >	m_localParams;
	QString				m_localDefault;
	QVector< ServerDefinition >&	m_globalParams;
	QString&			m_globalDefault;
	DefsListModel			m_defsListModel;
};

#endif // _MANAGE_SERVER_DEFS_DIALOG_HPP_INCLUDED
