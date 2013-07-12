#include "DataTreeSerialize.hpp"
#include "DataTree.hpp"
#include "WidgetVisitor.hpp"
#include "DebugHelpers.hpp"
#include <QDebug>
#include <QBitArray>

#undef WOLFRAME_LOWLEVEL_DEBUG
#ifdef WOLFRAME_LOWLEVEL_DEBUG
#define TRACE_VALUE( TITLE, VALUE)			qDebug() << "data tree serialize " << (TITLE) << shortenDebugMessageArgument(VALUE);
#define TRACE_ASSIGNMENT( TITLE, NAME, VALUE)		qDebug() << "data tree serialize " << (TITLE) << (NAME) << "=" << shortenDebugMessageArgument(VALUE);
#else
#define TRACE_VALUE( TITLE, VALUE)
#define TRACE_ASSIGNMENT( TITLE, NAME, VALUE)
#endif


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

static bool mapValue( QList<DataSerializeItem>& res, WidgetVisitor& visitor, QList<SerializeStackElement>& stk, int arrayidx)
{
	bool rt = true;
	QVariant value = stk.back().tree->value();
	QVariant defaultvalue = stk.back().tree->defaultvalue();
	if (stk.back().tree->elemtype() == DataTree::Constant)
	{

		QVariant resolved = visitor.resolve( value);
		res.push_back( DataSerializeItem( DataSerializeItem::Value, resolved.toString()));
	}
	else
	{
		QString propkey = value.toString();
		QVariant prop;

		if (arrayidx >= 0)
		{
			//... we are in an array context
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
					QVariant elem = prop.toList().at(ai);
					if (elem.isValid())
					{
						res.push_back( DataSerializeItem( DataSerializeItem::Value, elem.toString()));
					}
					else if (defaultvalue.isValid())
					{
						res.push_back( DataSerializeItem( DataSerializeItem::Value, defaultvalue.toString()));
					}
					else if (stk.back().tree->isOptional())
					{
						rt = false;
					}
					else
					{
						qCritical() << "accessing non existing property" << propkey;
						rt = false;
					}
				}
				else
				{
					if (ai > 0) qCritical() << "accessing array with index out of range:" << propkey << "index:" << ai << "array:" << prop;
					rt = false;
				}
			}
			else if (prop.isValid())
			{
				res.push_back( DataSerializeItem( DataSerializeItem::Value, prop.toString()));
			}
			else if (defaultvalue.isValid())
			{
				res.push_back( DataSerializeItem( DataSerializeItem::Value, defaultvalue.toString()));
			}
			else if (stk.back().tree->isOptional())
			{
				rt = false;
			}
			else
			{
				qCritical() << "accessing non existing property" << propkey;
				rt = false;
			}
		}
		else
		{
			prop = visitor.property( propkey);
			if (prop.type() == QVariant::List)
			{
				qCritical() << "referencing list of element in a single element context:" << propkey;
				rt = false;
			}
			else if (prop.isValid())
			{
				res.push_back( DataSerializeItem( DataSerializeItem::Value, prop.toString()));
			}
			else if (defaultvalue.isValid())
			{
				res.push_back( DataSerializeItem( DataSerializeItem::Value, defaultvalue.toString()));
			}
			else if (stk.back().tree->isOptional())
			{
				rt = false;
			}
			else
			{
				qCritical() << "accessing non existing property" << propkey;
				rt = false;
			}
		}
	}
	return rt;
}

