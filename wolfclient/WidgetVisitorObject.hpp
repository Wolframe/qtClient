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

#ifndef _WIDGET_VISIOR_OBJECT_HPP_INCLUDED
#define _WIDGET_VISIOR_OBJECT_HPP_INCLUDED
#include <QWidget>
#include <QStack>
#include <QString>
#include <QSharedPointer>
#include <QHash>
#include <QVariant>
#include <QDebug>
#include <cstring>

///\class WidgetEnabler
///\brief Structure to enable widgets based on a set of properties
class WidgetEnabler :public QObject
{
	Q_OBJECT

public:
	///\brief Constructor
	virtual ~WidgetEnabler(){}
	virtual void handle_changed()=0;

public slots:
	void changed()	{handle_changed();}
};


///\class WidgetListener
///\brief Structure to redirect Qt widget signals to widget visitor events
class WidgetListener :public QObject
{
	Q_OBJECT
public:
	///\enum DataSignalType
	///\brief Data signal type
	enum DataSignalType
	{
		SigChanged,
		SigActivated,
		SigEntered,
		SigPressed,
		SigClicked,
		SigDoubleClicked,
		SigDestroyed
	};

	enum {NofDataSignalTypes=(int)SigDestroyed+1};
	static const char* dataSignalTypeName( DataSignalType ii)
	{
		static const char* ar[]
		= {"changed", "activated", "entered", "pressed", "clicked", "doubleclicked", "destroyed", 0};
		return ar[(int)ii];
	}
	static bool getDataSignalType( const char* name, DataSignalType& dt)
	{
		const char* signame;
		for (int ii=0; 0!=(signame=dataSignalTypeName((DataSignalType)ii)); ++ii)
		{
			if (std::strcmp( name, signame) == 0)
			{
				dt = (DataSignalType)ii;
				return true;
			}
		}
		return false;
	}

public:
	///\brief Constructor
	virtual ~WidgetListener(){};

	virtual void handleDataSignal( DataSignalType dt)=0;
	virtual void handleShowContextMenu( const QPoint& pos)=0;

public slots:
	void changed()					{handleDataSignal( SigChanged);}
	void activated()				{handleDataSignal( SigActivated);}
	void entered()					{handleDataSignal( SigEntered);}
	void pressed()					{handleDataSignal( SigPressed);}
	void clicked()					{handleDataSignal( SigClicked);}
	void doubleclicked()				{handleDataSignal( SigDoubleClicked);}
	void destroyed()				{handleDataSignal( SigDestroyed);}
	void showContextMenu( const QPoint& pos)	{handleShowContextMenu( pos);}
};


///\class State
///\brief Accessor for a widget (implemented for every widget type supported)
struct WidgetVisitorObject
{
public:
	///\brief Constructor
	explicit WidgetVisitorObject( QWidget* widget_)
		:m_widget(widget_){}

	///\brief Destructor
	virtual ~WidgetVisitorObject(){}

	QWidget* widget() const	{return m_widget;}

public://Interface methods implemented for different widget types:
	///\brief Clear contents of the widget
	virtual void clear(){}
	///\brief Switch to a substructure context inside the widget but not to a child widget
	virtual bool enter( const QString& /*name*/, bool /*writemode*/)	{return false;}
	///\brief Leave the current substructure context an switch to ist (enter) ancessor
	virtual bool leave( bool /*writemode*/)					{return false;}
	///\brief Get a property or set of properties of the current substructure context addressed by name
	virtual QVariant property( const QString&)=0;
	///\brief Set a property of the current substructure context addressed by name
	virtual bool setProperty( const QString&, const QVariant&)=0;
	///\brief Restore the widget state from a variable
	virtual void setState( const QVariant& /*state*/){}
	///\brief Get the current the widget state
	virtual QVariant getState() const					{return QVariant();}
	///\brief Hook to complete the feeding of data
	virtual void endofDataFeed(){}
	///\brief Check if a an element can appear more than once
	virtual bool isArrayElement( const QString&/*name*/) const		{return false;}
	///\brief Check if a an element has is draggable
	virtual bool hasDrag() const						{return false;}

	///\brief Connect all widget signals that should trigger an event on a signal of type 'type'
	virtual void connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& /*listener*/)
	{
		qCritical() << "try to connect to signal not provided" << m_widget->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
	}
	///\brief Connect widget signals that should trigger an event for enabling or disabling a data referencing widget
	virtual void connectWidgetEnabler( WidgetEnabler& /*enabler*/){}

private:
	QWidget* m_widget;
};
typedef QSharedPointer<WidgetVisitorObject> WidgetVisitorObjectR;

#endif


