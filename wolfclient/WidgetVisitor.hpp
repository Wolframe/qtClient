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

#ifndef _WOLFRAME_WIDGET_VISIOR_HPP_INCLUDED
#define _WOLFRAME_WIDGET_VISIOR_HPP_INCLUDED
#include "serialize/DataSerializeItem.hpp"
#include "serialize/VisitorInterface.hpp"
#include "WidgetVisitorObject.hpp"
#include <QWidget>
#include <QStack>
#include <QHash>
#include <QString>
#include <QSharedPointer>
#include <QVariant>

/** Implementation Documentation:

1 Introduction
The WidgetVisitor implements an Iterator on a widget tree. It can access
internal Widget properties (setter and getter methods of the
WidgetVisitorObject interface) or dynamic properties of the widget.
Additionally there are mechanisms to address widgets in the context by
a path [Widget Path].

2 Widget Path
A widget path is a list of identifiers separated by dot '.'.
The identifiers of a path are either reserved identifiers ('main'
for the current window root widget), symbolic links to other
widgets declared as dynamic properties (link:<name> := <widgetid>)
or name references to immediate children widgets.

2.1. Widget Path Resolving
The widget visitors tree iterator is implemented as stack of widget
visitor object references (a child or parent widget or a widget
referenced by a symbolic link).
Internal paths are managed by the WidgetVisitorObject implementation.

2.1.2 Widget Path Resolving Example
For resolving a property
         'customertable.customer.address.name'
where address refers to a customer subwidget and name defines a symbolic link, we
have the following elements on the widget visitor stack (a very artificial example :-):

    [customertable 1]   (1 is the internal state of customertable after selection of customer)
    [address 0]
    [name 0]

We get to this state calling:
    WidgetVisitor visitor( <customertable widget>* )
    visitor.enter( "customer");
    visitor.enter( "address");
or
    WidgetVisitor visitor( <customertable widget>* )
    visitor.enter( "customer.address");

We can access the property name with:
    QVariant name = visitor.property( "name");

If we are just interested in the property only, we can also call instead:
    QVariant name = WidgetVisitor( <customertable widget>* ).property( "customer.address.name");

Internally happens the same.


2.2. Widget Path Resolving Algorithm

2.2.1 Property Path Resolving (property,setProperty)
 [A] Check if an multipart property is referenced and try to step into the substructures to return the property if yes (see "2.2")
 [B] Check if an internal property of the widget is referenced return it if yes
 [C] Check if a dynamic property is referenced and return it if yes

2.2.2 Widget Path Resolving (enter,leave)
 [A] Check if an multipart property is referenced and try to step into the substructures to return the property if yes (see "2.2")
 [B] Check if name refers to a widget internal item and follow it if yes
 [C] Check if name refers to a symbolic link and follow the link if yes
 [D] On top level check if name refers to an ancessor or an ancessor child and follow it if yes
 [E] Check if name refers to a child and follow it if yes


3. Dynamic Properties Used

 'contextmenu'           Defines a context menu with a comma separated list of identifiers of actions defined as propery value. Two following commas without menu entry identifier are used to define a separator (QMenu::addSeparator())
 'contextmenu:NAME'      Defines the (translatable) text of a context menu entry. NAME refers to a non empty name in the list of context menu entries

 'action'                Defines an action. This can either be
                         a) load action request for a widget that is not a push button
                         b) action without answer than OK/ERROR for a push button
 'action:IDENTIFIER'     Defines an action either
                         a) related to a context menu entry (when clicked) or
                         b) related to a dataslot declaration of this widget named with IDENTIFIER.

 'form'                  Defines a form to be opened on click (push button).
                         If the widget has an action defined, then the action is executed before and the form is opened when the action succeeds and not opened when it fails.
 'form:IDENTIFIER'       Defines a form related to a context menu entry (when clicked)
			 If the context menu entry has also an action defined, then the action is executed before and the form is opened when the action succeeds and not opened when it fails.

 'answer'                Defines the format of a validated load action answer

 'global:IDENTIFIER'     Defines an assignment from a global variable IDENTIFIER at initialization and writing the global variable when closing the widget (WidgetVisitor::readGlobals()/writeGlobals())
 'assign:PROP'           Defines an assingment of property PROP to the property defined as value "assign:PROP" on data load and refresh (WidgetVisitor::readAssignments()/writeAssignments())
 'link:'                 Defines a symbolic link to another widget.
                         Defining the property "link:<name>" = <widgetid>: defines <name> to be a reference to the widget with the widgetid set to <widgetid>.
                         Used to read data from other widgets. The widgetid can be passed as parameter for children to reference or defined hardcoded for singleton widgets.

 'datasignal:IDENTIFIER' Defines a signal of type IDENTIFIER (clicked,doubleclicked,destroyed,signaled,...) with the slot name and destination address defined as property value of "datasignal:IDENTIFIER"
                         Datasignal destinations can be defined as follows
                         a) As widgetid (Attention HACK: If something is a widgetid is decided by searching for a ':' !!! Do not use ':' in slot identifiers.
                         b) As slot identifier (declared with 'dataslot')
                         c) widget path
                         For a) and and c) a preceding identifier followed by '@' specifies the name of the identifier of the target slot.

 'state:IDENFITIER'      Defines a conditional state of the widget. IDENTIFIER is one of 'enabled','disabled','hidden','visible' the property value defines the condition that defines the state defined with IDENTIFIER.
                         A condition can either be
                         a) A property reference in '{' '}' brackets. The condition is met when the property referenced is valid
                         b) An expression of the form <prop> <op> <value>, where <prop> is a property reference in '{' '}' brackets, <op> an operator and <value> a constant value
                            Valid operators are: '==' (string),'!=' (string),'<=' (integer), '<' (integer) ,'>=' (integer), '>' (integer)
                         Remark: The owner widget of the properties referenced must exist when the widget is initialized. Otherwise the trigger for state change will not be installed !!!
                         Remark: For 'action' definitions there are conditional state triggers ('state:enabled') installed automatically for all referenced action properties without default value
 'dataslot'              Defines a comma separated list of slots for the signal of with the property value as slot identifer and optionally a sender widget id in '(..)' brackets.
 'widgetid'              Unique identifier of the widget used for identifying it (resolving symbolic links, address of a request aswer)
 'synonym:NAME'          Defines a mapping from the identifier NAME to the property value. The map is used to describe a renaming of an answer tag of an implicitely mapped request answer (when no 'answer' is defined. used only for recursive structures)


3.1 Reserved Internal Dynamic Properties of Widgets
The '_w_' prefix is used for internal widget properties not of interest for the user.
 '_w_state'              Internal WidgetVistitorObject state representation
 '_w_formstack'          Stack of parent form calls to be opened with _CLOSE_ resp. _RESET_
 '_w_statestack'         Stack of parent form states to be refreshed with _CLOSE_ resp. _RESET_ (array parallel to '_w_formstack')
 '_w_selected'           Used by widget visitor objects to store the selection set by 'selected' to establish independency of load data and selection (the selection can be made before loading the data, for example passed by form parameters)
 '_w_rowvalue'           Used by the table widget visitor to the table row ids for selection in case of a row oriended table definition
 '_w_columnvalue'        Used by the table widget visitor to the table columns ids for selection in case of a column oriended table definition
 '_w_origtext'           Original text of the label before '%1','%2',... substitutions
 '_w_pictures'           List of pictures for the case of putting many pictures in one label (not implemented yet)
 '_w_id'                 Used to store an identifier for later use


4. Order of Events and Actions

4.1. Form Load
    (1) set form parameters  (URL syntax of form call)
    (2) read globals         ('global:VAR')
    (3) read assignments     ('assign:VAR')
    (4) initiate form localization load
    (5) connect event signal/slots
    (6) issue all domain load requests in reverse order of depht (children widgets first)

4.2. Widget Request Answer
    (1) find recipient widget
    (2) save widget state
    (3) clear widget data
    (4) read globals
    (5) read assignments
    (6) initialize with delivered answer
    (7) restore widget state

4.3. Widget Request Error
    (1) show error
    (2) emit data load error if defined

4.3. Refresh With Domain Load Request
    (1) emit domain load request if defined

4.3. Refresh Without Domain Load Request
    (1) save widget state
    (2) clear widget data
    (3) read globals
    (4) read assignments
    (5) restore widget state

Remark:
    Subsequent data requests with same tag are deleted if not sent yet to
        the server (only the last one is sent)

**/

