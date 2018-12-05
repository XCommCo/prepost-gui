#ifndef SOLVERDEFINITIONLISTDIALOG_H
#define SOLVERDEFINITIONLISTDIALOG_H

#include <QDialog>

class SolverDefinitionList;

namespace Ui
{
	class SolverDefinitionListDialog;
}

/// Dialog to show the list of solvers currently installed.
class SolverDefinitionListDialog : public QDialog
{
	Q_OBJECT

public:
	SolverDefinitionListDialog(SolverDefinitionList* list, QWidget* parent = nullptr);
	~SolverDefinitionListDialog();
	/// Show detail dialog on the specified index (row)
	void showDetail(int index);

public slots:
	/// Handler for double clicking on solver definition table;
	void handleCellDoubleClick(int row, int column);
	/// Show detail dialog about the solver currently selected
	void showDetailOfCurrent();

private:
	void changeEvent(QEvent* e) override;

	void setup();

private:
	Ui::SolverDefinitionListDialog* ui;
	SolverDefinitionList* m_solverList;
};

#endif // SOLVERDEFINITIONLISTDIALOG_H
