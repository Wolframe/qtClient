//
// DataHandler.hpp
//

#ifndef _DATA_HANDLER_INCLUDED
#define _DATA_HANDLER_INCLUDED

#include <QWidget>
#include <QString>
#include <QByteArray>

#include "DataLoader.hpp"

namespace _Wolframe {
	namespace QtClient {

	class DataHandler : public QObject
	{
		Q_OBJECT
		
		public:
			DataHandler( DataLoader *_dataLoader );
			virtual ~DataHandler( ) {};
			void readFormData( QString name, QWidget *form, QByteArray &data );
			void writeFormData( QString name, QWidget *form, QByteArray *data );
			void resetFormData( QWidget *form );
			void loadFormDomain( QString form_name, QString widget_name, QWidget *form, QByteArray &data );

		private:
			void loadFormDomains( QString name, QWidget *form );
		
		private:
			DataLoader *m_dataLoader;
	};
} // namespace QtClient
} // namespace _Wolframe

#endif // _DATA_HANDLER_INCLUDED