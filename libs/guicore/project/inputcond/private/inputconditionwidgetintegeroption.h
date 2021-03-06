#ifndef INPUTCONDITIONWIDGETINTEGEROPTION_H
#define INPUTCONDITIONWIDGETINTEGEROPTION_H

#include "../inputconditionwidget.h"

class InputConditionContainerInteger;
class SolverDefinitionTranslator;

class QComboBox;

class InputConditionWidgetIntegerOption : public InputConditionWidget
{
	Q_OBJECT

public:
	InputConditionWidgetIntegerOption(QDomNode defnode, const SolverDefinitionTranslator& t, InputConditionContainerInteger* cont);

	void addTooltip(const QString& tooltip) override;

	void setDisabled(bool disable);

public slots:
	void setValue(int);

private slots:
	void informChange(int);

private:
	InputConditionContainerInteger* m_container;
	QComboBox* m_comboBox;
};

#endif // INPUTCONDITIONWIDGETINTEGEROPTION_H
