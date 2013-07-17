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

#ifndef _Wolframe_CLIENT_HPP_INCLUDED
#define _Wolframe_CLIENT_HPP_INCLUDED

#include <QObject>
#include <QWidget>
#include <QAbstractSocket>
#include <QList>
#include <QQueue>
#include <QStringList>
#include <QTimer>
#include <QSslSocket>
#include <QSslCertificate>

#include "serverDefinition.hpp"

class WolframeClientProtocol;

class LIBWOLFRAMECLIENT_VISIBILITY WolframeClient : public QObject
{
	Q_OBJECT

	public:
		enum State {
			Disconnected,
			AboutToConnect,
			Connected,
			Data,
			AboutToDisconnect
		};

		struct WolframeRequest
		{
			QString command;
			QStringList params;
			QString content;

			WolframeRequest( const QString &_command, const QStringList &_params, const QString &_content )
				: command( _command ), params( _params ), content( _content ) { }
		};

	private:
		ServerDefinition m_connParams;
		State m_state;
		QAbstractSocket *m_socket;
		bool m_hasErrors;
		WolframeClientProtocol *m_protocol;
		QTimer *m_timeoutTimer;

	public:
		WolframeClient( const ServerDefinition _connParams,	QWidget *_parent = 0 );
		virtual ~WolframeClient( );

		void setConnectionParameters( const ServerDefinition _connParams );

		static bool SSLsupported( );

// low-level commands, pre-protocol, for debugging mainly
		void connect();
		void disconnect();
		void auth();
		void request( const QString& cmd, const QString& tag, const QByteArray& content );
// inquire status
		bool isConnected( ) const;
		const QString serverName() const;
		bool isEncrypted( ) const;
		const QString encryptionCipher() const;

	private:
		void processProtocol();

	private slots:
		void timeoutOccurred( );
		void error( QAbstractSocket::SocketError );
		void dataAvailable( );
		void privateConnected( );
		void privateDisconnected( );

		void handleResult( );

	signals:
// low-level events
		void connected( );
		void disconnected( );
		void error( QString error );
		void authOk( );
		void authFailed( );

// low-level commands, pre-protocol, for debugging mainly
		void lineReceived( QString line );
		void lineSent( QString line );

// generic implementation of a command execute implementing the frame of the protocol
		void resultReceived( );
		void resultHandled( );

// high-level commands
		void answerReceived( bool success, const QString& tag, const QByteArray& content );

// intentionally no WITH_SSL here (maintain binary compatibility, a D-ptr is better (TODO), but
// how to slots work with d_ptr?)
	private:
		bool m_initializedSsl;

	private slots:
		void initializeSsl( );
		QSslCertificate getCertificate( QString filename );
		void sslErrors( const QList<QSslError> &errors );
		void encrypted( );
		void peerVerifyError( const QSslError &e );
};

#endif // _Wolframe_CLIENT_HPP_INCLUDED
