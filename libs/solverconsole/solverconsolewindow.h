#ifndef SOLVERCONSOLEWINDOW_H
#define SOLVERCONSOLEWINDOW_H

#include "solverconsole_global.h"
#include <guicore/base/clipboardoperatablewindowinterface.h>
#include <guicore/base/snapshotenabledwindowinterface.h>
#include <guicore/base/windowwithzindexinterface.h>

#include <QMainWindow>
#include <QCloseEvent>
#include <QByteArray>
#include <QProcess>
#include <QProcessEnvironment>

class QPlainTextEdit;
class iRICMainWindowInterface;
class ProjectData;
class ProjectDataItem;
class SolverConsoleWindowProjectDataItem;
class SolverConsoleWindowBackgroundColorCommand;
class QAction;

/**
	* @brief The solver console window.
	*
	* The console window monitor the progress of solver calculation by showing
	* the STDOUT of solver.
	*/
class SOLVERCONSOLEDLL_EXPORT SolverConsoleWindow :
	public QMainWindow,
	public ClipboardOperatableWindowInterface,
	public SnapshotEnabledWindowInterface,
	public WindowWithZIndexInterface
{
	Q_OBJECT

public:
	SolverConsoleWindow(iRICMainWindowInterface* parent);
	~SolverConsoleWindow();

	/// Set newly created project data.
	void setProjectData(ProjectData* d);
	SolverConsoleWindowProjectDataItem* projectDataItem();
	/// Implementation of close event.
	void closeEvent(QCloseEvent* e) override;
	/// Returns true when the solver is running.
	bool isSolverRunning();
	/// Setup window position and size to the default.
	void setupDefaultGeometry();
	/// Update Window title depending on Solver name and status
	void updateWindowTitle();
	/// Returns true to inform this window supports copy().
	bool acceptCopy() const override {return true;}
	/// Copy the selected string in solver console window.
	void copy() const override;
	/// Clear the console log.
	void clear();

	void startSolverSilently();
	void terminateSolverSilently();
	void waitForSolverFinish();

	QPixmap snapshot() override;

	QAction* exportLogAction;

public slots:
	void startSolver();
	void terminateSolver();

	void editBackgroundColor();

private slots:
	/// Handle the signal that informs that the solver has output at stderr.
	void readStderr();
	/// Handle the signal that informs that the solver has output at stdout.
	void readStdout();
	/// Handle the signal that informs that the solver has finished calculation.
	void handleSolverFinish(int, QProcess::ExitStatus);

signals:
	void solverStarted();
	void solverFinished();

private:
	/// Initialization
	void init();

	void createCancelFile();	  ///< create ".cancel" file to ask solver to stop.
	void removeCancelFile();   	///< remove ".cancel" file.
	void removeCancelOkFile();  ///< remove ".cancel_ok" file.

	void appendLogLine(const QString& line);

	QColor backgroundColor() const;
	void setBackgroundColor(const QColor& c);

	SolverConsoleWindowProjectDataItem* m_projectDataItem;
	/// ProjectData instance
	ProjectData* m_projectData;
	/// Plain text edit widget to display solver STDOUT output
	QPlainTextEdit* m_console;

	QProcess* m_process;
	bool m_solverKilled;
	bool m_destructing;

	iRICMainWindowInterface* m_iricMainWindow;

	class Impl;
	Impl* impl;

	class SetBackgroundColorCommand;

public:
	friend class SolverConsoleWindowProjectDataItem;
};

#ifdef _DEBUG
	#include "private/solverconsolewindow_impl.h"
#endif // _DEBUG

#endif // SOLVERCONSOLEWINDOW_H
