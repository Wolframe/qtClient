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
				}
				break;
			case DataSerializeItem::CloseTag:
				break;
			case DataSerializeItem::Attribute:
				break;
			case DataSerializeItem::Value:
				break;
		}
	}
	return false;
}


