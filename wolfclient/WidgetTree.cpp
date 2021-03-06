#include "WidgetTree.hpp"
#include "WidgetMessageDispatcher.hpp"
#include "WidgetDragAndDrop.hpp"
#include "WidgetRequest.hpp"
#include "WidgetDataSignal.hpp"
#include "debugview/DebugHelpers.hpp"
#include "debugview/DebugLogTree.hpp"
#include <QDebug>
#include <QPushButton>

#undef WOLFRAME_LOWLEVEL_DEBUG

static bool isInternalWidget( const QWidget* widget)
{
	return widget->objectName().isEmpty()
		|| widget->objectName().startsWith( "qt_")
		|| widget->objectName().startsWith( "_q");
}

static bool nodeProperty_hasListener( const QWidget* widget, const QVariant&)
{
	if (WidgetListenerImpl::hasDataSignals( widget)) return true;
	if (widget->property( "contextmenu").isValid()) return true;
	return false;
}

static bool nodeProperty_hasDebugListener( const QWidget* widget, const QVariant&)
{
	if (WidgetListenerImpl::hasDataSignals( widget)) return true;
	if (widget->property( "contextmenu").isValid()) return true;
	if (widget->property( "action").isValid()) return true;
	if (widget->property( "form").isValid()) return true;
	foreach (const QByteArray& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith( "action:") || prop.startsWith( "form:")) return true;
	}
	return false;
}


WidgetTree::WidgetTree( DataLoader *_dataLoader, QHash<QString,QVariant>* _globals, bool debug_)
	:m_globals(_globals)
	,m_dataLoader(_dataLoader)
	,m_debug(debug_){}



void WidgetTree::setEnablers( QWidget* widget, const QList<WidgetEnablerImpl::Trigger>& trigger)
{
	WidgetVisitor visitor( widget);
	QHash<QString,WidgetEnablerR> enablermap;

	foreach (const WidgetEnablerImpl::Trigger& trg, trigger)
	{
		QWidget* ownerwidget = visitor.getPropertyOwnerWidget( trg.condition.property);
		if (ownerwidget)
		{
			WidgetEnablerR enabler;
			QString objName = ownerwidget->objectName();

			QHash<QString,WidgetEnablerR>::const_iterator eni = enablermap.find( objName);
			if (eni != enablermap.end())
			{
				enabler = eni.value();
			}
			else
			{
				enabler = WidgetEnablerR( new WidgetEnablerImpl( widget, trigger));
				QHash<QString,QList<WidgetEnablerR> >::const_iterator fi = m_enablers.find( objName), fe = m_enablers.end();
				if (fi == fe)
				{
					m_enablers.insert( objName, QList<WidgetEnablerR>());
				}
				enablermap[ objName] = enabler;
				m_enablers[ objName].push_back( enabler);
			}
			WidgetVisitor ownervisitor( ownerwidget);
			ownervisitor.connectWidgetEnabler( *enabler);
		}
		else
		{
			qCritical() << "could not evaluate widget delivering property" << trg.condition.property << "for widget" << visitor.widgetPath();
		}
	}
	QHash<QString,WidgetEnablerR>::iterator bi = enablermap.begin(), be = enablermap.end();
	for (; bi != be; ++bi)
	{
		bi.value()->handle_changed();
	}
}

void WidgetTree::setPushButtonEnablers( QPushButton* pushButton)
{
	typedef WidgetEnablerImpl::Trigger Trigger;
	QList<Trigger> triggers;
	QList<QString> enable_props;
	WidgetVisitor button_visitor( pushButton);

	foreach (const QString& prop, getActionRequestProperties( button_visitor))
	{
		if (!enable_props.contains( prop))
		{
			triggers.push_back( Trigger( WidgetEnablerImpl::Enabled, prop));
			enable_props.push_back( prop);
		}
	}
	foreach (const QString& prop, getFormCallProperties( pushButton->property( "form").toString()))
	{
		triggers.push_back( Trigger( WidgetEnablerImpl::Enabled, prop));
		if (!enable_props.contains( prop)) enable_props.push_back( prop);
	}
	setEnablers( pushButton, triggers);
}

