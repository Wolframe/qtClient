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

///\class WidgetListener
///\brief Forward declaration
class WidgetListener;
///\class WidgetEnabler
///\brief Forward declaration
class WidgetEnabler;
///\class DataLoader
///\brief Forward declaration
class DataLoader;

///\class State
///\brief Accessor for a widget (implemented for every widget type supported)
struct WidgetVisitorObject
{
	///\enum DataSignalType
	///\brief Data signal type
	enum DataSignalType
	{
		SigChanged,
		SigActivated,
		SigEntered,
		SigPressed,
		SigClicked,
		SigDoubleClicked
	};
	enum {NofDataSignalTypes=(int)SigDoubleClicked+1};
	static const char* dataSignalTypeName( DataSignalType ii)
	{
		static const char* ar[]
		= {"changed", "activated", "entered", "pressed", "clicked", "doubleclicked", 0};
		return ar[(int)ii];
	}
	static bool getDataSignalType( const char* name, DataSignalType& dt);

	///\brief Constructor
	explicit WidgetVisitorObject( QWidget* widget_);

	///\brief Copy constructor
	WidgetVisitorObject( const WidgetVisitorObject& o);

	///\brief Destructor
	virtual ~WidgetVisitorObject(){}

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
	virtual bool isArrayElement( const QString&/*name*/)			{return false;}

	///\brief Create listener object for the widget
	virtual WidgetListener* createListener( DataLoader* dataLoader);
	///\brief Connect all widget signals that should trigger an event on a signal of type 'type'
	virtual void connectDataSignals( DataSignalType dt, WidgetListener& listener);
	///\brief Connect widget signals that should trigger an event for enabling or disabling a data referencing widget
	virtual void connectWidgetEnabler( WidgetEnabler& /*enabler*/){}

public://Common methods:
	QVariant getSynonym( const QString& name) const;
	QString getLink( const QString& name) const;
	QVariant dynamicProperty( const QString& name) const;
	bool setDynamicProperty( const QString&, const QVariant& value);
	QWidget* widget() const		{return m_widget;}

private:
	struct DataSignals
	{
		QList<QString> id[(int)NofDataSignalTypes];
	};
	friend class WidgetVisitorStackElement;
	friend class WidgetVisitor;

	typedef QPair< QString,QString> LinkDef;
	typedef QPair< QString,QString> Assignment;

	QWidget* m_widget;				//< widget reference
	QHash<QString,QString> m_synonyms;		//< synonym name map
	QList<LinkDef> m_links;				//< symbolic links to other objects
	QList<Assignment> m_assignments;		//< assignment done at initialization and destruction
	QList<Assignment> m_globals;			//< assignment done at initialization and destruction
	DataSignals m_datasignals;			//< datasignals to emit on certain state changes
	QList<QString> m_dataslots;			//< dataslot to declare a receiver by name for being informed on certain state changes
	QHash<QString,QVariant> m_dynamicProperties;	//< map of defined dynamic properties
	int m_synonym_entercnt;				//< counter for how many stack elements to pop on a leave (for multipart synonyms)
	int m_internal_entercnt;			//< counter for calling State::leave() before removing stack elements
};
typedef QSharedPointer<WidgetVisitorObject> WidgetVisitorObjectR;

#endif


