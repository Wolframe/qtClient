#include "serialize/DataStructDescription.hpp"
#include "debugview/DebugLogTree.hpp"

#undef WOLFRAME_LOWLEVEL_DEBUG

static void skipBrk( QString::const_iterator& itr, const QString::const_iterator& end)
{
	if (itr == end || *itr != '{')
	{
		qCritical() << "internal: illegal call in this state (skipBrk)";
		return;
	}
	int brkcnt = 1;
	++itr;
	for (; itr != end; ++itr)
	{
		if (*itr == '{')
		{
			++brkcnt;
			continue;
		}
		if (*itr == '}')
		{
			--brkcnt;
			if (brkcnt == 0) break;
		}
	}
}

static void skipSpaces( QString::const_iterator& itr, const QString::const_iterator& end)
{
	for (; itr != end && itr->isSpace(); ++itr) {}
}

static bool isAlphaNum( QChar ch)
{
	return ch.isLetter() || ch == '_' || ch.isDigit();
}


class ParseContext
{
private:
	struct Element
	{
		QString name;
		QString structname;
		bool isArray;
		QSharedPointer<DataStructDescription> description;

		Element( const Element& o)
			:name(o.name),structname(o.structname),isArray(o.isArray),description(o.description){}
		Element( const QString& name_, const QString& structname_, bool isArray_)
			:name(name_),structname(structname_),isArray(isArray_),description(new DataStructDescription()){}
		Element()
			:isArray(false),description(new DataStructDescription()){}
	};

	QList<Element> stk;
	QList<QString> errmsg;

public:
	ParseContext( const ParseContext& o)
		:stk(o.stk),errmsg(o.errmsg){}
	ParseContext()
	{
		stk.push_back( Element());
	}

	const QList<QString>& errors() const
	{
		return errmsg;
	}

	void setError( const QString& msg)
	{
		QString pt;
		foreach (const Element& ei, stk)
		{
			if (pt.size()) pt.append("/");
			pt.append( ei.name);
		}
		errmsg.push_back( QString("error in datastructure description at ") + pt + ": " + msg);
	}

	const DataStructDescription* description() const
	{
		if (stk.isEmpty()) return 0;
		return &*stk.back().description;
	}

	bool open( const QString& name, const QString& structname, bool array)
	{
		stk.push_back( Element( name, structname, array));
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL open(" << name << "," << structname << ")";
#endif
		return true;
	}