void WidgetTree::setDeclaredEnablers( QWidget* widget)
{
	QList<WidgetEnablerImpl::Trigger> triggers;
	foreach (const QByteArray& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith( "state:"))
		{
			WidgetVisitor vv( widget);

			QByteArray nam = prop.mid( 6, prop.size()-6);
			QVariant condexpr = widget->property( prop);
			WidgetEnablerImpl::Condition cond;

			if (!WidgetEnablerImpl::parseCondition( cond, condexpr.toString()))
			{
				qCritical() << "failed to set enabler defined by property" << prop << "of widget" << widget->metaObject()->className() << vv.widgetPath();
			}
			if (nam == "visible")
			{
				WidgetEnablerImpl::Trigger trg( WidgetEnablerImpl::Visible, cond);
				triggers.push_back( trg);
			}
			else if (nam == "hidden")
			{
				WidgetEnablerImpl::Trigger trg( WidgetEnablerImpl::Hidden, cond);
				triggers.push_back( trg);
			}
			else if (nam == "enabled")
			{
				WidgetEnablerImpl::Trigger trg( WidgetEnablerImpl::Enabled, cond);
				triggers.push_back( trg);
			}
			else if (nam == "disabled")
			{
				WidgetEnablerImpl::Trigger trg( WidgetEnablerImpl::Disabled, cond);
				triggers.push_back( trg);
			}
			else
			{
				qCritical() << "unknown state property name" << prop << "in widget" << vv.widgetPath();
			}
		}
	}
	setEnablers( widget, triggers);
}

void WidgetTree::signalEnablers()
{
	QHash<QString,QList<WidgetEnablerR> >::iterator li = m_enablers.begin(), le = m_enablers.end();
	for (; li != le; ++li)
	{
		QList<WidgetEnablerR>::iterator ei = li->begin(), ee = li->end();
		for (; ei != ee; ++ei) ei->data()->handle_changed();
	}
}

QVariant WidgetTree::getWidgetStates() const
{
	QList<QVariant> rt;
	if (m_visitor.widget()) foreach( QWidget *widget, m_visitor.widget()->findChildren<QWidget*>())
	{
		if (!isInternalWidget( widget))
		{
			WidgetVisitor visitor( widget);
			QVariant state = visitor.getState();
			if (state.isValid())
			{
				rt.push_back( visitor.objectName());
				rt.push_back( state);
			}
		}
	}
	if (rt.isEmpty()) return QVariant();
	return QVariant(rt);
}

void WidgetTree::setWidgetStates( const QVariant& state)
{
	QList<QVariant> statelist = state.toList();
	QList<QVariant>::const_iterator itr = statelist.begin();

	foreach( QWidget *widget, m_visitor.widget()->findChildren<QWidget*>())
	{
		if (!isInternalWidget( widget))
		{
			if (itr != statelist.end() && widget->objectName() == itr->toString())
			{
				WidgetVisitor visitor( widget);
				++itr;
				if (itr != statelist.end())
				{
					if (widget->property( "action").isValid())
					{
						widget->setProperty( "_w_state", *itr);
					}
					else
					{
						visitor.setState( *itr);
					}
					++itr;
				}
			}
		}
	}
}

void WidgetTree::saveVariables()
{
	m_visitor.do_writeAssignments();
	m_visitor.do_writeGlobals( *m_globals);
}

