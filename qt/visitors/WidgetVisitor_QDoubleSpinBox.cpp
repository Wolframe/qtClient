#include "WidgetVisitor_QDoubleSpinBox.hpp"
#include "WidgetVisitor.hpp"

WidgetVisitorState_QDoubleSpinBox::WidgetVisitorState_QDoubleSpinBox( QWidget* widget_)
	:WidgetVisitor::State(widget_)
	,m_doubleSpinBox(qobject_cast<QDoubleSpinBox*>( widget_))
{
}

bool WidgetVisitorState_QDoubleSpinBox::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QDoubleSpinBox::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QDoubleSpinBox::clear()
{
	// set to value between minimum and maximum
	double midval = (m_doubleSpinBox->minimum() + m_doubleSpinBox->maximum()) / 2;
	int stp = (int)(midval / m_doubleSpinBox->singleStep());
	double initval = m_doubleSpinBox->minimum() + stp * m_doubleSpinBox->singleStep();
	m_doubleSpinBox->setValue( initval);
}

QVariant WidgetVisitorState_QDoubleSpinBox::property( const QString& name)
{
	if (name.isEmpty())
	{
		return m_doubleSpinBox->value();
	}
	return QVariant();
}

bool WidgetVisitorState_QDoubleSpinBox::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_doubleSpinBox->setValue( data.toDouble());
	}
	return false;
}

const QList<QString>& WidgetVisitorState_QDoubleSpinBox::dataelements() const
{
	static const DataElements dataElements( "");
	return dataElements;
}

bool WidgetVisitorState_QDoubleSpinBox::isRepeatingDataElement( const QString& /*name*/)
{
	return false;
}

void WidgetVisitorState_QDoubleSpinBox::setState( const QVariant& state)
{
	if (state.isValid()) m_doubleSpinBox->setValue( state.toDouble());
}

QVariant WidgetVisitorState_QDoubleSpinBox::getState() const
{
	return QVariant( m_doubleSpinBox->value());
}

