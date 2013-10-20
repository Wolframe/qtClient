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
#ifndef _WIDGET_DATATREE_HPP_INCLUDED
#define _WIDGET_DATATREE_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QSharedPointer>

#error DEPRECATED

///\class DataTree
///\brief Defines a tree of data items
class DataTree
{
public:
	enum ElementType {Invalid,Single,Array};
	static const char* elementTypeName( ElementType i)
	{
		static const char* ar[] = {"Invalid","Single","Array"};
		return ar[i];
	}

	explicit DataTree( ElementType elemtype_=Invalid)
		:m_elemtype(elemtype_),m_value_initialized(false),m_isOptional(false),m_isConstant(false),m_nofattributes(0){}
	DataTree( const DataTree& o)
		:m_elemtype(o.m_elemtype),m_value_initialized(o.m_value_initialized),m_isOptional(o.m_isOptional),m_isConstant(o.m_isConstant),m_value(o.m_value),m_defaultvalue(o.m_defaultvalue),m_nodear(o.m_nodear),m_nofattributes(o.m_nofattributes){}
	DataTree( const QVariant& value_)
		:m_elemtype(Single),m_value_initialized(true),m_isOptional(false),m_isConstant(true),m_value(value_),m_nofattributes(0){}

	void addNode( const QString& name_, const DataTree& tree_);
	void addAttribute( const QString& name_, const DataTree& tree_);
	void addArrayElement( const DataTree& tree_);
	bool setContent( const DataTree& elem_);

	void setNode( const QString& name_, const DataTree& tree_);
	const DataTree& node( const QString& name_) const;

	int findNodeIndex( const QString& name_) const;

	const QString nodename( int idx) const			{return m_nodear.at(idx).name;}
	const QSharedPointer<DataTree>& nodetree(int i) const	{return m_nodear.at(i).tree;}
	QSharedPointer<DataTree> nodetree( int i)		{return m_nodear.at(i).tree;}

	int size() const					{return m_nodear.size();}
	const QVariant& value() const				{return m_value;}
	const QVariant& defaultvalue() const			{return m_defaultvalue;}
	bool isOptional() const					{return m_isOptional;}
	bool isConstant() const					{return m_isConstant;}
	bool value_initialized() const				{return m_value_initialized;}
	ElementType elemtype() const				{return m_elemtype;}

	bool isValid() const					{return m_elemtype != Invalid;}
	bool isAttributeNode( int idx) const			{return (idx >= 0 && idx < m_nofattributes);}

	static DataTree fromString( const QString::const_iterator& begin, const QString::const_iterator& end);
	static DataTree fromString( const QString& str);
	static DataTree variableReference( const QVariant& value_, const QVariant& defaultvalue_, bool optional_);
	QString toString() const;

	void pushNodeValue( const QVariant& value);
	DataTree copyStructure() const;

private:
	static bool mapDataTreeToString( const DataTree& dt, QString& str);
	static bool mapDataValueToString( const QVariant& val, QString& str);

private:
	struct Node
	{
		Node( const QString& name_, const DataTree& tree_)
			:name(name_),tree( new DataTree( tree_)){}
		Node( const Node& o)
			:name(o.name),tree(o.tree){}
		Node(){}

		QString name;
		QSharedPointer<DataTree> tree;
	};

	ElementType m_elemtype;
	bool m_value_initialized;
	bool m_isOptional;
	bool m_isConstant;
	QVariant m_value;
	QVariant m_defaultvalue;
	QList<Node> m_nodear;
	int m_nofattributes;
};

class ActionDefinition
{
public:
	ActionDefinition( const QString& content);
	ActionDefinition( const ActionDefinition& o);
	QString toString() const;

	const QString& command() const			{return m_command;}
	const QString& doctype() const			{return m_doctype;}
	const QString& rootelement() const		{return m_rootelement;}
	const DataTree& structure() const		{return m_structure;}
	bool isValid() const				{return m_structure.isValid();}
	const QList<QString>& condProperties() const	{return m_condProperties;}

private:
	QList<QString> m_condProperties;
	QString m_command;
	QString m_doctype;
	QString m_rootelement;
	DataTree m_structure;
};

class ActionResultDefinition
{
public:
	ActionResultDefinition( const QString& content);
	ActionResultDefinition( const ActionResultDefinition& o);
	QString toString() const;

	const QString& doctype() const			{return m_doctype;}
	const QString& rootelement() const		{return m_rootelement;}
	const DataTree& structure() const		{return m_structure;}
	bool isValid() const				{return m_structure.isValid();}

private:
	QString m_doctype;
	QString m_rootelement;
	DataTree m_structure;
};

#endif