bool WidgetTree::initialize( QWidget* ui_, QWidget* oldUi, const QString& formcall, const QString& logid)
{
	openLogStruct( logid);
	openLogStruct( "load");

	m_formCall.init( formcall);
	m_visitor = WidgetVisitor( ui_);
	m_enablers.clear();
	m_listeners.clear();

	// initialize the form variables given by globals
	bool rudpflag_bk = m_visitor.allowUndefDynPropsInit( true);

	WidgetVisitor::init_widgetids( m_visitor.widget());
	m_visitor.do_readGlobals( *m_globals);

	// initialize the form variables given by form parameters
	foreach (const FormCall::Parameter& param, m_formCall.parameter())
	{
		if (!m_visitor.setProperty( QString( param.first), param.second))
		{
			qCritical() << "Failed to set UI parameter" << param.first << "=" << param.second;
		}
		else
		{
			qDebug( ) << "Set UI parameter" << param.first << "=" << param.second;
		}
	}
	// initialize the form variables given by assignments
	m_visitor.do_readAssignments();
	m_visitor.allowUndefDynPropsInit( rudpflag_bk);

	// restore widget states if form was opened with a '_CLOSE_'
	if (m_state.isValid())
	{
		setWidgetStates( m_state);
	}

	// set widget state according to 'state:..' properties and their conditionals
	foreach (QWidget* chld, ui_->findChildren<QWidget*>())
	{
		setDeclaredEnablers( chld);
	}

	// enable drop in all sub widgets where 'drop..' properties exists:
	enableDropForFormWidgetChildren( ui_);

	foreach (QWidget* chld, ui_->findChildren<QWidget *>()) {
		QVariant initialFocus = chld->property( "initialFocus");
		if (initialFocus.toBool()) chld->setFocus();
	}

	// connect listener to signals converted to data signals
	if (m_debug)
	{
		foreach (QWidget* datasig_widget, m_visitor.findSubNodes( nodeProperty_hasDebugListener))
		{
			WidgetVisitor datasig_widget_visitor( datasig_widget);
			WidgetListenerR listener( datasig_widget_visitor.createListener( m_dataLoader));
			listener->setDebug( m_debug);
			m_listeners[ datasig_widget_visitor.widgetid()].push_back( listener);
		}
	}
	else
	{
		foreach (QWidget* datasig_widget, m_visitor.findSubNodes( nodeProperty_hasListener))
		{
			WidgetVisitor datasig_widget_visitor( datasig_widget);
			WidgetListenerR listener( datasig_widget_visitor.createListener( m_dataLoader));
			listener->setDebug( m_debug);
			m_listeners[ datasig_widget_visitor.widgetid()].push_back( listener);
		}
	}

	// initialize enablers for push buttons
	foreach (QPushButton* pb, ui_->findChildren<QPushButton*>())
	{
		setPushButtonEnablers( pb);
	}

	// push on form stack for back link
	QList<QVariant> formstack;
	QList<QVariant> statestack;
	if( oldUi )
	{
		formstack = oldUi->property( "_w_formstack").toList();
		statestack = oldUi->property( "_w_statestack").toList();
	}
	formstack.push_back( formcall);
	statestack.push_back( m_state );

	qDebug() << "form stack for " << formcall << ":" << formstack;
	m_visitor.widget()->setProperty( "_w_formstack", QVariant( formstack));
	m_visitor.widget()->setProperty( "_w_statestack", QVariant( statestack));

	closeLogStruct(2);

	// loads the domains
	WidgetMessageDispatcher dispatcher( m_visitor);
	foreach (const WidgetRequest& request, dispatcher.getDataloadRequests( m_debug))
	{
		openLogStruct( request.header.toLogIdString());
		openLogStruct( "request");

		if (request.content.isEmpty())
		{
			qWarning() << "request is empty";
		}
		else
		{
			m_dataLoader->datarequest( request.header.command.toString(), request.header.toString(), request.content);
		}
		closeLogStruct(2);
	}
	return true;
}

void WidgetTree::handleDatasignal( WidgetVisitor& visitor, const char* signame)
{
	DataSignalHandler dshandler( m_dataLoader, m_debug);
	QList<DataSignalReceiver> rcvlist;
	QWidget* widget = visitor.widget();
	QVariant prop = widget->property( QByteArray("datasignal:") + signame);
	if (prop.isValid())
	{
		foreach (const QString& rcvaddr, parseDataSignalList( prop.toString()))
		{
			rcvlist.append( getDataSignalReceivers( visitor, rcvaddr, true));
		}
		foreach (const DataSignalReceiver& receiver, rcvlist)
		{
			dshandler.trigger( receiver.first, receiver.second);
		}
	}
}

