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
#include "serialize/DataStructSerialize.hpp"
#include "serialize/DataStruct.hpp"
#include "serialize/DataSerializeItem.hpp"
#include "debug/DebugHelpers.hpp"
#include "debug/DebugLogTree.hpp"
#include <QStringList>
#include <QDebug>

#undef WOLFRAME_LOWLEVEL_DEBUG

namespace {
class SerializationOutput
{
public:
	SerializationOutput( QList<DataSerializeItem>& serialization_)
		:serialization(&serialization_){}

	void value( const QVariant& val)
	{
		serialization->push_back( DataSerializeItem( DataSerializeItem::Value, val));
	}

	void content( const DataStruct& val)
	{
		getDataStructSerialization( *serialization, val);
	}

	void attribute( const QString& name, const QVariant& val)
	{
		serialization->push_back( DataSerializeItem( DataSerializeItem::Attribute, QVariant(name)));
		serialization->push_back( DataSerializeItem( DataSerializeItem::Value, val));
	}

	void openTag( const QVariant& val)
	{
		serialization->push_back( DataSerializeItem( DataSerializeItem::OpenTag, val));
	}

	void closeTag()
	{
		serialization->push_back( DataSerializeItem( DataSerializeItem::CloseTag, QVariant()));
	}

private:
	QList<DataSerializeItem>* serialization;
};
} //anonymous namespace


bool getDataStructSerialization( QList<DataSerializeItem>& serialization, const DataStruct& data)
{
	SerializationOutput out( serialization);

	if (data.array())
	{
		qCritical() << "internal: invalid call of getDataStructSerialization (with array)";
		return false;
	}
	if (data.atomic())
	{
		out.value( data.value());
		return true;
	}
	else if (data.description())
	{
		DataStruct::const_iterator ei = data.structbegin(), ee = data.structend();
		DataStructDescription::const_iterator di = data.description()->begin(), de = data.description()->end();
		for (; ei != ee && di != de; ++di,++ei)
		{
			if (di->attribute())
			{
				if (ei->array())
				{
					DataStruct::const_iterator ai = ei->arraybegin(), ae = ei->arrayend();
					for (; ai != ae; ++ai)
					{
						out.attribute( di->name, ai->value());
					}
				}
				else
				{
					out.attribute( di->name, ei->value());
				}
			}
			else
			{
				if (ei->array())
				{
					DataStruct::const_iterator ai = ei->arraybegin(), ae = ei->arrayend();
					for (; ai != ae; ++ai)
					{
						out.openTag( di->name);
						out.content( *ai);
						out.closeTag();
					}
				}
				else
				{
					out.openTag( di->name);
					out.content( *ei);
					out.closeTag();
				}
			}
		}
		return true;
	}
	qCritical() << "internal: invalid call of getDataStructSerialization (invalid parameter)";
	return false;
}

