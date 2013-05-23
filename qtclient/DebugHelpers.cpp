#include "DebugHelpers.hpp"

template <class StringType>
static StringType shortenDebugMessageArgument_( const StringType& msg)
{
	StringType rt;
	int cnt = 0;
	typename StringType::const_iterator mi = msg.begin(), me = msg.end();
	for (; mi != me; ++mi)
	{
		if (*mi == '<' || *mi == '>' || *mi == '"' || *mi == '\'' || *mi == ' ')
		{
			cnt = 0;
		}
		if (cnt == 60)
		{
			rt.append( "...");
			for (; mi != me; ++mi)
			{
				if (*mi == '<' || *mi == '>' || *mi == '"' || *mi == '\'' || *mi == ' ')
				{
					rt.push_back( *mi);
					break;
				}
			}
			if (mi == me) break;
			cnt = 0;
		}
		else
		{
			rt.push_back( *mi);
		}
		cnt++;
	}
	return rt;
}

QString shortenDebugMessageArgument( const QString& msg)
{
	return shortenDebugMessageArgument_( msg);
}

QByteArray shortenDebugMessageArgument( const QByteArray& msg)
{
	return shortenDebugMessageArgument_( msg);
}

QVariant shortenDebugMessageArgument( const QVariant& val)
{
	if (val.type() == QVariant::String && val.toString().size() > 200)
	{
		return shortenDebugMessageArgument_( val.toString());
	}
	if (val.type() == QVariant::ByteArray && val.toByteArray().size() > 200)
	{
		return shortenDebugMessageArgument_( val.toByteArray());
	}
	if (val.type() == QVariant::List)
	{
		QList<QVariant> rt;
		foreach (const QVariant& vv, val.toList())
		{
			rt.push_back( shortenDebugMessageArgument( vv));
		}
		return QVariant(rt);
	}
	return val;
}