QWidget* WidgetTree::deliverAnswer( const QString& tag, const QByteArray& content, QString& followform)
{
	QWidget* rt = 0;
	WidgetRequestHeader requestheader( tag);
	WidgetMessageDispatcher dispatcher( m_visitor);

	openLogStruct( requestheader.toLogIdString());
	openLogStruct( "answer");

	if (!requestheader.recipient.widgetid.isValid())
	{
		qCritical() << "cannot deliver answer for request with undefined widget id (tag " << tag << ")";
		closeLogStruct( 2);
		return 0;
	}
	QList<QWidget*> rcpl = dispatcher.findRecipientWidgets( requestheader.recipient.widgetid);
	if (rcpl.isEmpty())
	{
		closeLogStruct( 2);
		return 0;
	}

	if (requestheader.type == WidgetRequestHeader::Action)
	{
		qDebug() << "got action request answer header=[" << requestheader.toString() << "] data=" << shortenDebugMessageArgument(content);
		foreach (QWidget* actionwidget, rcpl)
		{
			QPushButton* button = qobject_cast<QPushButton*>( actionwidget);
			if (button) button->setDown( false);
			if (rt)
			{
				qCritical() << "ambiguous form switch request (duplicate widget address for action request)";
				break;
			}
			rt = actionwidget;
			if (!content.isEmpty())
			{
				// set widget answer for dependent actions/forms:
				WidgetVisitor rcpvisitor( actionwidget, (WidgetVisitor::VisitorFlags)(WidgetVisitor::BlockSignals));
				if (!setWidgetAnswer( rcpvisitor, content))
				{
					qCritical() << "Failed to assign action answer to widget:" << rcpvisitor.widgetPath() << "message tag:" << tag << "message data:" << shortenDebugMessageArgument(content);
				}
			}
			followform = requestheader.actionid.toString();
		}
	}
	else
	{
		foreach (QWidget* rcp, rcpl)
		{
			rt = rcp;

			//block signals before assigning the answer
			typedef QHash<QWidget*,bool> TraceMap;
			TraceMap blksig;
#ifdef WOLFRAME_LOWLEVEL_DEBUG
			qDebug() << "blocking signals in WidgetTree for receiving object" << rcp->metaObject()->className() << rcp->objectName();
#endif

			{
				bool prevstate = rcp->blockSignals(true);
				blksig.insert( rcp, prevstate);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
				qDebug() << "block signals of previously" << ((prevstate)?"blocked":"unblocked") << "widget" << rcp->metaObject()->className() << rcp->objectName();
#endif
			}
			foreach (QWidget* cld, rcp->findChildren<QWidget*>())
			{
				if (!isInternalWidget( cld))
				{
					bool prevstate = cld->blockSignals(true);
					blksig.insert( cld, prevstate);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
					qDebug() << "block signals of previously" << ((prevstate)?"blocked":"unblocked") << "widget" << cld->metaObject()->className() << cld->objectName();
#endif
				}
			}
			WidgetVisitor rcpvisitor( rcp, (WidgetVisitor::VisitorFlags)(WidgetVisitor::BlockSignals));
			if (!setWidgetAnswer( rcpvisitor, content))
			{
				qCritical() << "Failed to assign request answer to widget:" << rcpvisitor.widgetPath() << "message tag:" << tag << "message data:" << shortenDebugMessageArgument(content);
			}
			rcpvisitor.setState( rcp->property( "_w_state"));

			// handle data signal 'loaded' if defined:
			if (rcp->property( "datasignal:loaded").isValid())
			{
				handleDatasignal( rcpvisitor, "loaded");
			}

			//unblock blocked signals after assigning the answer
			TraceMap::const_iterator ti = blksig.find( rcp);
			if (ti != blksig.end())
			{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
				bool prevstate = rcp->blockSignals( ti.value());
				qDebug() << ((ti.value())?"blocking":"unblocking") << "signals of previously" << ((prevstate)?"blocked":"unblocked") << rcp->metaObject()->className() << rcp->objectName();
#else
				rcp->blockSignals( ti.value());
#endif
			}
			else
			{
				qDebug() << "widget to reset blocked state not found" << rcp->objectName();
			}

			foreach (QWidget* cld, rcp->findChildren<QWidget*>())
			{
				if (!isInternalWidget( cld))
				{
					TraceMap::const_iterator ti = blksig.find( cld);
					if (ti != blksig.end())
					{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
						bool prevstate = cld->blockSignals( ti.value());
						qDebug() << ((ti.value())?"blocking":"unblocking") << "signals of previously" << ((prevstate)?"blocked":"unblocked") << cld->metaObject()->className() << cld->objectName();
#else
						cld->blockSignals( ti.value());
#endif
					}
					else
					{
						qDebug() << "widget to reset blocked state not found" << cld->objectName();
					}
				}
			}
		}
		signalEnablers();
	}
	closeLogStruct( 2);
	return rt;
}