	bool close()
	{
		if (stk.size() <= 1)
		{
			setError( "internal: illegal state in structure parser");
			return false;
		}
		Element elem = stk.back();
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "IN close() PARSED" << elem.description->toString();
#endif

		stk.pop_back();
		int idx = stk.back().description->addStructure( elem.name, *elem.description);
		if (0>idx)
		{
			setError( QString( "duplicate definition of structure '") + elem.name + "'");
			return false;
		}
		if (elem.isArray)
		{
			stk.back().description->at( idx).makeArray();
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL close()";
#endif
		return true;
	}

	int defineAttributeConst( const QString& name, const QVariant& value)
	{
		int idx = stk.back().description->addAttribute( name, value);
		if (idx < 0)
		{
			setError( QString( "duplicate definition of value '") + name + "'");
			return idx;
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL defineAttributeConst(" << name << value << ")";
#endif
		return idx;
	}

	int defineAttributeVariable( const QString& name, const QString& var, const QVariant& defaultvalue)
	{
		int idx = stk.back().description->addAttributeVariable( name, var, defaultvalue);
		if (idx < 0)
		{
			setError( QString( "duplicate definition of value '") + name + "'");
			return idx;
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL defineAttributeVariable(" << name << var << defaultvalue << ")";
#endif
		return idx;
	}

	int defineAtomConst( const QString& name, const QVariant& value)
	{
		int idx = stk.back().description->addAtom( name, value);
		if (idx < 0)
		{
			setError( QString( "duplicate definition of value '") + name + "'");
			return idx;
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL defineAtomConst(" << name << value << ")";
#endif
		return idx;
	}

	int defineAtomVariable( const QString& name, const QString& var, const QVariant& defaultvalue)
	{
		int idx = stk.back().description->addAtomVariable( name, var, defaultvalue);
		if (idx < 0)
		{
			setError( QString( "duplicate definition of value '") + name + "'");
			return idx;
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL defineAtomVariable(" << name << var << defaultvalue << ")";
#endif
		return idx;
	}

	///\brief define indirection node (recursive structure)
	///\param[in] name name of the node
	///\param[in] indirection_structname name of the structure of the indirection (currently a parent but could also be an "oncle")
	int defineIndirection( const QString& name, const QString& indirection_structname)
	{
		int idx = -1;
		bool resolved = false;
		QList<Element>::const_iterator ei = stk.end();
		while (ei != stk.begin())
		{
			--ei;
			if (ei->structname == indirection_structname)
			{
				idx = stk.back().description->addIndirection( name, ei->description.data());
				if (idx < 0)
				{
					setError( QString( "duplicate definition of element '") + indirection_structname + "'");
					return idx;
				}
				resolved = true;
				break;
			}
		}
		if (!resolved)
		{
			setError( QString( "failed to resolve indirection '") + indirection_structname + "'");
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL defineIndirection(" << name << "," << indirection_structname << ")";
#endif
		return idx;
	}

	struct VariableReference
	{
		QString varname;
		QVariant defaultvalue;
		bool optional;

		VariableReference()
			:optional(false){}
	};

	bool parseVariableReference( VariableReference& def, QString::const_iterator& is, const QString::const_iterator& es)
	{
		QString::const_iterator start = is+1;
		skipBrk( is, es);
		if (is == es)
		{
			setError( QString("curly brackets not balanced {..}:") + QString( start,is-start));
			return false;
		}
		def.varname = QString( start, is-start).trimmed();
		++is;
		int dd;
		if (def.varname.indexOf('{') >= 0)
		{
			setError( QString( "expected variable reference instead of expression '") + def.varname + "'");
			return false;
		}
		if (def.varname == "?")
		{
			def.varname = "";
			def.optional = true;
		}
		else if ((dd=def.varname.indexOf(':')) >= 0)
		{
			def.optional = true;

			QString defaultvaluestr = def.varname.mid( dd+1, def.varname.size()-dd-1);
			if (defaultvaluestr == "?")
			{
				def.varname = def.varname.mid( 0, dd);
			}
			else
			{
				def.defaultvalue = QVariant( defaultvaluestr);
				def.varname = def.varname.mid( 0, dd);
			}
		}
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL parseVariableReference(" << QString(is, es-is) << ") RETURNS" << def.varname << def.defaultvalue;
#endif
		return true;
	}

	bool parseString( QString& str, QString::const_iterator& itr, const QString::const_iterator& end)
	{
		QChar eb = *itr;
		QString::const_iterator start;
		for (start=++itr; itr != end && *itr != eb; ++itr)
		{
			if (*itr == '\\')
			{
				++itr;
				if (itr == end)
				{
					setError( "string not terminated");
					return false;
				}
			}
		}
		if (itr == end)
		{
			setError( "string not terminated");
			return false;
		}
		str = QString( start, itr-start);
		if (itr != end) ++itr;
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "CALL parseString(" << QString( itr, end-itr) << ") RETURNS" << str;
#endif
		return true;
	}

	bool parseArrayMarker( QString::const_iterator& itr, const QString::const_iterator& end)
	{
		++itr;
		skipSpaces( itr, end);

		if (itr != end && *itr == ']')
		{
			++itr;
			return true;
		}
		else
		{
			setError( "array marker not closed, expected ']' after '['");
			return false;
		}
	}

	bool parseDataStructDescription( QString::const_iterator& bi, const QString::const_iterator& be)
	{
		while (bi != be)
		{
			bool isIndirection = false;
			skipSpaces( bi, be);
			if (bi == be) break;

			if (*bi == '^')
			{
				++bi;
				isIndirection = true;
			}
			if (isAlphaNum(*bi))
			{
				QString nodename;

				QString structname;
				bool isArray = false;

				for (; bi != be && isAlphaNum(*bi); ++bi)
				{
					nodename.push_back( *bi);
				}
				skipSpaces( bi, be);
				if (*bi == ':')
				{
					structname.clear();
					++bi;
					for (; bi != be && isAlphaNum(*bi); ++bi)
					{
						structname.push_back( *bi);
					}
					skipSpaces( bi, be);
				}
				else
				{
					structname = nodename;
				}
				if (*bi == '[')
				{
					if (!parseArrayMarker( bi, be)) return false;
					isArray = true;
					skipSpaces( bi, be);
				}
				if (isIndirection)
				{
					int idx = defineIndirection( nodename, structname);
					if (idx < 0) return false;
					if (isArray)
					{
						stk.back().description->at( idx).makeArray();
					}
				}
				else if (bi == be)
				{
					setError( "unexpected end of expression");
					return false;
				}
				else if (*bi == '{')
				{
					//... Content
					QString::const_iterator start = bi+1;
					skipBrk( bi, be);
					if (bi == be)
					{
						setError( "invalid tree (curly brackets not balanced)");
						return false;
					}
					skipSpaces( start, bi);
					if (start == bi)
					{
						setError( "invalid tree (empty expression or variable reference: \"{}\")");
						return false;
					}
					if (*start == '{')
					{
						//... Content variable reference
						VariableReference vardef;
						if (!parseVariableReference( vardef, start, bi)) return false;
						int idx = defineAtomVariable( nodename, vardef.varname, vardef.defaultvalue);
						if (idx < 0) return false;

						if (vardef.optional)
						{
							if (vardef.varname.isEmpty())
							{
								stk.back().description->at( idx).setAnyValue();
							}
							stk.back().description->at( idx).setOptional();
						}
						if (isArray)
						{
							stk.back().description->at( idx).makeArray();
						}
						++bi;
					}
					else if (*start == '?')
					{
						//... Any content
						++start;
						skipSpaces( start, bi);
						if (start != bi)
						{
							setError( "invalid tree (unexpected token after '?')");
							return false;
						}
						int idx = defineAtomVariable( nodename, "", QVariant());
						if (idx < 0) return false;
						stk.back().description->at( idx).setOptional();
						++bi;
					}
					else
					{
						//... Content substructure
						if (!open( nodename, structname, isArray)) return false;
						if (!parseDataStructDescription( start, bi)) return false;
						if (!close()) return false;
						++bi;
					}
				}
				else if (*bi == '=')
				{
					// ... Attribute
					++bi; skipSpaces( bi, be);
					if (bi == be)
					{
						setError( "invalid tree (expected expresbion or string as attribute value)");
						return false;
					}
					if (*bi == '\'' || *bi == '\"')
					{
						//... Attribute constant
						QString content;
						if (!parseString( content, bi, be)) return false;
						if (0>defineAttributeConst( nodename, content)) return false;
					}
					else if (*bi == '{')
					{
						//... Attribute variable reference
						VariableReference vardef;
						if (!parseVariableReference( vardef, bi, be)) return false;
						int idx = defineAttributeVariable( nodename, vardef.varname, vardef.defaultvalue);
						if (idx < 0) return false;
						if (vardef.optional)
						{
							if (vardef.varname.isEmpty())
							{
								stk.back().description->at( idx).setAnyValue();
							}
							stk.back().description->at( idx).setOptional();
						}
						if (isArray)
						{
							stk.back().description->at( idx).makeArray();
						}
					}
					else if (isAlphaNum(*bi))
					{
						QString::const_iterator start = bi;
						for (++bi; bi != be && isAlphaNum(*bi); ++bi) {}
						if (0>defineAttributeConst( nodename, QString( start, bi-start))) return false;
					}
					else
					{
						setError( "invalid tree (expected curly bracket expression or string)");
						return false;
					}
				}
				else
				{
					setError( "invalid tree (expected curly bracket expression or string after identifier)");
				}
			}
			else if (*bi == '\'' || *bi == '\"')
			{
				//... Content constant
				QString content;
				if (!parseString( content, bi, be)) return false;
				if (0>defineAtomConst( "", content)) return false;
			}
			else if (*bi == '{')
			{
				//... Content variable
				VariableReference vardef;
				if (!parseVariableReference( vardef, bi, be)) return false;
				int idx = defineAtomVariable( "", vardef.varname, vardef.defaultvalue);
				if (idx < 0) return false;
				if (vardef.optional)
				{
					if (vardef.varname.isEmpty())
					{
						stk.back().description->at( idx).setAnyValue();
					}
					stk.back().description->at( idx).setOptional();
				}
			}
			else if (*bi == '?')
			{
				//... Any content
				int idx = defineAtomVariable( "", "", QVariant());
				if (idx < 0) return false;
				stk.back().description->at( idx).setAnyValue();
			}
			else
			{
				setError( "expected expression or string");
				return false;
			}
			skipSpaces( bi, be);
			if (bi != be)
			{
				if (*bi == ';' || *bi == ',')
				{
					++bi;
				}
				else if (*bi == '}')
				{
					setError( "curly brackets not balanced");
				}
				else
				{
					setError( "invalid tree");
					return false;
				}
			}
		}
		return true;
	}
};

bool DataStructDescription::parse( QString::const_iterator si, const QString::const_iterator& se, QList<QString>& err)
{
	qDebug() << "Parse DataStructDescription [" << QString( si, se-si) << "]";
	QString::const_iterator bi = si;
	QString::const_iterator be = se;
	if (*bi != '{')
	{
		qCritical() << "structure does not start with '{'";
		return false;
	}
	skipBrk( bi, be);
	if (bi == be)
	{
		qCritical() << "curly brackets not balanced in structure description:" << QString( si, se-si);
		return false;
	}
	be = bi;
	++bi;
	skipSpaces( bi, se);
	if (bi != se)
	{
		if (*bi == '}')
		{
			qCritical() << "curly brackets not balanced in structure description:" << QString( si, se-si);
		}
		else
		{
			qCritical() << "extra content at end of structure description";
		}
		return false;
	}
	bi = si+1;

	ParseContext ctx;
	if (!ctx.parseDataStructDescription( bi, be))
	{
		foreach (const QString& msg, ctx.errors())
		{
			qCritical() << msg;
		}
		err = ctx.errors();
		return false;
	}
	err.clear();
	const DataStructDescription* res = ctx.description();
	if (!res)
	{
		qCritical() << "internal: data structure description parse result empty";
		return false;
	}
	inherit( *res);
	return true;
}


bool DataStructDescription::parse( const QString& source, QList<QString>& err)
{
	QString::const_iterator si = source.begin(), se = source.end();
	return parse( si, se, err);	
}


