#include "DataTreeSerialize.hpp"
#include "DataTree.hpp"
#include "WidgetVisitor.hpp"
#include <QDebug>
#include <QBitArray>

struct SerializeStackElement
{
	QString name;
	QSharedPointer<DataTree> tree;
	int arraysize;
	int arrayelemidx;
	int nodeidx;
	QHash<QString, QVariant> arrayValueMap;
	QBitArray initialized;

	SerializeStackElement()
		:arraysize(-1),arrayelemidx(0),nodeidx(0){}
	SerializeStackElement( const QString& name_, const DataTree& tree_)
		:name(name_),tree( new DataTree( tree_)),arraysize(-1),arrayelemidx(0),nodeidx(0),initialized(tree_.size(), false){}
	SerializeStackElement( const QString& name_, const QSharedPointer<DataTree>& tree_)
		:name(name_),tree(tree_),arraysize(-1),arrayelemidx(0),nodeidx(0),initialized(tree_->size(), false){}
	SerializeStackElement( const SerializeStackElement& o)
		:name(o.name),tree(o.tree),arraysize(o.arraysize),arrayelemidx(o.arrayelemidx),nodeidx(o.nodeidx),arrayValueMap(o.arrayValueMap),initialized(o.initialized){}
};

static void mapValue( QList<DataSerializeItem>& rt, WidgetVisitor& visitor, QList<SerializeStackElement>& stk, int arrayidx)
{
	QString value = stk.back().tree->value().toString();
	if (value.size() > 1)
	{
		if (value.at(0) == '{' && value.at(value.size()-1) == '}')
		{
			QString propkey = value.mid( 1, value.size()-2);
			QVariant prop;
			if (arrayidx >= 0)
			{
				int ai = stk.at(arrayidx).arrayelemidx;
				if (ai == 0)
				{
					prop = stk[ arrayidx].arrayValueMap[ propkey] = visitor.property( propkey);
				}
				else
				{
					prop = stk[ arrayidx].arrayValueMap[ propkey];
				}
				if (prop.type() == QVariant::List)
				{
					if (prop.toList().size() > ai)
					{
						rt.push_back( DataSerializeItem( DataSerializeItem::Value, prop.toList().at(ai)));
					}
					else
					{
						qCritical() << "accessing array with index out of range:" << propkey;
						rt.push_back( DataSerializeItem( DataSerializeItem::Value, ""));
					}
				}
				else if (prop.isValid())
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, prop.toString()));
				}
				else
				{
					qCritical() << "accessing non existing property" << propkey;
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, QString()));
				}
			}
			else
			{
				prop = visitor.property( propkey);
				if (prop.type() == QVariant::List)
				{
					qCritical() << "referencing list of element in a single element context:" << propkey;
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, ""));
				}
				else if (prop.isValid())
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, prop.toString()));
				}
				else
				{
					qCritical() << "accessing non existing property" << propkey;
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, QString()));
				}
			}
		}
		else
		{
			QVariant resolved = visitor.resolve( value);
			rt.push_back( DataSerializeItem( DataSerializeItem::Value, resolved.toString()));
		}
	}
	else
	{
		rt.push_back( DataSerializeItem( DataSerializeItem::Value, value));
	}
}

static int calcArraySize( WidgetVisitor& visitor, const QSharedPointer<DataTree>& dt)
{
	if (dt->elemtype() != DataTree::Array) return 0;
	int rt = 0;

	QList<SerializeStackElement> stk;
	stk.push_back( SerializeStackElement( QString(), dt));
	while (!stk.isEmpty())
	{
		QVariant value = visitor.resolve( stk.back().tree->value());
		if (value.type() == QVariant::List)
		{
			int arsize = value.toList().size();
			if (rt < arsize) rt = arsize;
		}
		int ni = stk.back().nodeidx++;
		if (ni >= stk.back().tree->size())
		{
			stk.pop_back();
		}
		else
		{
			stk.push_back( SerializeStackElement( QString(), stk.back().tree->nodevalue( ni)));
		}
	}
	return rt;
}

static int getLastArrayIdx( const QList<SerializeStackElement>& stk)
{
	int arrayidx = stk.size()-1;
	for (;arrayidx >= 0; --arrayidx)
	{
		if (stk.at(arrayidx).tree->elemtype() == DataTree::Array)
		{
			break;
		}
	}
	return arrayidx;
}