QWidget* WidgetTree::deliverError( const QString& tag, const QByteArray& /*content*/)
{
	QWidget* rt = 0;
	WidgetMessageDispatcher dispatcher( m_visitor);
	WidgetRequestHeader requestheader( tag);

	openLogStruct( requestheader.toLogIdString());
	openLogStruct( "error answer");

	if (!requestheader.recipient.widgetid.isValid())
	{
		qCritical() << "cannot deliver error for request with undefined widget id (tag " << tag << ")";
		closeLogStruct( 2);
		return 0;
	}
	QList<QWidget*> rcpl = dispatcher.findRecipientWidgets( requestheader.recipient.widgetid);
	if (rcpl.isEmpty())
	{
		closeLogStruct( 2);
		return 0;
	}
	rt = rcpl.at(0);
	if (requestheader.type == WidgetRequestHeader::Action)
	{
		foreach (QWidget* actionwidget, rcpl)
		{
			rt = actionwidget;
			QPushButton* button = qobject_cast<QPushButton*>( actionwidget);
			if (button) button->setDown( false);
		}
	}
	closeLogStruct( 2);
	return rt;
}

QString WidgetTree::popCurrentForm()
{
	QString rt;
	QList<QVariant> formstack = m_visitor.widget()->property("_w_formstack").toList();
	QList<QVariant> statestack = m_visitor.widget()->property("_w_statestack").toList();
	m_state = QVariant();

	if (formstack.isEmpty())
	{
		return QString();
	}
	else
	{
		qDebug() << "form stack before pop (_RESET_):" << formstack;

		rt = formstack.back().toString();
		formstack.pop_back();
		if (!statestack.isEmpty())
		{
			statestack.pop_back();
		}
		m_visitor.widget()->setProperty( "_w_formstack", QVariant( formstack));
		m_visitor.widget()->setProperty( "_w_statestack", QVariant( statestack));

		qDebug() << "load form from stack:" << rt;
		return rt;
	}
}

QString WidgetTree::popPreviousForm()
{
	QString rt;
	//... _CLOSE_ calls loadForm with the predecessor of
	//    the top form of the form stack and the formstate
	//    stored there (parallel on the statestack).
	QList<QVariant> formstack = m_visitor.widget()->property("_w_formstack").toList();
	QList<QVariant> statestack = m_visitor.widget()->property("_w_statestack").toList();
	m_state = QVariant();

	if (!formstack.isEmpty())
	{
		qDebug() << "form stack before pop (_CLOSE_):" << formstack;

		// discard the top element of the form/state stack
		formstack.pop_back();
		if (!statestack.isEmpty())
		{
			statestack.pop_back();
		}
		if (formstack.isEmpty()) return QString();

		// fetch the predecessor of the form/state stack
		// as next form+state
		rt = formstack.back().toString();
		formstack.pop_back();

		if (!statestack.isEmpty())
		{
			m_state = statestack.back();
			statestack.pop_back();
		}
		m_visitor.widget()->setProperty( "_w_formstack", QVariant( formstack));
		m_visitor.widget()->setProperty( "_w_statestack", QVariant( statestack));

		qDebug() << "load form from stack:" << rt;
	}
	return rt;
}

void WidgetTree::saveWidgetStates()
{
	QList<QVariant> statestack = m_visitor.widget()->property("_w_statestack").toList();
	if (!statestack.isEmpty())
	{
		// replace state of the state stack:
		statestack.pop_back();
		statestack.push_back( getWidgetStates());
		m_visitor.widget()->setProperty( "_w_statestack", QVariant( statestack));
	}
}

