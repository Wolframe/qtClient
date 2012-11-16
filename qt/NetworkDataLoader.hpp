//
// NetworkDataLoader.hpp
//

#ifndef _NETWORK_DATA_LOADER_INCLUDED
#define _NETWORK_DATA_LOADER_INCLUDED

#include "DataLoader.hpp"
#include "WolframeClient.hpp"

#include <QHash>
#include <QPair>
#include <QScopedPointer>
	
class NetworkDataLoader : public DataLoader
{	
	// intentionally omitting Q_OBJECT here, is done in DataLoader!
	
	public:
		NetworkDataLoader( WolframeClient *_wolframeClient );
		virtual ~NetworkDataLoader( ) {};

		virtual void request( QString formName, QString widgetName, QByteArray xml, QHash<QString, QString> *props );

	private:
		void handleCreate( QString name, QByteArray data, QHash<QString, QString> *props );
		void handleRead( QString name, QHash<QString, QString> *props );
		void handleUpdate( QString name, QByteArray data, QHash<QString, QString> *props );
		void handleDelete( QString name, QHash<QString, QString> *props );
		void handleDomainDataLoad( QString formName, QString widgetName, QHash<QString, QString> *props );
		QString mapAction( QString action );
		
	public slots:
		virtual void gotAnswer( QStringList params, QString content );
		virtual void gotError( QString error );
			
	private:
		WolframeClient *m_wolframeClient;
		QScopedPointer<QHash<QString, QPair<QString, QString> > > m_map;
};

#endif // _NETWORK_DATA_LOADER_INCLUDED
//
// NetworkDataLoader.hpp
//
