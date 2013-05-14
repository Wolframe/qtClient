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
			stk.push_back( SerializeStackElement( QString(), stk.back().tree->nodetree( ni)));
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

static QString getVariableName( const QString& value)
{
	if (value.isEmpty()) return value;
	if (value.at(0) == '{' && value.at(value.size()-1) == '}')
	{
		QString res = value.mid( 1, value.size()-2);
		return res;
	}
	return value;
}

QList<DataSerializeItem> getWidgetDataSerialization( const DataTree& datatree, WidgetVisitor& visitor)
{
	QList<DataSerializeItem> rt;
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
		else if (stk.back().tree->isAttributeNode( ni))
		{
			rt.push_back( DataSerializeItem( DataSerializeItem::Attribute, stk.back().tree->nodename( ni)));
			stk.push_back( SerializeStackElement( stk.back().tree->nodename( ni), stk.back().tree->nodetree(ni)));
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
			stk.push_back( SerializeStackElement( stk.back().tree->nodename( ni), stk.back().tree->nodetree(ni)));

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
	const DataTree* schemanode;
	DataTree* datanode;
	int nodeidx;
	QBitArray initialized;

	AssignStackElement()
		:schemanode(0),datanode(0),nodeidx(0){}
	AssignStackElement( const QString& name_, const DataTree* schemanode_, DataTree* datanode_)
		:name(name_),schemanode( schemanode_),datanode( datanode_),nodeidx(0),initialized(schemanode->size(), false){}
	AssignStackElement( const AssignStackElement& o)
		:name(o.name),schemanode(o.schemanode),datanode(o.datanode),nodeidx(o.nodeidx),initialized(o.initialized){}
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

static bool fillDataTree( DataTree& datatree, const DataTree& schematree, const QList<DataSerializeItem>& answer)
{
	QList<AssignStackElement> stk;
	stk.push_back( AssignStackElement( QString(), &schematree, &datatree));

	if (schematree.elemtype() == DataTree::Array)
	{
		qCritical() << "illegal widget answer description: root node is array";
		return false;
	}
	DataSerializeItem::Type prevtype = DataSerializeItem::CloseTag;

	QList<DataSerializeItem>::const_iterator ai = answer.begin(), ae = answer.end();
	for (; ai != ae; ++ai)
	{
		switch (ai->type())
		{
			case DataSerializeItem::OpenTag:
			{
				int ni = stk.back().nodeidx = stk.back().schemanode->findNodeIndex( ai->value().toString());
				if (ni < 0)
				{
					qCritical() << "element not defined in answer description:" << ai->value() << "at" << elementPath(stk);
					return false;
				}
				if (stk.back().schemanode->isAttributeNode( ni))
				{
					qCritical() << "element is defined as attribute in answer description:" << stk.back().schemanode->nodename( ni) << "at" << elementPath(stk);
					return false;
				}
				if (stk.back().schemanode->elemtype() == DataTree::Array)
				{}
				else if (stk.back().initialized.testBit( ni))
				{
					qCritical() << "duplicate single element:" << stk.back().schemanode->nodename( ni) << "at" << elementPath(stk);
					return false;
				}
				stk.back().initialized.setBit( ni, true);

				stk.push_back( AssignStackElement( stk.back().schemanode->nodename( ni), stk.back().schemanode->nodetree(ni).data(), stk.back().datanode->nodetree(ni).data()));
				break;
			}
			case DataSerializeItem::CloseTag:
			{
				int ii=0, nn=stk.back().initialized.size();
				for (; ii<nn; ++ii)
				{
					if (!stk.back().initialized.testBit( ii) && stk.back().schemanode->nodetree( ii)->elemtype() != DataTree::Array)
					{
						qCritical() << "element not initialized in answer:" << stk.back().schemanode->nodename( ii) << "at" << elementPath(stk);
						return false;
					}
				}
				stk.pop_back();
				break;
			}
			case DataSerializeItem::Attribute:
			{
				int ni = stk.back().nodeidx = stk.back().schemanode->findNodeIndex( ai->value().toString());
				if (ni < 0)
				{
					qCritical() << "attribute not defined in answer description:" << ai->value().toString() << "at" << elementPath(stk);
					return false;
				}
				if (stk.back().schemanode->elemtype() == DataTree::Array)
				{}
				else if (stk.back().initialized.testBit( ni))
				{
					qCritical() << "duplicate attribute:" << stk.back().schemanode->nodename( ni) << "at" << elementPath(stk);
					return false;
				}
				stk.back().initialized.setBit( ni, true);
				if (!stk.back().schemanode->isAttributeNode( ni))
				{
					qCritical() << "attribute is defined as element in answer description:" << ai->value().toString() << "at" << elementPath(stk);
					return false;
				}
				break;
			}
			case DataSerializeItem::Value:
			{
				DataTree* datanode = 0;
				const DataTree* schemanode = 0;
				if (prevtype == DataSerializeItem::Attribute)
				{
					int ni = stk.back().nodeidx;
					schemanode = stk.back().schemanode->nodetree(ni).data();
					datanode = stk.back().datanode->nodetree(ni).data();
				}
				else
				{
					schemanode = stk.back().schemanode;
					datanode = stk.back().datanode;
				}
				if (schemanode && schemanode->value().isValid())
				{
					QString ref = schemanode->value().toString();
					if (ref.size() && ref.at(0) == '{' && ref.at( ref.size()-1) == '}')
					{
						datanode->pushNodeValue( ai->value());
					}
					else if (ref == "{?}")
					{
					}
					else
					{
						qCritical() << "illegal target variable definition in tree:" << ref;
						return false;
					}
				}
				break;
			}
		}
		prevtype = ai->type();
	}
	if (stk.isEmpty() || stk.size() != 1)
	{
		qCritical() << "tags not balanced in answer" << stk.size();
		return false;
	}
	return true;
}

static void getCommonPrefix( QVariant& prefix, const DataTree* schemanode)
{
	if (!prefix.isValid())
	{
		prefix = schemanode->value();
	}
	else if (schemanode->value().isValid())
	{
		QStringList p1 = prefix.toString().split('.');
		QStringList p2 = getVariableName( schemanode->value().toString()).split('.');
		QStringList::const_iterator i1 = p1.begin();
		QStringList::const_iterator i2 = p2.begin();
		int prefixlen = 0;
		for (; i1 != p1.end() && i2 != p2.end(); ++i1,++i2,++prefixlen)
		{
			if (*i1 != *i2) break;
		}
		QString newprefix;
		for (int pi=0; pi<prefixlen; ++pi)
		{
			if (pi) newprefix.push_back('.');
			newprefix.append( p1.at( pi));
		}
		prefix = QVariant( newprefix);
	}
	int ii = 0, nn = schemanode->size();
	for (; ii<nn; ++ii)
	{
		getCommonPrefix( prefix, schemanode->nodetree(ii).data());
	}
}

static bool getArraySize( int& arraysize, const DataTree* datanode)
{
	if (datanode->value().type() == QVariant::List)
	{
		if (arraysize == -1)
		{
			arraysize = datanode->value().toList().size();
		}
		else
		{
			if (arraysize != datanode->value().toList().size())
			{
				qCritical() << "array size do not match" << "!=" << arraysize << datanode->value().toList().size();
				return false;
			}
		}
	}
	int ii = 0, nn = datanode->size();
	for (; ii<nn; ++ii)
	{
		if (datanode->nodetree(ii)->elemtype() != DataTree::Array)
		{
			if (!getArraySize( arraysize, datanode->nodetree(ii).data()))
			{
				return false;
			}
		}
	}
	return true;
}

struct JoinAssignStackElem
{
	int nodeidx;
	const DataTree* schemanode;
	DataTree* datanode;

	JoinAssignStackElem()
		:nodeidx(0),schemanode(0),datanode(0){}
	JoinAssignStackElem( const JoinAssignStackElem& o)
		:nodeidx(o.nodeidx),schemanode(o.schemanode),datanode(o.datanode){}
	JoinAssignStackElem( const DataTree* schemanode_, DataTree* datanode_)
		:nodeidx(0),schemanode(schemanode_),datanode(datanode_){}
};

QList<WidgetDataAssignmentInstr> getWidgetDataAssignments( const DataTree& schematree, const QList<DataSerializeItem>& answer)
{
	QList<WidgetDataAssignmentInstr> rt;

	DataTree datatree = schematree.copyStructure();
	if (!fillDataTree( datatree, schematree, answer))
	{
		qCritical() << "failed to validate request answer";
		return rt;
	}
	QList<JoinAssignStackElem> stk;
	QStringList prefixstk;

	stk.push_back( JoinAssignStackElem( &schematree, &datatree));

	while (!stk.isEmpty())
	{
		int nodeidx = stk.back().nodeidx++;
		if (nodeidx == 0 && stk.back().datanode->value().isValid())
		{
			QString varname( getVariableName( stk.back().schemanode->value().toString()));
			if (!prefixstk.isEmpty())
			{
				QString prefix = prefixstk.back();
				if (varname.startsWith( prefix))
				{
					if (varname.size() == prefix.size())
					{
						varname = varname.mid( prefix.size(), varname.size() - prefix.size());
					}
					else if (varname.at( prefix.size()) == '.')
					{
						varname = varname.mid( prefix.size() + 1, varname.size() - prefix.size() -1);
					}
				}
			}
			rt.push_back( WidgetDataAssignmentInstr( varname, stk.back().datanode->value()));
		}
		if (nodeidx < stk.back().schemanode->size())
		{
			const DataTree* schemanode = stk.back().schemanode->nodetree( nodeidx).data();
			DataTree* datanode = stk.back().datanode->nodetree( nodeidx).data();

			if (schemanode->elemtype() == DataTree::Array)
			{
				QVariant prefix;
				getCommonPrefix( prefix, stk.back().schemanode);
				int arraysize = -1;
				getArraySize( arraysize, stk.back().datanode);
				rt.push_back( WidgetDataAssignmentInstr( arraysize, prefix.toString()));
				prefixstk.push_back( prefix.toString());
			}
			stk.push_back( JoinAssignStackElem( schemanode, datanode));
		}
		else
		{
			const DataTree* schemanode = stk.back().schemanode;
			if (schemanode->elemtype() == DataTree::Array)
			{
				rt.push_back( WidgetDataAssignmentInstr());
				prefixstk.pop_back();
			}
			stk.pop_back();
		}
	}
	return rt;
}



