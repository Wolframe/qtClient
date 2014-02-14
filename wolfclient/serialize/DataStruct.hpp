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
#ifndef _WOLFRAME_STDCLIENT_DATA_STRUCT_HPP_INCLUDED
#define _WOLFRAME_STDCLIENT_DATA_STRUCT_HPP_INCLUDED
#include <QString>
#include <QVariant>
#include <QPair>
#include <QList>
#include <QSharedPointer>

///\brief Forward declaration for DataStruct
class DataStructDescription;

///\class DataStruct
///\brief Data structure to represent the data unit processed by Wolframe
class DataStruct
{
public:
	DataStruct();
	DataStruct( const DataStructDescription* description_);
	DataStruct( const QVariant& value);
	DataStruct( const DataStruct& o);

	DataStruct& operator = ( const DataStruct& o);
	~DataStruct();

	const QVariant& value() const;
	bool setValue( const QVariant&);

	bool atomic() const;
	bool array() const;
	bool indirection() const;

	int compare( const DataStruct& o);

	bool push();

	const DataStruct* back() const;
	DataStruct* back();

	const DataStruct* at( int idx) const;
	DataStruct* at( int idx);

	const DataStruct* get( const QString& name) const;
	DataStruct* get( const QString& name);

	const DataStruct* prototype() const;
	const DataStructDescription* description() const	{return m_description;}

	bool initialized() const				{return m_initialized;}
	void setInitialized( bool v=true)			{m_initialized = v;}

	void expandIndirection( bool arrayind);
	bool check() const;

public:
	class const_iterator
	{
	public:
		const_iterator( DataStruct const* ptr_)
			:m_ptr(ptr_){}
		const_iterator& operator++()			{++m_ptr; return *this;}
		const_iterator operator++(int)			{const_iterator rt(m_ptr); ++m_ptr; return rt;}

		const DataStruct& operator*() const		{return *m_ptr;}
		DataStruct const* operator->() const		{return m_ptr;}

		bool operator==( const const_iterator& o) const	{return m_ptr==o.m_ptr;}
		bool operator!=( const const_iterator& o) const	{return m_ptr!=o.m_ptr;}
		bool operator>=( const const_iterator& o) const	{return m_ptr>=o.m_ptr;}
		bool operator>( const const_iterator& o) const	{return m_ptr>o.m_ptr;}
		bool operator<=( const const_iterator& o) const	{return m_ptr<=o.m_ptr;}
		bool operator<( const const_iterator& o) const	{return m_ptr<o.m_ptr;}

		const_iterator operator+( int idx) const	{return const_iterator( m_ptr+idx);}
		const_iterator operator-( int idx) const	{return const_iterator( m_ptr-idx);}
		int operator-( const const_iterator& o) const	{return m_ptr-o.m_ptr;}

	private:
		DataStruct const* m_ptr;
	};

	class iterator
	{
	public:
		iterator( DataStruct* ptr_)
			:m_ptr(ptr_){}
		iterator& operator++()				{++m_ptr; return *this;}
		iterator operator++(int)			{iterator rt(m_ptr); ++m_ptr; return rt;}

		DataStruct& operator*()				{return *m_ptr;}
		DataStruct* operator->()			{return m_ptr;}

		bool operator==( const iterator& o) const	{return m_ptr==o.m_ptr;}
		bool operator!=( const iterator& o) const	{return m_ptr!=o.m_ptr;}
		bool operator>=( const iterator& o) const	{return m_ptr>=o.m_ptr;}
		bool operator>( const iterator& o) const	{return m_ptr>o.m_ptr;}
		bool operator<=( const iterator& o) const	{return m_ptr<=o.m_ptr;}
		bool operator<( const iterator& o) const	{return m_ptr<o.m_ptr;}

		iterator operator+( int idx) const		{return iterator( m_ptr+idx);}
		iterator operator-( int idx) const		{return iterator( m_ptr-idx);}
		int operator-( const iterator& o) const		{return m_ptr-o.m_ptr;}

	private:
		DataStruct* m_ptr;
	};

	const_iterator structbegin() const;
	const_iterator structend() const;
	iterator structbegin();
	iterator structend();

	const_iterator arraybegin() const;
	const_iterator arrayend() const;
	iterator arraybegin();
	iterator arrayend();

	int size() const					{return m_size;}

	///\brief Print the contents of a structure (structures in curly brackets as in the simpleform language)
	void print( QString& out, const QString& indent, const QString& newitem, int level, int maxElemSize=-1) const;

	///\brief Return the contents of a structure as string (format as in print with no indent and newlines)
	QString toString( int maxElemSize=-1) const;

private:
	friend class DataStructDescription;
	friend class DataStructIndirection;
	void setDescription( const DataStructDescription* description_);
	void assign( const DataStruct& o);
	void initStructElements();
	void release();
	bool makeArray();

	int m_size;
	union
	{
		QVariant* elem;
		DataStruct* ref;
	}
	m_data;
	const DataStructDescription* m_description;
	bool m_initialized;
};

class DataStructIndirection :public DataStruct
{
public:
	DataStructIndirection( const DataStructDescription* descr);
};


#endif

