// List of locales

	for ( int langIdx = QLocale::C; langIdx <= QLocale::LastLanguage; langIdx++ )	{
		QLocale::Language lang = static_cast< QLocale::Language >( langIdx ) ;
		QList< QLocale::Country > countries = QLocale::countriesForLanguage( lang );
		for ( int i = 0; i < countries.count(); i++ )	{
			QLocale::Country country = countries.at( i );
			QString label = QLocale::languageToString( lang );
			label += QLatin1Char( '/' );
			label += QLocale::countryToString( country );
			QLocale locale( lang, country );
			if ( this->locale().language() == lang && this->locale().country() == country )
				curLocaleIndex = index;
			m_localeCombo->addItem( label, locale );
			index++;
		}
	}
