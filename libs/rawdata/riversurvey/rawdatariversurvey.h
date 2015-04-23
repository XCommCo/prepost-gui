#ifndef RAWDATARIVERSURVEY_H
#define RAWDATARIVERSURVEY_H

#include "rd_riversurvey_global.h"
#include <guicore/pre/rawdata/rawdata.h>
#include "rawdatariverpathpoint.h"
#include "rawdatarivershapeinterpolator.h"

#include <vtkSmartPointer.h>
#include <vtkPolygon.h>
#include <vtkActor.h>
#include <vtkLODActor.h>
#include <vtkDataSetMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkDoubleArray.h>
#include <vtkLabeledDataMapper.h>
#include <vtkActor2D.h>
#include <vtkStringArray.h>

#include <QPoint>
#include <QList>

#include <iriclib.h>

class vtkProperty;
class QAction;
class QPolygonF;
class RawDataRiverSurveyCrosssectionWindow;
class RawDataRiverPathPointMoveDialog;
class RawDataRiverPathPointShiftDialog;
class RawDataRiverPathPointRotateDialog;
class RawDataRiverPathPointExtensionAddDialog;
class RawDataRiverSurveySelectionChangeCommand;
class RawDataRiverPathPointMoveCommand;
class RawDataRiverPathPointShiftCenterCommand;
class RawDataRiverPathPointRotateCommand;
class RawDataRiverPathPointRenameCommand;
class RawDataRiverPathPointDeleteCommand;
class RawDataRiverPathPointAddExtensionCommand;
class RawDataRiverPathPointRemoveExtensionCommand;
class RawDataRiverSurveyTranslateCommand;
class RawDataRiverSurveyMouseRotateCommand;
class RawDataRiverSurveyMouseShiftCommand;
class RawDataRiverSurveyMouseMoveExtensionCommand;
class RawDataRiverPathPointInsertDialog;
class RawDataRiverPathPointInsertCommand;
class RawDataRiverPathPointExpandCommand;
class RawDataRiverPathPointExpandDialog;
class RawDataRiverSurveyCrosssectionEditCommand;
class RawDataRiverSurveyBackgroundGridCreateThread;
class RawDataRiverCrosssectionAltitudeMoveDialog;
class RawDataRiverSurveyCrosssectionDragEditCommand;
class RawDataRiverSurveyCrosssectionWindowGraphicsView;
class RawDataRiverSurveyDisplaySettingCommand;

class GridCreatingConditionRiverSurveyInterface;
class RawDataRiverSurveyProxy;

/// Polygon container.
/**
 * RawDataPolygon uses vtkPolygon instance as the container (m_polygon),
 * but has interfaces to use QPolygon instances for input / output.
 * QPolygon has easier api to define polygons.
 */
class RD_RIVERSURVEY_EXPORT RawDataRiverSurvey : public RawData
{
	Q_OBJECT
public:
	enum MouseEventMode {
		meNormal,
		meTranslate,
		meTranslatePrepare,
		meRotateRight,
		meRotatePrepareRight,
		meRotateLeft,
		meRotatePrepareLeft,
		meShift,
		meShiftPrepare,
		meMoveExtentionEndPointLeft,
		meMoveExtensionEndPointPrepareLeft,
		meMoveExtentionEndPointRight,
		meMoveExtensionEndPointPrepareRight,
		meExpansionRight,
		meExpansionPrepareRight,
		meExpansionLeft,
		meExpansionPrepareLeft,
		meAddingExtension,
		meInserting,