bool fillDataStructSerialization( DataStruct& data, const QList<DataSerializeItem>& serialization)
{
	QList<DataSerializeItem>::const_iterator si = serialization.begin(), se = serialization.end();
	QList<DataStruct*> stk;
	QStringList path;
	QString attributename;
	stk.push_back( &data);
	DataSerializeItem::Type lastelemtype = DataSerializeItem::CloseTag;

	for (; si != se && stk.size(); ++si)
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "fillDataStruct visit element " << DataSerializeItem::typeName( si->type()) << " '" << shortenDebugMessageArgument( si->value());
#endif
		switch (si->type())
		{
			case DataSerializeItem::OpenTag:
			{
				const DataStructDescription* descr = stk.back()->description();
				if (!descr)
				{
					qCritical() << "unexpected open tag in non structure" << si->value().toString() << "at" << path.join("/");
					return false;
				}
				int idx = descr->findidx( si->value().toString());
				if (idx < 0)
				{
					qCritical() << "undefined element" << si->value().toString() << "in structure instead of {" << descr->names(",") << "} at " << path.join("/");
					return false;
				}
				DataStruct::iterator ei = stk.back()->structbegin() + idx;
				DataStructDescription::const_iterator di = descr->begin() + idx;

				if (ei->indirection())
				{
					// Handle Indirection:
					ei->expandIndirection();
				}
				if (di->anyValue())
				{
					// Handle {?} (ignoring subtree)
					int taglevel = 0;
					for (; si != se; ++si)
					{
						switch (si->type())
						{
							case DataSerializeItem::OpenTag: ++taglevel; break;
							case DataSerializeItem::CloseTag: --taglevel; break;
							case DataSerializeItem::Attribute: break;
							case DataSerializeItem::Value: break;
						}
						if (taglevel == 0) break;
					}
					if (si == se)
					{
						qCritical() << "tags not balanced in structure fill input";
						return false;
					}
					if (ei->initialized())
					{
						qCritical() << "duplicate initialization of element" << si->value().toString() << "in" << path.join("/");
						return false;
					}
					ei->setInitialized();
				}
				else if (ei->array())
				{
					// Handle array (expand new element of the array):
					ei->push();
					ei->setInitialized();
					if (ei->back()->indirection())
					{
						ei->back()->expandIndirection();
					}
					ei->back()->setInitialized();
					stk.push_back( ei->back());
				}
				else
				{
					// Handle single element:
					if (ei->initialized())
					{
						qCritical() << "duplicate initialization of element" << si->value().toString() << "in" << path.join("/");
						return false;
					}
					ei->setInitialized();
					stk.push_back( &*ei);
				}
				lastelemtype = si->type();
				attributename.clear();
				break;
			}
			case DataSerializeItem::CloseTag:
			{
				DataStruct* elem = stk.back();
				const DataStructDescription* descr = elem->description();
				if (descr)
				{
					// Check if all elements are initialized that have to be:
					DataStructDescription::const_iterator di = descr->begin(), de = descr->end();
					DataStruct::iterator ei = elem->structbegin(), ee = elem->structend();

					for (; di != de && ei != ee; ++di,++ee)
					{
						if (!di->optional() && !di->array() && !di->anyValue() && !ei->initialized())
						{
							qCritical() << "element" << di->name << "not initialized" << "at" << path.join("/");
						}
					}
				}
				stk.pop_back();
				if (stk.isEmpty())
				{
					qCritical() << "tags not balanced in serialization" << "at" << path.join("/");
					return false;
				}
				lastelemtype = si->type();
				attributename.clear();
				break;
			}
			case DataSerializeItem::Attribute:
				lastelemtype = si->type();
				attributename = si->value().toString();
				break;

			case DataSerializeItem::Value:
				if (lastelemtype == DataSerializeItem::Attribute)
				{
					// Initialize attribute value:
					const DataStructDescription* descr = stk.back()->description();
					if (!descr)
					{
						qCritical() << "unexpected attribute in non structure" << si->value().toString() << "at" << path.join("/");
						return false;
					}
					int idx = descr->findidx( attributename);
					if (idx < 0)
					{
						qCritical() << "attribute name not defined" << attributename << "at" << path.join("/");
						return false;
					}
					DataStructDescription::const_iterator di = descr->begin() + idx;
					DataStruct::iterator ei = stk.back()->structbegin() + idx;
					if (di->anyValue())
					{
						// Handle {?} (ignoring value):
						ei->setInitialized();
					}
					else if (ei->array())
					{
						// Handle array (expand new element of the array):
						if (!ei->prototype()->atomic())
						{
							qCritical() << "try to assign attribute value to non atomic element" << attributename << "in" << path.join("/");
							return false;
						}
						ei->push();
						*ei->back() = DataStruct( si->value());
						ei->setInitialized();
						ei->back()->setInitialized();
					}
					else if (ei->initialized())
					{
						qCritical() << "duplicate initialization of element" << attributename << "in" << path.join("/");
					}
					else if (ei->atomic())
					{
						*ei = DataStruct( si->value());
						ei->setInitialized();
					}
					else
					{
						qCritical() << "atomic element expected for attribute value" << attributename << "in" << path.join("/");
					}
					lastelemtype = DataSerializeItem::OpenTag;
					attributename.clear();
				}
				else
				{
					// Initialize content value:
					DataStruct* elem = stk.back();

					///\note Case array already handled in open tag
					if (elem->atomic())
					{
						*elem = DataStruct( si->value());
						elem->setInitialized();
					}
					else
					{
						elem = stk.back()->get( "");
						if (!elem)
						{
							qCritical() << "content value not defined not defined for structure at" << path.join("/");
							return false;
						}
						if (elem->array())
						{
							if (!elem->prototype()->atomic())
							{
								qCritical() << "try to assign attribute value to non atomic element" << attributename << "in" << path.join("/");
								return false;
							}
							elem->push();
							*elem->back() = DataStruct( si->value());
							elem->setInitialized();
							elem->back()->setInitialized();
						}
						else if (elem->atomic())
						{
							if (elem->initialized())
							{
								qCritical() << "duplicate initialization of content element in" << path.join("/");
							}
							*elem = DataStruct( si->value());
							elem->setInitialized();
						}
						else
						{
							qCritical() << "content value non atomic at" << path.join("/");
						}
					}
					lastelemtype = si->type();
					attributename.clear();
				}
				break;
		}
	}
	return true;
}