QString getVariableName( const QString& value)
{
	if (value.isEmpty()) return value;
	if (value.at(0) == '{' && value.at(value.size()-1) == '}')
	{
		QString res = value.mid( 1, value.size()-2);
		return res;
	}
	return value;
}

QList<DataSerializeItem> getWidgetDataSerialization( const DataTree& datatree, QWidget* widget)
{
	QList<DataSerializeItem> rt;
	WidgetVisitor visitor( widget, (WidgetVisitor::VisitorFlags)(WidgetVisitor::BlockSignals));
	QList<SerializeStackElement> stk;
	stk.push_back( SerializeStackElement( QString(), datatree));
	int arrayidx = -1;

	if (stk.back().tree->elemtype() == DataTree::Array)
	{
		qCritical() << "illegal widget data serialization: root node is array";
		return rt;
	}
	while (!stk.isEmpty())
	{
		if (stk.back().tree->value().isValid())
		{
			mapValue( rt, visitor, stk, arrayidx);
		}
		int ni = stk.back().nodeidx;
		if (ni >= stk.back().tree->size())
		{
			rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, ""));

			if (stk.back().tree->elemtype() == DataTree::Array && ++stk.back().arrayelemidx < stk.back().arraysize)
			{
				rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, stk.back().name));
				stk.back().nodeidx = 0;
			}
			else
			{
				stk.pop_back();
				if (arrayidx >= stk.size()) arrayidx = getLastArrayIdx( stk);
				if (!stk.isEmpty()) stk.back().nodeidx++;
			}
			continue;
		}
		else if (stk.back().tree->isAttribute( ni))
		{
			rt.push_back( DataSerializeItem( DataSerializeItem::Attribute, stk.back().tree->nodename( ni)));
			stk.push_back( SerializeStackElement( stk.back().tree->nodename( ni), stk.back().tree->nodevalue(ni)));
			if (stk.back().tree->elemtype() == DataTree::Array)
			{
				qCritical() << "illegal data tree description: attributes not allowed as array (" << stk.back().tree->nodename( ni) << ")";
				return QList<DataSerializeItem>();
			}
			mapValue( rt, visitor, stk, arrayidx);
			stk.pop_back();
			stk.back().nodeidx++;
		}
		else
		{
			rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, stk.back().tree->nodename( ni)));
			stk.push_back( SerializeStackElement( stk.back().tree->nodename( ni), stk.back().tree->nodevalue(ni)));

			if (stk.back().tree->elemtype() == DataTree::Array)
			{
				arrayidx = stk.size()-1;
				stk.back().arraysize = calcArraySize( visitor, stk.back().tree);
			}
		}
	}
	return rt;
}

struct AssignStackElement
{
	QString name;
	QSharedPointer<DataTree> tree;
	int nodeidx;
	QBitArray initialized;

	AssignStackElement()
		:nodeidx(0){}
	AssignStackElement( const QString& name_, const DataTree& tree_)
		:name(name_),tree( new DataTree( tree_)),nodeidx(0),initialized(tree_.size(), false){}
	AssignStackElement( const QString& name_, const QSharedPointer<DataTree>& tree_)
		:name(name_),tree(tree_),nodeidx(0),initialized(tree_->size(), false){}
	AssignStackElement( const AssignStackElement& o)
		:name(o.name),tree(o.tree),nodeidx(o.nodeidx),initialized(o.initialized){}
};

static QString elementPath( const QList<AssignStackElement>& stk)
{
	QString rt;
	QList<AssignStackElement>::const_iterator pi = stk.begin(), pe = stk.end();
	for (; pi != pe; ++pi)
	{
		if (pi != stk.begin()) rt.push_back( '/');
		rt.append( pi->name);
	}
	return rt;
}