		meTranslateDialog,
		meRotateDialog,
		meShiftDialog,
		meExpansionDialog
	};
	/// Constructor
	RawDataRiverSurvey(ProjectDataItem* d, RawDataCreator* creator, SolverDefinitionGridRelatedCondition* condition);
	virtual ~RawDataRiverSurvey();
	void setupActors();
	void setupMenu();
	bool addToolBarButtons(QToolBar* /*parent*/);
	void informSelection(PreProcessorGraphicsViewInterface* v);
	void informDeselection(PreProcessorGraphicsViewInterface* v);
	void addCustomMenuItems(QMenu* menu);
	void viewOperationEnded(PreProcessorGraphicsViewInterface* /*v*/);
	void keyPressEvent(QKeyEvent* /*event*/, PreProcessorGraphicsViewInterface* /*v*/);
	void keyReleaseEvent(QKeyEvent* /*event*/, PreProcessorGraphicsViewInterface* /*v*/);
	void mouseDoubleClickEvent(QMouseEvent* /*event*/, PreProcessorGraphicsViewInterface* /*v*/);
	void mouseMoveEvent(QMouseEvent* /*event*/, PreProcessorGraphicsViewInterface* /*v*/);
	void mousePressEvent(QMouseEvent* /*event*/, PreProcessorGraphicsViewInterface* /*v*/);
	void mouseReleaseEvent(QMouseEvent* /*event*/, PreProcessorGraphicsViewInterface* /*v*/);
	void updateZDepthRangeItemCount(ZDepthRange& range);
	void assignActionZValues(const ZDepthRange& range);
	bool getValueRange(double* min, double* max);
	QDialog* propertyDialog(QWidget* parent);
	void handlePropertyDialogAccepted(QDialog* d);
	QColor doubleToColor(double d);
	void setupScalarArray();
	void updateInterpolators();
	void updateShapeData();
	void updateSelectionShapeData();
	RawDataRiverPathPoint* headPoint(){return m_headPoint;}
	vtkStructuredGrid* backgroundGrid(){return m_backgroundGrid;}
	void updateCrossectionWindows();
	void setColoredPoints(RawDataRiverPathPoint* black, RawDataRiverPathPoint* red, RawDataRiverPathPoint* blue);
	void setGridCreatingCondition(GridCreatingConditionRiverSurveyInterface* cond){m_gridCreatingCondition = cond;}
	GridCreatingConditionRiverSurveyInterface* gridCreatingCondition(){return m_gridCreatingCondition;}
	void useDivisionPointsForBackgroundGrid(bool use);
	void refreshBackgroundGrid();
	void cancelBackgroundGridUpdate();
	void toggleCrosssectionWindowsGridCreatingMode(bool gridMode);
	void informCtrlPointUpdateToCrosssectionWindows();

private slots:
	void moveSelectedPoints();
	void deleteSelectedPoints();
	void shiftSelectedPoints();
	void expandSelectedPoints();
	void rotateSelectedPoint();
	void renameSelectedPoint();
	void addLeftExtensionPoint();
	void addRightExtensionPoint();
	void removeLeftExtensionPoint();
	void removeRightExtensionPoint();
	void restoreMouseEventMode();
	void insertNewPoint();
	void addNewPoint();
	void updateBackgroundGrid();
	void openCrossSectionWindow();
	void displaySetting();
	void switchInterpolateModeToLinear();
	void switchInterpolateModeToSpline();
	RawDataProxy* getProxy();

signals:
	void dataUpdated();
protected:
	const static int LINEDIVS = 36;
	void updateMouseCursor(PreProcessorGraphicsViewInterface* v);
	void doLoadFromProjectMainFile(const QDomNode& node);
	void doSaveToProjectMainFile(QXmlStreamWriter& writer);
	void loadExternalData(const QString& /*filename*/);
	void saveExternalData(const QString& /*filename*/);
	void updateFilename(){
		m_filename = m_name;
		m_filename.append(".dat");
	}
	int iRICLibType() const {return IRIC_GEO_RIVERSURVEY;}
	void doApplyOffset(double x, double y);
	/// The pointdata, that has the positions of
	/// River center, left bank, and right bank
	vtkSmartPointer<vtkPoints> m_points;
	vtkSmartPointer<vtkPoints> m_rightBankPoints;

	vtkSmartPointer<vtkUnstructuredGrid> m_riverCenters;
	vtkSmartPointer<vtkUnstructuredGrid> m_selectedRiverCenters;
	vtkSmartPointer<vtkUnstructuredGrid> m_selectedLeftBanks;
	vtkSmartPointer<vtkUnstructuredGrid> m_selectedRightBanks;
	vtkSmartPointer<vtkUnstructuredGrid> m_rightBankPointSet;

	vtkSmartPointer<vtkStructuredGrid> m_riverCenterLine;
	vtkSmartPointer<vtkStructuredGrid> m_leftBankLine;
	vtkSmartPointer<vtkStructuredGrid> m_rightBankLine;

	vtkSmartPointer<vtkStructuredGrid> m_backgroundGrid;

	vtkSmartPointer<vtkUnstructuredGrid> m_firstAndLastCrosssections;
	vtkSmartPointer<vtkUnstructuredGrid> m_crosssections;
	vtkSmartPointer<vtkUnstructuredGrid> m_selectedCrosssections;
	vtkSmartPointer<vtkUnstructuredGrid> m_crosssectionLines;

	vtkSmartPointer<vtkUnstructuredGrid> m_blackCrosssection;
	vtkSmartPointer<vtkUnstructuredGrid> m_redCrosssection;
	vtkSmartPointer<vtkUnstructuredGrid> m_blueCrosssection;

	vtkSmartPointer<vtkActor> m_riverCenterActor;
	vtkSmartPointer<vtkActor> m_selectedRiverCenterActor;
	vtkSmartPointer<vtkActor> m_selectedLeftBankActor;
	vtkSmartPointer<vtkActor> m_selectedRightBankActor;

	vtkSmartPointer<vtkLODActor> m_riverCenterLineActor;
	vtkSmartPointer<vtkLODActor> m_leftBankLineActor;
	vtkSmartPointer<vtkLODActor> m_rightBankLineActor;

