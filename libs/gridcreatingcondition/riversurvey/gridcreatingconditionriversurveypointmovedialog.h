#ifndef GRIDCREATINGCONDITIONRIVERSURVEYPOINTMOVEDIALOG_H
#define GRIDCREATINGCONDITIONRIVERSURVEYPOINTMOVEDIALOG_H

#include <QDialog>
#include <QAbstractButton>


#include <geodata/riversurvey/geodatariverpathpoint.h>

class GridCreatingConditionRiverSurvey;

namespace Ui
{
	class GridCreatingConditionRiverSurveyPointMoveDialog;
}

class GridCreatingConditionRiverSurveyPointMoveDialog : public QDialog
{
	Q_OBJECT

public:
	explicit GridCreatingConditionRiverSurveyPointMoveDialog(
		GridCreatingConditionRiverSurvey* gc,
		double lowerLimit, double upperLimit,
		QWidget* parent = nullptr);
	~GridCreatingConditionRiverSurveyPointMoveDialog();

	void apply();
	void accept() override;
	void reject() override;

private:
	void setSValue();
	void doOffset(bool preview = false);
	void doReset();

private:
	Ui::GridCreatingConditionRiverSurveyPointMoveDialog* ui;

	GridCreatingConditionRiverSurvey* m_condition;
	double m_LowerLimit;
	double m_UpperLimit;
	double m_SValue;

	bool m_applied;

private slots:
	void on_buttonBox_clicked(QAbstractButton* button);
};

#endif // GRIDCREATINGCONDITIONRIVERSURVEYPOINTMOVEDIALOG_H