QList<DataSerializeItem> getWidgetDataAssignments( const DataTree& datatree, QWidget* widget, const QList<DataSerializeItem>& answer)
{
	QList<DataSerializeItem> rt;
	WidgetVisitor visitor( widget, (WidgetVisitor::VisitorFlags)(WidgetVisitor::BlockSignals));
	QList<AssignStackElement> stk;
	stk.push_back( AssignStackElement( QString(), datatree));
	QList<DataSerializeItem>::const_iterator ai = answer.begin(), ae = answer.end();

	if (stk.back().tree->elemtype() == DataTree::Array)
	{
		qCritical() << "illegal widget answer description: root node is array";
		return rt;
	}
	for (; ai != ae; ++ai)
	{
		switch (ai->type())
		{
			case DataSerializeItem::OpenTag:
			{
				int ni = stk.back().nodeidx = stk.back().tree->findNodeIndex( ai->value().toString());
				if (ni < 0)
				{
					qCritical() << "element not defined in answer description:" << ai->value() << "at" << elementPath(stk);
					return QList<DataSerializeItem>();
				}
				stk.back().initialized.setBit( ni, true);
				if (stk.back().tree->isAttributeNode( ni))
				{
					qCritical() << "element is defined as attribute in answer description:" << stk.back().tree->nodename( ni) << "at" << elementPath(stk);
				}
				stk.push_back( AssignStackElement( stk.back().tree->nodename( ni), stk.back().tree->nodevalue(ni)));
				if (stk.back().tree->elemtype() == DataTree::Array)
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, ""));
				}
				break;
			}
			case DataSerializeItem::CloseTag:
			{
				int ii=0, nn=stk.back().initialized.size();
				for (; ii<nn; ++ii)
				{
					if (!stk.back().initialized[ii] && stk.back().tree->nodevalue( ii)->elemtype() != DataTree::Array)
					{
						qCritical() << "element not initialized in answer:" << stk.back().tree->nodename( ii) << "at" << elementPath(stk);
					}
				}
				if (stk.back().tree->elemtype() == DataTree::Array)
				{
					int taglevel = -1;
					QStringList commonPrefix;
					bool commonPrefixSet = false;
					QList<DataSerializeItem>::iterator di = rt.end();

					while (di != rt.begin())
					{
						--di;
						if (di->type() == DataSerializeItem::CloseTag)
						{
							--taglevel;
						}
						if (di->type() == DataSerializeItem::OpenTag)
						{
							++taglevel;
							if (taglevel == 0)
							{
								*di = DataSerializeItem( DataSerializeItem::OpenTag, commonPrefix.join("."));
								break;
							}
						}
						if (taglevel == -1 && di->type() == DataSerializeItem::Attribute)
						{
							if (!commonPrefixSet)
							{
								commonPrefix = di->value().toString().split('.');
								commonPrefixSet = true;
							}
							else
							{
								QStringList prefix = di->value().toString().split('.');
								int ii=0, nn=prefix.size();
								for (; ii<nn && ii<commonPrefix.size() && prefix.at(ii)==commonPrefix.at(ii); ++ii);
								while (ii < commonPrefix.size()) commonPrefix.removeLast();
							}
						}
					}
					if (!commonPrefix.isEmpty())
					{
						QString prefix = commonPrefix.join(".") + ".";
						for (taglevel=0; di != rt.end(); ++di)
						{
							if (di->type() == DataSerializeItem::CloseTag)
							{
								--taglevel;
							}
							if (di->type() == DataSerializeItem::OpenTag)
							{
								++taglevel;
							}
							if (taglevel == 1 && di->type() == DataSerializeItem::Attribute)
							{
								QString name = di->value().toString();
								if (!name.startsWith( prefix))
								{
									qCritical() << "internal error (getWidgetDataAssignments prefix calc)";
								}
								else
								{
									*di = DataSerializeItem( DataSerializeItem::Attribute, name.mid( prefix.size(), name.size() - prefix.size()));
								}
							}
						}
					}
					rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, commonPrefix.join(".")));
				}
				stk.pop_back();
			}
			case DataSerializeItem::Attribute:
			{
				int ni = stk.back().nodeidx = stk.back().tree->findNodeIndex( ai->value().toString());
				if (ni < 0)
				{
					qCritical() << "attribute not defined in answer description:" << ai->value().toString() << "at" << elementPath(stk);
					return QList<DataSerializeItem>();
				}
				stk.back().initialized.setBit( ni, true);
				if (!stk.back().tree->isAttributeNode( ni))
				{
					qCritical() << "attribute is defined as element in answer description:" << ai->value().toString() << "at" << elementPath(stk);
				}
				break;
			}
			case DataSerializeItem::Value:
			{
				int ni = stk.back().nodeidx;
				QVariant ref = stk.back().tree->nodevalue(ni)->value();
				if (!ref.isValid())
				{
					qCritical() << "value not defined in answer description:" << stk.back().tree->nodename(ni) << "at" << elementPath(stk);
				}
				else
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::Attribute, getVariableName( ref.toString())));
					rt.push_back( DataSerializeItem( DataSerializeItem::Value, ai->value()));
				}
			}
		}
	}
	if (!stk.isEmpty())
	{
		qCritical() << "tags not balanced in answer";
	}
	return rt;
}