	vtkSmartPointer<vtkActor> m_firstAndLastCrosssectionsActor;
	vtkSmartPointer<vtkActor> m_crossectionsActor;
	vtkSmartPointer<vtkActor> m_selectedCrossectionsActor;

	vtkSmartPointer<vtkActor> m_blackCrossectionsActor;
	vtkSmartPointer<vtkActor> m_redCrossectionsActor;
	vtkSmartPointer<vtkActor> m_blueCrossectionsActor;

	vtkSmartPointer<vtkActor> m_backgroundActor;
	vtkSmartPointer<vtkActor> m_crosssectionLinesActor;

	vtkSmartPointer<vtkStringArray> m_labelArray;
	vtkSmartPointer<vtkLabeledDataMapper> m_labelMapper;
	vtkSmartPointer<vtkActor2D> m_labelActor;
private:
	RawDataRiverPathPoint* selectedPoint();
	void setupLine(vtkUnstructuredGrid* grid, RawDataRiverPathPoint* p);

	void allActorsOff();
	void updateSplineSolvers();
	void setupCursors();
	void setupActions();
	/// Enable or disable actions depending on the selection status.
	void updateMouseEventMode();
	void updateActionStatus();

	QPixmap m_pixmapMove;
	QPixmap m_pixmapRotate;
	QPixmap m_pixmapExpand;
	QPixmap m_pixmapShift;

	QCursor m_cursorMove;
	QCursor m_cursorRotate;
	QCursor m_cursorExpand;
	QCursor m_cursorShift;

	RawDataRiverPathPoint* m_headPoint;

	RawDataRiverSurveyBackgroundGridCreateThread* m_gridThread;

	RiverCenterLineSolver m_CenterLineSolver;
	RiverLeftBankSolver m_LeftBankSolver;
	RiverRightBankSolver m_RightBankSolver;

	QAction* m_addUpperSideAction;
	QAction* m_addLowerSideAction;
	QAction* m_moveAction;
	QAction* m_rotateAction;
	QAction* m_shiftAction;
	QAction* m_expandAction;
	QAction* m_deleteAction;
	QAction* m_renameAction;
	QAction* m_addLeftExtensionPointAction;
	QAction* m_addRightExtensionPointAction;
	QAction* m_removeLeftExtensionPointAction;
	QAction* m_removeRightExtensionPointAction;
	QAction* m_openCrossSectionWindowAction;
	QAction* m_showBackgroundAction;
	QAction* m_interpolateSplineAction;
	QAction* m_interpolateLinearAction;

	QMenu* m_rightClickingMenu;

	bool m_definingBoundingBox;
	bool m_leftButtonDown;
	bool m_showBackground;
	bool m_showLines;
	int m_opacityPercent;
	int m_crosssectionLinesScale;
	QColor m_crosssectionLinesColor;

	QPoint m_dragStartPoint;
	QPoint m_currentPoint;

	MouseEventMode m_mouseEventMode;
	Qt::KeyboardModifiers m_keyboardModifiers;
	GridCreatingConditionRiverSurveyInterface* m_gridCreatingCondition;
public:
	friend class RawDataRiverSurveyCreator;
	friend class RawDataRiverSurveyImporter;
	friend class RawDataRiverPathPointMoveDialog;
	friend class RawDataRiverPathPointShiftDialog;
	friend class RawDataRiverPathPointRotateDialog;
	friend class RawDataRiverPathPointExtensionAddDialog;
	friend class RawDataRiverPathPointInsertDialog;
	friend class RawDataRiverSurveySelectionChangeCommand;
	friend class RawDataRiverPathPointMoveCommand;
	friend class RawDataRiverPathPointShiftCenterCommand;
	friend class RawDataRiverPathPointRotateCommand;
	friend class RawDataRiverPathPointRenameCommand;
	friend class RawDataRiverPathPointDeleteCommand;
	friend class RawDataRiverPathPointAddExtensionCommand;
	friend class RawDataRiverPathPointRemoveExtensionCommand;
	friend class RawDataRiverSurveyTranslateCommand;
	friend class RawDataRiverSurveyMouseRotateCommand;
	friend class RawDataRiverSurveyMouseShiftCommand;
	friend class RawDataRiverSurveyMouseMoveExtensionCommand;
	friend class RawDataRiverPathPointInsertCommand;
	friend class RawDataRiverPathPointExpandCommand;
	friend class RawDataRiverPathPointExpandDialog;
	friend class RawDataRiverSurveyCrosssectionEditCommand;
	friend class RawDataRiverSurveyCrosssectionDragEditCommand;
	friend class RawDataRiverCrosssectionAltitudeMoveDialog;
	friend class RawDataRiverSurveyCrosssectionWindowGraphicsView;
	friend class RawDataRiverSurveyDisplaySettingCommand;

	friend class RawDataRiverSurveyProxy;
};

#endif // RAWDATAPOLYGON_H