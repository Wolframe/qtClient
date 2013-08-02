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
#ifndef _WIDGET_DATA_STRUCT_DESCRIPTION_HPP_INCLUDED
#define _WIDGET_DATA_STRUCT_DESCRIPTION_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QSharedPointer>

///\brief Forward declaration for DataStructDescription
class DataStruct;

///\class DataStructDescription
///\brief Description of a data structure as array of attributes and content elements
class DataStructDescription
{
public:
	///\enum Type
	///\brief Structure element type
	enum Type
	{
		atomic_,
		variableref_,
		struct_,
		indirection_
	};
	///\brief Constructor
	DataStructDescription()
		:m_nofattributes(0){}
	///\brief Copy constructor
	DataStructDescription( const DataStructDescription& o)
		:m_nofattributes(o.m_nofattributes)
		,m_ar(o.m_ar)
	{
		setIndirectionDescription( &o);
	}

	///\brief Destructor
	~DataStructDescription(){}

	///\brief Change pointers to indirection descriptors from 'o' to 'this'
	void setIndirectionDescription( const DataStructDescription* o);

	///\brief One element of the structure description. Refers to the element with the same index in the corresponding DataStruct
	struct Element
	{
		Element(){}
		Element( const QString& name_, const QVariant& initvalue_);
		Element( const QString& name_, const QString& varname, const QVariant& defaultvalue);
		Element( const QString& name_, const DataStructDescription* substruct_, bool pointer_=false);
		Element( const Element& o);
		~Element();

		Type type;
		QString name;				//< name of the element in UTF-8
		QString variableref;			//< variable reference
		DataStruct* initvalue;			//< initialization value of the element
		const DataStructDescription* substruct;	//< substructure in case of an element that is itself a structure

		///\brief Flags describing some properties of the element
		enum Flags
		{
			NoFlags=0x0,		//< no flags set
			Optional=0x1,		//< element is an optional
			Mandatory=0x2,		//< element is an mandatory
			Attribute=0x4,		//< element is an attribute
			Array=0x8,		//< element is an array
			AnyValue=0x10		//< element is undefined
		};
		unsigned char flags;		//< internal representation of the flags of this element

		bool optional() const						{return (flags & (unsigned char)Optional) != 0;}
		bool mandatory() const						{return (flags & (unsigned char)Mandatory) != 0;}
		bool attribute() const						{return (flags & (unsigned char)Attribute) != 0;}
		bool array() const						{return (flags & (unsigned char)Array) != 0;}
		bool anyValue() const						{return (flags & (unsigned char)AnyValue) != 0;}

		void setOptional( bool v=true)					{if (v) flags |= (unsigned char)Optional;  else flags -= (flags & (unsigned char)Optional);}
		void setMandatory( bool v=true)					{if (v) flags |= (unsigned char)Mandatory; else flags -= (flags & (unsigned char)Mandatory);}
		void setAttribute( bool v=true)					{if (v) flags |= (unsigned char)Attribute; else flags -= (flags & (unsigned char)Attribute);}
		void setAnyValue( bool v=true)					{if (v) flags |= (unsigned char)AnyValue; else flags -= (flags & (unsigned char)AnyValue);}

		bool makeArray();
		QString variablename() const;					//< return the name of the variable referenced in case of a variableref_
	};

	///\brief Const iterator on the elements of the definition
	typedef QList<Element>::const_iterator const_iterator;
	///\brief Iterator on the elements of the definition
	typedef QList<Element>::iterator iterator;

public:
	///\brief Random access or 0 if no random access defined (throws logic error on ABR/ABW)
	const Element& at( int idx) const					{return m_ar.at( idx);}
	Element& at( int idx)							{return m_ar[idx];}

	///\brief Get the last element (throws logic error on ABR/ABW)
	const Element& back() const						{return m_ar.back();}
	Element& back()								{return m_ar.back();}

	///\brief Get the an iterator on the first element (direct child)
	const_iterator begin() const						{return m_ar.begin();}
	iterator begin()							{return m_ar.begin();}
	///\brief Get the an iterator on the end of the list of elements
	const_iterator end() const						{return m_ar.end();}
	iterator end()								{return m_ar.end();}

	///\brief Add an variable attribute definition to the structure description
	int addAttributeVariable( const QString& name_, const QString& variblename, const QVariant& defaultvalue);
	///\brief Add an attribute definition to the structure description
	int addAttribute( const QString& name_, const QVariant& initvalue_);
	///\brief Add an atomic element definition to the structure description
	int addAtomVariable( const QString& name_, const QString& variblename, const QVariant& defaultvalue);
	///\brief Add an atomic element definition to the structure description
	int addAtom( const QString& name_, const QVariant& initvalue_);
	///\brief Add a substructure definition to the structure description
	int addStructure( const QString& name_, const DataStructDescription& substruct_);
	///\brief Add an indirection definition to the structure description (an indirection is a element expanded on access, e.g. for defining recursive structures)
	int addIndirection( const QString& name_, const DataStructDescription* descr);
	///\brief Add a reference to a variable
	int addElement( const Element& elem);
	///\brief Inherit the elements from another structure description
	void inherit( const DataStructDescription& parent);

	///\brief Find an element by name in the structure description
	int findidx( const QString& name) const;
	const_iterator find( const QString& name) const;
	iterator find( const QString& name);

	///\brief Get the number of elements in the structure description
	int size() const							{return m_ar.size();}

	///\brief Compare two structure descriptions element by element (recursively)
	int compare( const DataStructDescription& o) const;
	///\brief Get the list of names as string with 'sep' as separator for logging
	QString names( const QString& sep) const;

	///\brief Print the contents of a structure description (structures in curly brackets as in the simpleform language)
	void print( QString& out, const QString& indent, const QString& newitem, int level) const;

	///\brief Return the contents of a structure description as string (format as in print with no indent and newlines)
	QString tostring() const;

	///\brief Parses this data struct description
	//... imlemented in DataStructDescriptionParse.cpp
	bool parse( const QString& source, QList<QString>& err);

	///\brief Gets an initialized instance of this structure
	DataStruct* createDataInstance() const;

private:
	int m_nofattributes;
	QList<Element> m_ar;
};

typedef QSharedPointer<DataStructDescription> DataStructDescriptionR;

#endif


