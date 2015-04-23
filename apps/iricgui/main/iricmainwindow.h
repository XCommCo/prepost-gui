#ifndef IRICMAINWINDOW_H
#define IRICMAINWINDOW_H

#include <QMainWindow>
#include <QLocale>
#include <QList>

#include <misc/versionnumber.h>
#include <guicore/base/iricmainwindowinterface.h>
#include "../continuoussnapshot/continuoussnapshotwizard.h"

class QMenu;
class QToolbar;
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QXmlStreamWriter;
class QDomNode;

class SolverConsoleWindow;
class PostProcessorWindow;
class SolverDefinitionList;
class SolverDefinition;
class SolverDefinitionAbstract;
class ProjectWorkspace;
class MousePositionWidget;
class VTK2DGraphicsView;
class GridCreatingConditionCreator;
class CoordinateSystemBuilder;

class ProjectData;
class iRICMainWindowActionManager;
class iRICMainWindowMiscDialogManager;
class PostProcessorWindowFactory;

/// This class represents the main window of iRIC 2.0.
class iRICMainWindow : public iRICMainWindowInterface
{
	Q_OBJECT
private:
	static const int MAX_RECENT_PROJECTS;
	static const int MAX_RECENT_SOLVERS;
public:
	/// Constructor
	iRICMainWindow(QWidget *parent = 0);
	/// Destructor
	~iRICMainWindow();
	/// THe PreProcessorWindow
	PreProcessorWindowInterface* preProcessorWindow(){
		return m_PreProcessorWindow;
	}
	/// The SolverConsoleWindow
	SolverConsoleWindow* solverConsoleWindow(){
		return m_SolverConsoleWindow;
	}
	const QLocale locale(){return m_locale;}
	bool isSolverRunning();
	void warnSolverRunning();
	void switchCgnsFile(const QString& newcgns);
	ProjectWorkspace* workspace(){return m_workspace;}
	const VersionNumber versionNumber() const {return m_versionNumber;}
	void setDebugMode(bool debug);
	bool isDebugMode(){return m_debugMode;}
	bool continuousSnapshotInProgress(){return m_continuousSnapshotInProgress;}
	void setContinuousSnapshotInProgress(bool prog){m_continuousSnapshotInProgress = prog;}
	const QString tmpFileName(int len = 0) const;
	AnimationControllerInterface* animationController(){return m_animationController;}
	CoordinateSystemBuilder* coordinateSystemBuilder() const {return m_coordinateSystemBuilder;}
	void initForSolverDefinition();
	void loadSubWindowsFromProjectMainFile(const QDomNode& node);
	void saveSubWindowsToProjectMainFile(QXmlStreamWriter& writer);
	QStringList containedFiles() const;
	void loadFromCgnsFile(const int fn);
	void saveToCgnsFile(const int fn);
	void closeCgnsFile();
	void toggleGridEditFlag();
	void clearResults();
	bool clearResultsIfGridIsEdited();
	void setProjectData(ProjectData *projectData);
	/// Check whether work folder is set to a good folder. If it is good returns true.
	bool checkWorkFolderWorks();
	void importCalculationResult(const QString& filename);

private:
	void parseArgs();
	/// Save Window State (normal, maximized, or minimized, windowsize)
	void saveWindowState();
	/// Restore Window State (normal, maximized, or minimized, windowsize)
	void restoreWindowState();
	/// Load plugin dlls.
	void loadPlugins();
	/// Initialize settings.
	void initSetting();
	/// The central widget
	QMdiArea* m_Center;
	// pointers to inner windows
	PreProcessorWindowInterface* m_PreProcessorWindow;
	SolverConsoleWindow* m_SolverConsoleWindow;

protected:
	void closeEvent(QCloseEvent *);
public slots:
	/// open start dialog.
	void openStartDialog();
	/// open dialog and create new project.
	void newProject();
	/// Create new project for the specified solver.
	/**
	 * This method simply call newProject(SolverDefinitionAbstract*) .
	 * This method is defined to make it possible to recieve signal
	 * QSignalMap::map(QObject*) .
	 */
	void newProject(QObject* solver){
		newProject(reinterpret_cast<SolverDefinitionAbstract*>(solver));
	}
	/// Create new project for the specified solver.
	void newProject(SolverDefinitionAbstract* solver);
	/// Open a project file
	void openProject();
	/// Open a project file
	void openProject(const QString& filename);
	/// Open a recently opened (or saved) project.
	void openRecentProject();
	/// Close the current project.
	bool closeProject();
	/// Save current project
	bool saveProject();
	/// Save current project
	bool saveProject(const QString& name, bool asFolder);
	/// Save current project. Show a dialog to select filename.
	bool saveProjectAsFile();
	/// Save current project. Show a dialog to select foldername.
	bool saveProjectAsFolder();
	/// Import Google Map images
	void importGoogleMapImages();
	/// Export current cgns file.
	void exportCurrentCgnsFile();
	/// Export Calculation result into VTK files.
	void exportCalculationResult();
	/// Export Particles into VTK files.
	void exportParticles();
	/// Export Google Earth KML for street view
	void exportStKML();
	/// Import Visualization/Graph settings.
	void importVisGraphSetting();
	/// Export Visualization/Graph settings.
	void exportVisGraphSetting();

