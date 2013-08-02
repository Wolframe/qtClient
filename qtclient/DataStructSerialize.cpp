#include "DataStructSerialize.hpp"
#include "DataStruct.hpp"
#include "DataSerializeItem.hpp"
#include <QStringList>
#include <QDebug>

bool getDataStructSerialization( QList<DataSerializeItem>& serialization, const DataStruct& value)
{
	return false;
}

bool fillDataStructSerialization( DataStruct& value, const QList<DataSerializeItem>& serialization)
{
	QList<DataSerializeItem>::const_iterator si = serialization.begin(), se = serialization.end();
	QList<DataStruct*> stk;
	QStringList path;
	DataStruct* elem = 0;
	QString attributename;
	stk.push_back( &value);
	DataSerializeItem::Type lastelemtype = DataSerializeItem::CloseTag;

	for (; si != se && stk.size(); ++si)
	{
		switch (si->type())
		{
			case DataSerializeItem::OpenTag:
				elem = stk.back()->get( si->value().toString());
				if (!elem)
				{
					qCritical() << "atomic value expected in structure instead of tag" << si->value().toString() << "at" << path.join("/");
					return false;
				}
				if (elem->array())
				{
					elem->push();
					elem->setInitialized();
					elem->back()->setInitialized();
					stk.push_back( elem->back());
				}
				else if (elem->initialized())
				{
					qCritical() << "duplicate initialization of element" << si->value().toString() << "in" << path.join("/");
				}
				else
				{
					elem->setInitialized();
					stk.push_back( elem);
				}
				lastelemtype = si->type();
				break;

			case DataSerializeItem::CloseTag:
				stk.pop_back();
				if (stk.isEmpty())
				{
					qCritical() << "tags not balanced in serialization";
				}
				lastelemtype = si->type();
				break;

			case DataSerializeItem::Attribute:
				lastelemtype = si->type();
				attributename = si->value().toString();
				break;

			case DataSerializeItem::Value:
				if (lastelemtype == DataSerializeItem::Attribute)
				{
					elem = stk.back()->get( attributename);
					if (!elem)
					{
						qCritical() << "attribute name not defined" << attributename << "at" << path.join("/");
						return false;
					}
					if (elem->initialized())
					{
						qCritical() << "duplicate initialization of element" << attributename << "in" << path.join("/");
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
						*elem = DataStruct( si->value());
						elem->setInitialized();
					}
					else
					{
						qCritical() << "atomic element expected for attribute value" << attributename << "in" << path.join("/");
					}
					lastelemtype = DataSerializeItem::OpenTag;
				}
				else
				{
					elem = stk.back();
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
							qCritical() << "duplicate initialization of element" << attributename << "in" << path.join("/");
						}
						*elem = DataStruct( si->value());
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
						}
						else
						{
							qCritical() << "content value non atomic at" << path.join("/");
						}
					}
					lastelemtype = si->type();
				}
				break;
		}
	}
	return false;
}




