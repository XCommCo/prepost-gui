#ifndef GRIDCREATINGCONDITIONRIVERSURVEY15DPOINTADDDIALOG_H
#define GRIDCREATINGCONDITIONRIVERSURVEY15DPOINTADDDIALOG_H

#include <rawdata/riversurvey/rawdatariverpathpoint.h>

#include <QDialog>

class GridCreatingConditionRiverSurvey15D;

class QAbstractButton;

namespace Ui {
	class GridCreatingConditionRiverSurvey15DPointAddDialog;
}

class GridCreatingConditionRiverSurvey15DPointAddDialog : public QDialog {
	Q_OBJECT
public:
	GridCreatingConditionRiverSurvey15DPointAddDialog(GridCreatingConditionRiverSurvey15D* cond, QWidget *parent = 0);
	~GridCreatingConditionRiverSurvey15DPointAddDialog();

public slots:
	void accept();
	void reject();

private slots:
	void handleButtonClick(QAbstractButton* button);

private:
	bool m_applied;
	void apply();
	RawDataRiverPathPoint::CtrlPointsAddMethod buildMethod();
	GridCreatingConditionRiverSurvey15D* m_condition;

	Ui::GridCreatingConditionRiverSurvey15DPointAddDialog *ui;
};

#endif // GRIDCREATINGCONDITIONRIVERSURVEY15DPOINTADDDIALOG_H