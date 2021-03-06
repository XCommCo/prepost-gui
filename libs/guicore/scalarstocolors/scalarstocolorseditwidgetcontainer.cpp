#include "scalarstocolorseditwidget.h"
#include "scalarstocolorseditwidgetcontainer.h"

#include <QVBoxLayout>
#include <QWidget>

ScalarsToColorsEditWidgetContainer::ScalarsToColorsEditWidgetContainer(QWidget* parent) :
	QWidget {parent},
	m_widget {nullptr}
{}

ScalarsToColorsEditWidget* ScalarsToColorsEditWidgetContainer::widget() const
{
	return m_widget;
}

void ScalarsToColorsEditWidgetContainer::setWidget(ScalarsToColorsEditWidget* w)
{
	m_widget = w;
	QVBoxLayout* l = new QVBoxLayout();
	l->setMargin(0);
	l->addWidget(w);
	setLayout(l);
}

void ScalarsToColorsEditWidgetContainer::save()
{
	m_widget->save();
}

QSize ScalarsToColorsEditWidgetContainer::sizeHint() const
{
	return m_widget->sizeHint();
}
