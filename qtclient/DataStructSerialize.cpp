#include "DataStructSerialize.hpp"
#include "DataStruct.hpp"
#include "DataSerializeItem.hpp"
#include <QStringList>
#include <QDebug>

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
		DataStruct::const_iterator ei = data.begin(), ee = data.end();
		DataStructDescription::const_iterator di = data.description()->begin(), de = data.description()->end();
		for (; ei != ee && di != de; ++di,++ei)
		{
			if (di->attribute())
			{
				if (ei->array())
				{
					DataStruct::const_iterator ai = ei->begin(), ae = ei->end();
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
					DataStruct::const_iterator ai = ei->begin(), ae = ei->end();
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

				if (ei->indirection())
				{
					// Handle Indirection:
					ei->expandIndirection();
				}
				if (di->anyValue())
				{
					// Handle {?} (ignoring subtree)
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
					// Check if all elements are initialized that have to be:
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
					// Initialize attribute value:
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
				}
				else
				{
					// Initialize content value:
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




