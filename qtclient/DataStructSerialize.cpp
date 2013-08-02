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
	DataStruct* elem;
	stk.push_back( &value);

	for (; si != se && stk.size(); ++si)
	{
		switch (si->type())
		{
			case DataSerializeItem::OpenTag:
				if (stk.back()->array())
				{
					stk.back()->push();
					return false;
				}
				else
				{
					elem = stk.back()->get( si->value().toString());
					if (!elem)
					{
						qCritical() << "fill data structure with undefined tag" << path.join("/");
						return false;
					}
					if (elem->array())
					{
						elem->push();
						stk.push_back( elem->back());
						elem->setInitialized();
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
				}
				break;
			case DataSerializeItem::CloseTag:
				stk.pop_back();
				if (stk.isEmpty())
				{
					qCritical() << "tags not balanced in serialization";
				}
				break;
			case DataSerializeItem::Attribute:
				break;
			case DataSerializeItem::Value:
				break;
		}
	}
	return false;
}