///\class WidgetListenerImpl
///\brief Forward declaration
class WidgetListenerImpl;
///\class WidgetEnablerImpl
///\brief Forward declaration
class WidgetEnablerImpl;
///\class DataLoader
///\brief Forward declaration
class DataLoader;
///\class FormWidget
///\brief Forward declaration
class FormWidget;

///\class WidgetVisitor
///\brief Tree to access to (read/write) of widget data
class WidgetVisitor
	:public VisitorInterface
{
	public:
		///\brief Default constructor
		WidgetVisitor(){}

		///\enum Flags
		///\brief Flags stearing behaviour
		enum VisitorFlags {None=0x0,BlockSignals=0x1,AllowUndefDynPropsInit=0x2};

		///\brief Constructor
		///\param[in] root Root of widget tree visited
		///\param[in] flags flags stearing visitor behaviour
		explicit WidgetVisitor( QWidget* root, VisitorFlags flags_=WidgetVisitor::None);

		///\brief Copy constructor
		///\param[in] o object to copy
		WidgetVisitor( const WidgetVisitor& o)
			:m_stk(o.m_stk)
			,m_blockSignals(o.m_blockSignals)
			,m_allowUndefDynPropsInit(o.m_allowUndefDynPropsInit){}

		///\brief Constructor by object
		explicit WidgetVisitor( const WidgetVisitorObjectR& obj, VisitorFlags flags);

		///\brief Destructor
		virtual ~WidgetVisitor(){}

		VisitorFlags flags() const
		{
			return (VisitorFlags)((int)(m_blockSignals?BlockSignals:None)|(int)(m_allowUndefDynPropsInit?AllowUndefDynPropsInit:None));
		}

		static void init_widgetids( QWidget* widget);

		///\brief Sets the current node to the child with name 'name'
		virtual bool enter( const QString& name, bool writemode);
		///\brief Sets the current node to the root widget or child with name 'name' of the root widget
		bool enter_root( const QString& name);

		///\brief Set the current node to the parent that called enter to this node.
		virtual void leave( bool writemode);

		///\brief Get the property of the current node by 'name'
		///\param[in] name name of the property
		///\return property variant (any type)
		virtual QVariant property( const QString& name);
		///\brief Get the property of the current node by 'name'
		///\param[in] name name of the property
		///\return property variant (any type)
		QVariant property( const char* name);

		///\brief Set the property of the current node
		///\param[in] name name of the property
		///\param[in] value property value as variant (any type)
		///\return true on success
		virtual bool setProperty( const QString& name, const QVariant& value);
		///\brief Set the property of the current node
		///\param[in] name name of the property
		///\param[in] value property value as variant (any type)
		///\return true on success
		bool setProperty( const char* name, const QVariant& value);

		QWidget* getPropertyOwnerWidget( const QString& name) const;
		void connectWidgetEnabler( WidgetEnabler& enabler);

		///\brief Find all sub widgets matching a condition and return them as their visitor context
		typedef bool (*NodeProperty)( const QWidget* widget_, const QVariant& cond);
		QList<QWidget*> findSubNodes( NodeProperty prop, const QVariant& cond=QVariant()) const;

		QList<QWidget*> children( const QString& name=QString()) const;

		///\brief Eval if id describes a widget reference from the current widget
		///\param[in] id identifier to check
		///\return the widget reference if it exists
		QWidget* get_widget_reference( const QString& id);

		///\brief Get the unique identifier of the widget for server requests
		QString widgetid() const;
		///\brief Get the widget of the current state
		virtual QWidget* widget() const							{return m_stk.isEmpty()?0:m_stk.top().m_obj->widget();}
		///\brief Get the object name of the widget of the current state
		QString objectName() const;
		///\brief Get the class name of the widget of the current state
		QString className() const;

		///\brief Resolve variable references in value
		///\param[in] value value to resolve
		QVariant resolve( const QVariant& value);

		///\brief Resolve condition
		///\param[in] expr conditional expression to resolve
		bool evalCondition( const QVariant& expr);

		///\brief Get the form widget of this widget
		///\brief Return the form widget reference if this widget is a sub widget of a form or 0 if it does not exist
		FormWidget* formwidget() const;
		///\brief Get the UI root widget
		QWidget* uirootwidget() const;
		///\brief Get the UI root widget
		QWidget* predecessor( const QString& name) const;
		///\brief Get the complete path of the widget for error messages
		QString widgetPath() const;
		static QString widgetPath( const QWidget* widget);

		///\brief Resolve a symbolic link to a widget
		///\param[in] link name of symbolic link to resolve
		QWidget* resolveLink( const QString& link);

		///\brief Find form widget by widgetid
		///\param[in] widget id to search for
		FormWidget* findFormWidgetWithWidgetid( const QString& wid);

		///\brief Get the current widget state
		QVariant getState();
		///\brief Set the current widget state
		void setState( const QVariant& state);
		///\brief Declare end of data feed
		virtual void endofDataFeed();

		///\brief Clear widget data
		virtual void clear();

		///\brief Get all receivers of a datasignal (type)
		QList<QPair<QString,QWidget*> > get_datasignal_receivers( WidgetListener::DataSignalType type);
		///\brief Get all datasignal receivers specified by id or dataslot definition
		QList<QPair<QString,QWidget*> > get_datasignal_receivers( const QString& receiverid);

		///\brief Execute dynamic properties declared as 'assign:var' := 'value' as assingments "var = <value>"
		void readAssignments();
		///\brief Execute dynamic properties declared as 'assign:var' := 'value' as assingments "value = <var>"
		void writeAssignments();

		///\brief For all visitor sub widgets do assignments to form widgets based on assign: declarations
		void do_readAssignments();
		///\brief For all visitor sub widgets do assignments from form widgets based on assign: declarations
		void do_writeAssignments();

		///\brief Do assignments from global variables to form widgets based on global: declarations
		void readGlobals( const QHash<QString,QVariant>& globals);
		///\brief Do assignments to global variables from form widgets based on global: declarations
		void writeGlobals( QHash<QString,QVariant>& globals);

		///\brief For all visitor sub widgets do assignments from global variables to form widgets based on global: declarations
		void do_readGlobals( const QHash<QString,QVariant>& globals);
		///\brief For all visitor sub widgets do assignments to global variables from form widgets based on global: declarations
		void do_writeGlobals( QHash<QString,QVariant>& globals);

		///\brief Create listener object for the widget and wire all data signals
		WidgetListenerImpl* createListener( DataLoader* dataLoader);

		///\brief sets the AllowUndefDynPropsInit flag to value and return the previous value of it
		bool allowUndefDynPropsInit( bool value);

	private:
		///\brief Internal property get using 'level' to check property resolving step (B).
		///\param[in] name name of the property
		///\param[in] level element index in path (element is first element of a path <=> level == 0)
		QVariant property( const QString& name, int level);

		///\brief Internal property set using 'level' to check property resolving step (B).
		///\param[in] name name of the property
		///\param[in] value property value as variant (any type)
		///\param[in] level element index in path (element is first element of a path <=> level == 0)
		///\return true on success
		bool setProperty( const QString& name, const QVariant& value, int level);

		QWidget* getPropertyOwnerWidget( const QString& name, int level);

		///\brief Sets the current node to the child with name 'name'
		///\param[in] name name of the subnode
		///\param[in] writemode true if in write/create access and not in read access
		///\param[in] level element index in path (element is first element of a path <=> level == 0)
		bool enter( const QString& name, bool writemode, int level);

		void ERROR( const char* msg, const QString& arg) const;
		void ERROR( const char* msg) const;

	private:
		class State
		{
		public:
			~State();
			State();
			State( const State& o);
			State( WidgetVisitorObjectR obj_, bool blockSignals_);

		public://Common methods:
			QString getLink( const QString& name) const;
			QVariant dynamicProperty( const QString& name) const;
			bool setDynamicProperty( const QString&, const QVariant& value, bool reportUndefined);

		private:
			struct DataSignals
			{
				QList<QString> id[(int)WidgetListener::NofDataSignalTypes];
				DataSignals(){}
				DataSignals( const DataSignals& o)
				{
					for (int ii=0; ii<(int)WidgetListener::NofDataSignalTypes; ++ii)
					{
						id[ii] = o.id[ii];
					}
				}
			};
			friend class WidgetVisitorStackElement;
			friend class WidgetVisitor;

			typedef QPair< QString,QString> LinkDef;
			typedef QPair< QString,QString> Assignment;

			WidgetVisitorObjectR m_obj;
			QList<LinkDef> m_links;				//< symbolic links to other objects
			QList<Assignment> m_assignments;		//< assignment done at initialization and destruction
			QList<Assignment> m_globals;			//< assignment done at initialization and destruction
			DataSignals m_datasignals;			//< datasignals to emit on certain state changes
			QList<QString> m_dataslots;			//< dataslot to declare a receiver by name for being informed on certain state changes
			QHash<QString,QVariant> m_dynamicProperties;	//< map of defined dynamic properties
			int m_multipart_entercnt;			//< counter for how many stack elements to pop on a leave (for multipart elements)
			int m_internal_entercnt;			//< counter for calling State::leave() before removing stack elements
			bool m_blockSignals;
			bool m_blockSignals_bak;
		};

	private:
		QStack<State> m_stk;		//< stack of visited widgets. The current node is the top element
		bool m_blockSignals;		//< wheter to block signals or not
		bool m_allowUndefDynPropsInit;	//< true, if flag AllowUndefDynPropsInit is set: error log is suppressed for setting a not predefined dynamic properties
};

#endif