static int calcArraySize( WidgetVisitor& visitor, const QSharedPointer<DataTree>& dt)
{
	if (dt->elemtype() != DataTree::Array) return 0;
	int rt = 0;

	QList<SerializeStackElement> stk;
	stk.push_back( SerializeStackElement( QString(), dt));
	while (!stk.isEmpty())
	{
		QVariant value;
		if (stk.back().tree->value().isValid())
		{
			switch (stk.back().tree->elemtype())
			{
				case DataTree::Invalid:
				case DataTree::Constant: break;
				case DataTree::Single:
				case DataTree::Array:
					value = visitor.property( stk.back().tree->value().toString());
					break;
			}
		}
		if (value.type() == QVariant::Invalid)
		{
		}
		else if (value.type() == QVariant::List)
		{
			int arsize = value.toList().size();
			if (rt < arsize) rt = arsize;
		}
		else
		{
			if (rt == 0) rt = 1;
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
		int ni = stk.back().nodeidx;
		if (ni >= stk.back().tree->size())
		{
			if (stk.back().tree->value().isValid())
			{
				mapValue( rt, visitor, stk, arrayidx);
			}
			if (!rt.isEmpty() && rt.back().type() == DataSerializeItem::OpenTag && stk.back().tree->elemtype() != DataTree::Array)
			{
				rt.pop_back();
			}
			else
			{
				rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, ""));
			}
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
			if (!mapValue( rt, visitor, stk, arrayidx))
			{
				rt.pop_back();
			}
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
				if (stk.back().arraysize <= 0)
				{
					rt.pop_back();
					stk.pop_back();
					stk.back().nodeidx++;
				}
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
	QBitArray valueset;

	AssignStackElement()
		:schemanode(0),datanode(0),nodeidx(0){}
	AssignStackElement( const QString& name_, const DataTree* schemanode_, DataTree* datanode_)
		:name(name_),schemanode( schemanode_),datanode( datanode_),nodeidx(0),initialized(schemanode->size(), false),valueset(schemanode->size()+1, false){}
	AssignStackElement( const AssignStackElement& o)
		:name(o.name),schemanode(o.schemanode),datanode(o.datanode),nodeidx(o.nodeidx),initialized(o.initialized),valueset(o.valueset){}
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
	bool rt = true;
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
		TRACE_VALUE( DataSerializeItem::typeName( ai->type()), ai->value())

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
					rt = false;
				}
				if (stk.back().schemanode->nodetree(ni)->elemtype() == DataTree::Array)
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
					if (!stk.back().schemanode->nodetree(ii)->isOptional())
					{
						if (!stk.back().initialized.testBit( ii) && stk.back().schemanode->nodetree( ii)->elemtype() != DataTree::Array)
						{
							qCritical() << "element not initialized in answer:" << stk.back().schemanode->nodename( ii) << "at" << elementPath(stk);
							stk.back().datanode->nodetree( ii)->pushNodeValue( stk.back().schemanode->nodetree(ii)->defaultvalue());
						}
						else if (!stk.back().valueset.testBit( ii)
							&& stk.back().schemanode->nodetree(ii)->value().isValid())
						{
							stk.back().datanode->nodetree( ii)->pushNodeValue( QVariant());
						}
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
					rt = false;
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

					if (stk.back().valueset.testBit( ni))
					{
						qCritical() << "duplicate attribute element:" << stk.back().schemanode->nodename( ni) << "at" << elementPath(stk);
						return false;
					}
					stk.back().valueset.setBit( ni, true);
				}
				else
				{
					schemanode = stk.back().schemanode;
					datanode = stk.back().datanode;
					if (stk.size() > 1)
					{
						AssignStackElement* prev = &stk[ stk.size()-2];
						int ni = prev->nodeidx;
						if (stk.back().valueset.testBit( stk.back().valueset.size()-1))
						{
							// ... special index (last) in valueset used for checking duplicates only
							qCritical() << "duplicate value element:" << prev->schemanode->nodename( ni) << "at" << elementPath(stk);
							return false;
						}
						prev->valueset.setBit( ni, true);
						stk.back().valueset.setBit( stk.back().valueset.size()-1, true);
					}
					else
					{
						qCritical() << "value without enclosing tag";
						return false;
					}
				}
				if (schemanode && schemanode->value().isValid())
				{
					datanode->pushNodeValue( ai->value());
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
	return rt;
}

static void getCommonPrefix( QVariant& prefix, const DataTree* schemanode)
{
	if (schemanode->value().isValid())
	{
		if (!prefix.isValid())
		{
			prefix = schemanode->value().toString();
		}
		else
		{
			QString varname = schemanode->value().toString();
			QStringList p1 = prefix.toString().split('.');
			QStringList p2 = varname.split('.');
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
	}
	int ii = 0, nn = schemanode->size();
	for (; ii<nn; ++ii)
	{
		getCommonPrefix( prefix, schemanode->nodetree(ii).data());
	}
}

static bool getArraySize( int& arraysize, int& nofelements, const DataTree* datanode, const DataTree* schemanode, const char* debuglogname)
{
	int osize = -1;
	if (datanode->value().type() != QVariant::Invalid)
	{
		nofelements += 1;
	}
	if (datanode->value().type() == QVariant::List)
	{
		osize = datanode->value().toList().size();
		if (debuglogname) qDebug() << "[getArraySize] size of array" << debuglogname << "=" << osize;
	}
	if (osize >= 0)
	{
		if (arraysize == -1)
		{
			arraysize = osize;
		}
		else
		{
			if (arraysize != osize)
			{
				if (!debuglogname) qCritical() << "array size do not match" << "!=" << arraysize << osize;
				return false;
			}
		}
	}
	int ii = 0, nn = datanode->size();
	for (; ii<nn; ++ii)
	{
		if (!schemanode->nodetree(ii)->value().isValid())
		{
			continue;
		}
		if (datanode->nodetree(ii)->elemtype() != DataTree::Array)
		{
			if (debuglogname) qDebug() << "[getArraySize] visit element" << debuglogname << "/" << schemanode->nodename(ii);
			if (!getArraySize( arraysize, nofelements, datanode->nodetree(ii).data(), schemanode->nodetree(ii).data(), debuglogname?schemanode->nodename(ii).toLatin1().constData():0))
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
	QList<int> arraydimar;
	QStringList prefixstk;

	stk.push_back( JoinAssignStackElem( &schematree, &datatree));

	while (!stk.isEmpty())
	{
		int nodeidx = stk.back().nodeidx++;
		if (nodeidx == 0 && stk.back().datanode->value().isValid())
		{
			QString varname( stk.back().schemanode->value().toString());
			if (stk.back().schemanode->value().isValid())
			{
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
		}
		if (nodeidx < stk.back().schemanode->size())
		{
			const DataTree* schemanode = stk.back().schemanode->nodetree( nodeidx).data();
			DataTree* datanode = stk.back().datanode->nodetree( nodeidx).data();

			if (schemanode->elemtype() == DataTree::Array)
			{
				QVariant prefix;
				getCommonPrefix( prefix, schemanode);
				if (prefix.isValid() && prefix.toString().isEmpty())
				{
					qCritical() << "no common prefix for elements in array";
				}
				int arraysize = -1;
				int arrayinc = 1;
				int nofelements = 0;
				if (!getArraySize( arraysize, nofelements, datanode, schemanode, 0))
				{
					getArraySize( arraysize, nofelements, datanode, schemanode, stk.back().schemanode->nodename( nodeidx).toLatin1().constData());
					return QList<WidgetDataAssignmentInstr>();
				}
				if (!arraydimar.isEmpty()) arrayinc = arraydimar.back();
				if (arraysize < 0 && nofelements > 0)
				{
					arraysize = 1;/*PF:HACK: Have to redesign*/
				}
				arraydimar.push_back( (arraysize>1)?arraysize:1);
				rt.push_back( WidgetDataAssignmentInstr( arraysize/arrayinc, prefix.toString()));
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
				arraydimar.pop_back();
				prefixstk.pop_back();
			}
			stk.pop_back();
		}
	}
	return rt;
}



