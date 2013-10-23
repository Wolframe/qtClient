/************************************************************************

 Copyright (C) 2011 - 2013 Project Wolframe.
 All rights reserved.

 This file is part of Project Wolframe.

 Commercial Usage
    Licensees holding valid Project Wolframe Commercial licenses may
    use this file in accordance with the Project Wolframe
    Commercial License Agreement provided with the Software or,
    alternatively, in accordance with the terms contained
    in a written agreement between the licensee and Project Wolframe.

 GNU General Public License Usage
    Alternatively, you can redistribute this file and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wolframe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wolframe.  If not, see <http://www.gnu.org/licenses/>.

 If you have questions regarding the use of this file, please contact
 Project Wolframe.

************************************************************************/
#include "debug/DebugMessageHandler.hpp"
#include "debug/DebugLogTree.hpp"

static char g_debug = false;
void setDebugMode( bool v)
{
	g_debug = v;
}

#if QT_VERSION >= 0x050000
void wolframeMessageOutput( QtMsgType type, const QMessageLogContext & /*context */, const QString &msg )
{
	switch( type ) {
		case QtDebugMsg:
			if( g_debug ) {
				if (getDebugLogTree())
				{
					printLog( LogDebug, msg);
				}
				else
				{
#ifdef _WIN32
					OutputDebugString( qPrintable( msg ) );
#else
					fprintf( stderr, "%s\n", qPrintable( msg ) );
#endif
				}
			}
			break;

		case QtWarningMsg:
			if (getDebugLogTree())
			{
				printLog( LogWarning, msg);
			}
			else
			{
#ifdef _WIN32
				OutputDebugString( qPrintable( msg ) );
#else
				fprintf( stderr, "WARNING: %s\n", qPrintable( msg ) );
#endif
			}
			break;

		case QtCriticalMsg:
			if (getDebugLogTree())
			{
				printLog( LogCritical, msg);
			}
			else
			{
#ifdef _WIN32
				OutputDebugString( qPrintable( msg ) );
#else
				fprintf( stderr, "CRITICAL: %s\n", qPrintable( msg ) );
#endif
			}
			break;

		case QtFatalMsg:
			if (getDebugLogTree())
			{
				printLog( LogFatal, msg);
			}
			else
			{
#ifdef _WIN32
				OutputDebugString( qPrintable( msg ) );
#else
				fprintf( stderr, "FATAL: %s\n", qPrintable( msg ) );
#endif
			}
			break;

		default:
			break;
	}
}
#else

#ifdef _WIN32
static LPWSTR s2ws( const char *s )
{
	int len;
	int slength = (int)strlen( s );
	len = MultiByteToWideChar( CP_ACP, 0, s, slength, 0, 0 );
	wchar_t *buf = new wchar_t[len+1];

	MultiByteToWideChar( CP_ACP, 0, s, slength, buf, len );
	buf[len] = 0;
	return buf;
}
#endif

void wolframeMessageOutput( QtMsgType type, const char *msg )
{
	switch( type ) {
		case QtDebugMsg:
			if( g_debug ) {
				if (getDebugLogTree())
				{
					printLog( LogDebug, msg);
				}
				else
				{
#ifdef _WIN32
					LPWSTR wmsg = s2ws( msg );
					OutputDebugString( wmsg );
					delete[] wmsg;
#else
					fprintf( stderr, "%s\n", msg );
#endif
				}
			}
			break;

		case QtWarningMsg:
			if (getDebugLogTree())
			{
				printLog( LogWarning, msg);
			}
			else
			{
#ifdef _WIN32
				LPWSTR wmsg = s2ws( msg );
				OutputDebugString( wmsg );
				delete[] wmsg;
#else
				fprintf( stderr, "WARNING: %s\n", msg );		
#endif
			}
			break;

		case QtCriticalMsg:
			if (getDebugLogTree())
			{
				printLog( LogCritical, msg);
			}
			else
			{
#ifdef _WIN32
				LPWSTR wmsg = s2ws( msg );
				OutputDebugString( wmsg );
				delete[] wmsg;
#else
				fprintf( stderr, "CRITICAL: %s\n", msg );
#endif
			}
			break;

		case QtFatalMsg:
			if (getDebugLogTree())
			{
				printLog( LogFatal, msg);
			}
			else
			{
#ifdef _WIN32
				LPWSTR wmsg = s2ws( msg );
				OutputDebugString( wmsg );
				delete[] wmsg;
#else
				fprintf( stderr, "FATAL: %s\n", msg );
#endif
			}
			break;

		default:
			break;
	}
}
#endif