	void checkCgnsStepsUpdate();
	void ActiveSubwindowChanged(QMdiSubWindow* newActiveWindow);
	/// Do the tasks to do when the CGNS file is switched.
	void handleCgnsSwitch();
	void focusPreProcessorWindow();
	void focusSolverConsoleWindow();

	void create2dPostWindow();
	void create2dBirdEyePostWindow();
	void create3dPostWindow();
	void createGraph2dHybridWindow();
	void createGraph2dScatteredWindow();
	void openVerificationDialog();

	void setCurrentStep(unsigned int newstep);
	/// Enter modeless dialog mode. menus, toolbars are disabled.
	void enterModelessDialogMode();
	/// Exit modeless dialog mode. Menus, toolbars becomes usable.
	void exitModelessDialogMode();
	void setupRecentProjectsMenu();
	/// Clear calculation result in the current CGNS file.
	void clearCalculationResult();
	void tileSubWindows();

signals:
	void allPostProcessorsUpdated();
private slots:
	void showProjectPropertyDialog();
	/// Cut.
	void cut();
	/// Copy.
	void copy();
	/// Paste.
	void paste();
	/// Take snapshot and save to Raster image (like bmp, jpeg, png).
	void snapshot();
	/// Take snapshot and save to SVG image.
	void snapshotSvg();
	/// Take continuous snapshot and save to Raster image.
	void continuousSnapshot();
	/// Import Calculation result from CGNS file.
	void importCalculationResult();
	/// Import Measured data from CSV file.
	void importMeasuredData();

	void showPreferenceDialog();
	void updatePostActionStatus();

private:
	void updateWindowZIndices();
	void reflectWindowZIndices();
	/// Setup PreProcessor and SolverConsoleWindow.
	void setupBasicSubWindows();
	/// Setup Central Widget, that is, MDIArea.
	void setupCentralWidget();
	void setupForNewProjectData();
	/// Update window title depending on the current project data
	void updateWindowTitle();
//	void newDocument();
	void setupAnimationToolbar();
	void setupStatusBar();
	/// Update Recent Projects list
	void updateRecentProjects(const QString& filename);
	void removeFromRecentProjects(const QString& foldername);
	void updateRecentSolvers(const QString& filename);
	void removeFromRecentSolvers(const QString& foldername);
	void loadMetaData();
	void handleWizardAccepted(ContinuousSnapshotWizard* wizard);
	void saveContinuousSnapshot(ContinuousSnapshotWizard* wizard, QXmlStreamWriter* writer = 0);
	void addKMLElement(int time, QString url, double north, double south, double west, double east, double angle, QXmlStreamWriter* writer);
	QString timeString(int time);

	MousePositionWidget* m_mousePositionWidget;
	/// Action manager;
	iRICMainWindowActionManager* m_actionManager;

	/// Dialog manager
	iRICMainWindowMiscDialogManager* m_miscDialogManager;

	QMenu* m_menuWindow;
	QMenu* m_menuHelp;

	/// Workspace
	ProjectWorkspace* m_workspace;
	/// Project data
	ProjectData* m_projectData;
	/// Locale of current user
	QLocale m_locale;
	/// List of solver definitions
	SolverDefinitionList* m_solverDefinitionList;

	CoordinateSystemBuilder* m_coordinateSystemBuilder;

	/// List of grid creating programs
	QList<GridCreatingConditionCreator*> m_gridCreatingConditionCreators;
	/// The definition information of the solver currently used
	SolverDefinition* m_solverDefinition;

	AnimationControllerInterface* m_animationController;

	// This value is true only when opening a project file.
	bool m_isOpening;
	// This value is true only when saving a project file.
	bool m_isSaving;
	VersionNumber m_versionNumber;

	// for continuous snapshot
	ContinuousSnapshotWizard::Output m_output;
	ContinuousSnapshotWizard::Layout m_layout;
	bool m_transparent;

	QString m_directory;
	QString m_extension;
	int m_suffixLength;

	bool m_outputMovie;
	int m_movieLengthMode;
	int m_movieLength;
	int m_framesPerSecond;
	int m_movieProfile;

	int m_start;
	int m_stop;
	int m_samplingRate;

	bool m_continuousSnapshotInProgress;

	bool m_googleEarth;
	double m_leftLatitude;
	double m_leftLongitude;
	double m_rightLatitude;
	double m_rightLongitude;
	QPoint bottomLeft;
	QPoint bottomRight;
	QString m_kmlFilename;
	double m_angle;
	double m_north;
	double m_south;
	double m_east;
	double m_west;

	PostProcessorWindowFactory* m_postWindowFactory;

	bool m_debugMode;
public:
	friend class ProjectData;
	friend class iRICMainWindowActionManager;
	friend class iRICMainWindowMiscDialogManager;
};

#endif // IRICMAINWINDOW_H