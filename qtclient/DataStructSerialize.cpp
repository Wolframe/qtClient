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
	QString attributename;
	stk.push_back( &value);
	DataSerializeItem::Type lastelemtype = DataSerializeItem::CloseTag;

	for (; si != se && stk.size(); ++si)
	{
		switch (si->type())
		{
			case DataSerializeItem::OpenTag:
			{
				const DataStructDescription* descr = stk.back()->description();
				if (!descr)
				{
					qCritical() << "unexpected open tag in non structure" << si->value().toString() << "at" << path.join("/");
				}
				int idx = descr->findidx( si->value().toString());
				if (idx < 0)
				{
					qCritical() << "undefined element" << si->value().toString() << "in structure instead of tag" << si->value().toString() << "at" << path.join("/");
				}
				DataStruct::iterator ei = stk.back()->begin() + idx;
				DataStructDescription::const_iterator di = descr->begin() + idx;
				if (di->anyValue())
				{
					int taglevel = 1;
					for (; si != se && taglevel > 0; ++si)
					{
						switch (si->type())
						{
							case DataSerializeItem::OpenTag: taglevel++; break;
							case DataSerializeItem::CloseTag: taglevel++; break;
							case DataSerializeItem::Attribute: break;
							case DataSerializeItem::Value: break;
						}
					}
					if (si == se)
					{
						qCritical() << "tags not balanced in structure fill input";
						return false;
					}
					ei->setInitialized();
				}
				else if (di->type == DataStructDescription::indirection_)
				{

				}
				else if (ei->array())
				{
					ei->push();
					ei->setInitialized();
					ei->back()->setInitialized();
					stk.push_back( ei->back());
				}
				else if (ei->initialized())
				{
					qCritical() << "duplicate initialization of element" << si->value().toString() << "in" << path.join("/");
					return false;
				}
				else
				{
					ei->setInitialized();
					stk.push_back( &*ei);
				}
				lastelemtype = si->type();
				break;
			}
			case DataSerializeItem::CloseTag:
			{
				DataStruct* elem = stk.back();
				const DataStructDescription* descr = elem->description();
				if (!descr)
				{
					qCritical() << "expected structure on close tag" << "at" << path.join("/");
					return false;
				}
				else
				{
					DataStructDescription::const_iterator di = descr->begin(), de = descr->end();
					DataStruct::iterator ei = elem->begin(), ee = elem->end();

					for (; di != de && ei != ee; ++di,++ee)
					{
						if (!di->optional() && !di->anyValue() && !ei->initialized())
						{
							qCritical() << "element" << di->name << "not initialized" << "at" << path.join("/");
						}
					}
				}
				stk.pop_back();
				if (stk.isEmpty())
				{
					qCritical() << "tags not balanced in serialization" << "at" << path.join("/");
				}
				lastelemtype = si->type();
				break;
			}
			case DataSerializeItem::Attribute:
				lastelemtype = si->type();
				attributename = si->value().toString();
				break;

			case DataSerializeItem::Value:
				if (lastelemtype == DataSerializeItem::Attribute)
				{
					const DataStructDescription* descr = stk.back()->description();
					if (!descr)
					{
						qCritical() << "unexpected attribute in non structure" << si->value().toString() << "at" << path.join("/");
					}
					int idx = descr->findidx( attributename);
					if (idx < 0)
					{
						qCritical() << "attribute name not defined" << attributename << "at" << path.join("/");
						return false;
					}
					DataStructDescription::const_iterator di = descr->begin() + idx;
					DataStruct::iterator ei = stk.back()->begin() + idx;
					if (di->anyValue())
					{
						ei->setInitialized();
					}
					else if (ei->array())
					{
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
				}
				else
				{
					DataStruct* elem = stk.back();

					///\note Case array already handled in open tag
					if (elem->atomic())
					{
						if (elem->initialized())
						{
							qCritical() << "duplicate initialization of element" << attributename << "in" << path.join("/");
						}
						*elem = DataStruct( si->value());
						stk.pop_back();
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




