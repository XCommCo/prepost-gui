#include "preprocessorwindow.h"

#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QLabel>
#include <QPen>
#include <QBrush>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QUndoCommand>
#include <QColorDialog>
#include <QFile>
#include <QTextStream>
#include <vtkRenderer.h>

#include <guicore/base/iricmainwindowinterface.h>
#include <guibase/irictoolbar.h>
#include <misc/iricundostack.h>
#include <guibase/colortool.h>
#include <guicore/project/projectdata.h>
#include "preobjectbrowser.h"
#include "prepropertybrowser.h"
#include "preprocessorwindowprojectdataitem.h"
#include "preprocessorwindowactionmanager.h"
#include "preprocessordatamodel.h"
#include "preobjectbrowserview.h"
#include "preprocessorgraphicsview.h"
#include <guicore/project/projectmainfile.h>
#include <guicore/pre/grid/grid.h>
#include <guicore/pre/grid/structured2dgrid.h>
#include <guibase/graphicsmisc.h>
#include "datamodel/preprocessorrootdataitem.h"
#include "datamodel/preprocessorgridtypedataitem.h"
#include "datamodel/preprocessorgridandgridcreatingconditiondataitem.h"
#include "datamodel/preprocessorgriddataitem.h"
#include <guicore/project/backgroundimageinfo.h>
#include <guicore/pre/base/preprocessorgriddataiteminterface.h>
#include <guicore/pre/base/preprocessorgraphicsviewinterface.h>

PreProcessorWindow::PreProcessorWindow(QWidget* parent)
	: PreProcessorWindowInterface(parent)
{
	init();
}

void PreProcessorWindow::init(){
	setMinimumSize(480, 360);
	setWindowTitle(tr("Pre-processing Window"));

	// setup graphics view
	m_graphicsView = new PreProcessorGraphicsView(this);
	setCentralWidget(m_graphicsView);
	connect(m_graphicsView, SIGNAL(worldPositionChangedForStatusBar(QVector2D)), this, SIGNAL(worldPositionChangedForStatusBar(QVector2D)));

	m_dataModel = 0;
	m_actionManager = new PreProcessorWindowActionManager(this);
	m_objectBrowser = new PreObjectBrowser(this);
	addDockWidget(Qt::LeftDockWidgetArea, m_objectBrowser);
	m_propertyBrowser = new PrePropertyBrowser(this);
	m_propertyBrowser->hide();
	addDockWidget(Qt::LeftDockWidgetArea, m_propertyBrowser);

	m_initialState = saveState();

	m_isFirstHiding = true;
	m_isLastHiding = false;
}

void PreProcessorWindow::setProjectData(ProjectData* d)
{
	m_projectDataItem = new PreProcessorWindowProjectDataItem(this, d->mainfile());
}

PreProcessorWindowProjectDataItem* PreProcessorWindow::projectDataItem()
{
	return m_projectDataItem;
}

void PreProcessorWindow::setupDefaultGeometry()
{
	QWidget* parent = parentWidget();
	parent->move(0, 0);
	if (! parent->isMaximized()){
		parent->resize(700, 500);
	}
	m_propertyBrowser->hide();
	restoreState(m_initialState);
}

void PreProcessorWindow::showCalcConditionDialog()
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->showCalcConditionDialog();
}

bool PreProcessorWindow::importInputCondition(const QString& filename)
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	return model->importInputCondition(filename);
}

bool PreProcessorWindow::exportInputCondition(const QString& filename)
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	return model->exportInputCondition(filename);
}

bool PreProcessorWindow::isInputConditionSet()
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	return model->isInputConditionSet();
}

PreProcessorWindow::GridState PreProcessorWindow::checkGridState()
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	PreProcessorRootDataItem* root = dynamic_cast<PreProcessorRootDataItem*>(model->rootDataItem());
	QList<PreProcessorGridTypeDataItem*> gridTypeDataItems = root->gridTypeDataItems();
	QList<PreProcessorGridTypeDataItem*>::iterator it;
	bool ngexists = false;
	bool okexists = false;
	for (it = gridTypeDataItems.begin(); it != gridTypeDataItems.end(); ++it){
		PreProcessorGridTypeDataItem* typeItem = (*it);
		QList<PreProcessorGridAndGridCreatingConditionDataItemInterface*> conds = typeItem->conditions();
		QList<PreProcessorGridAndGridCreatingConditionDataItemInterface*>::iterator it2;
		for (it2 = conds.begin(); it2 != conds.end(); ++it2){
			PreProcessorGridAndGridCreatingConditionDataItemInterface* cond = *it2;
			PreProcessorGridDataItemInterface* g = cond->gridDataItem();
			if (g->grid() == 0){
				ngexists = true;
			}else{
				okexists = true;
			}
		}
	}
	if (okexists){
		if (ngexists){
			return PreProcessorWindow::grPartiallyUnfinished;
		}else{
			return PreProcessorWindow::grFinished;
		}
	}else{
		if (ngexists){
			return PreProcessorWindow::grUnfinished;
		}else{
			// there is no grid item.
			// we do not need to set up grids.
			return PreProcessorWindow::grFinished;
		}
	}
}

const QString PreProcessorWindow::checkGrid(bool detail)
{
	QString ret;
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	PreProcessorRootDataItem* root = dynamic_cast<PreProcessorRootDataItem*>(model->rootDataItem());
	QList<PreProcessorGridTypeDataItem*> gridTypeDataItems = root->gridTypeDataItems();
	QList<PreProcessorGridTypeDataItem*>::iterator it;
	QList<QString> gridNames;
	QList<QString> gridMessages;
	QFile logFile(model->projectData()->absoluteFileName("gridcheck.txt"));
	logFile.open(QFile::WriteOnly | QFile::Text);
	QTextStream logStream(&logFile);
	for (it = gridTypeDataItems.begin(); it != gridTypeDataItems.end(); ++it){
		PreProcessorGridTypeDataItem* typeItem = (*it);
		QList<PreProcessorGridAndGridCreatingConditionDataItemInterface*> conds = typeItem->conditions();
		QList<PreProcessorGridAndGridCreatingConditionDataItemInterface*>::iterator it2;
		for (it2 = conds.begin(); it2 != conds.end(); ++it2){
			PreProcessorGridAndGridCreatingConditionDataItemInterface* cond = *it2;
			PreProcessorGridDataItemInterface* g = cond->gridDataItem();
			Grid* grid = g->grid();
			if (grid == 0){
				gridNames.append(cond->caption());
				QString msg = "<ul>";
				msg += "<li>" + tr("Grid is not created or imported yet.") + "</li>";
				msg += "</ul>";
				gridMessages.append(msg);
				continue;
			}
			if (detail){
				logStream << tr("Checking grid %1 ...").arg(cond->caption()) << endl;
				QStringList messages = grid->checkShape(logStream);
				if (messages.count() > 0) {
					gridNames.append(cond->caption());
					QString msg;
					msg = "<ul>";
					for (int i = 0; i < messages.count(); ++i){
						QString tmpmsg = messages.at(i);
						msg += "<li>" + tmpmsg + "</li>";
					}
					msg += "</ul>";
					gridMessages.append(msg);
				}
			}
		}
	}
	logFile.close();
	if (gridNames.count() == 0){
		return ret;
	} else if (gridNames.count() == 1){
		ret = gridMessages.at(0);
		return ret;
	} else {
		ret = "<ul>";
		for (int i = 0; i < gridNames.count(); ++i) {
			ret += "<li>";
			gridNames.at(i);
			gridMessages.at(i);
			ret += "</li>";
		}
		ret += "</ul>";
		return ret;
	}
}

QPixmap PreProcessorWindow::snapshot()
{
	PreProcessorGraphicsViewInterface* view = m_dataModel->graphicsView();
	QImage img = view->getImage();
	QPixmap pixmap = QPixmap::fromImage(img);
	if (m_isTransparent) makeBackgroundTransparent(view, pixmap);

	return pixmap;
}

vtkRenderWindow* PreProcessorWindow::getVtkRenderWindow()
{
	return m_graphicsView->mainRenderer()->GetRenderWindow();
}

QList<QMenu*> PreProcessorWindow::getAdditionalMenus()
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	QList<QMenu*> menus;
	if (m_dataModel != 0){
		menus.append(model->additionalMenus());
	}
	menus.append(m_actionManager->calcCondMenu());
	return menus;
}

QToolBar* PreProcessorWindow::getAdditionalToolBar()
{
	if (m_dataModel == 0){return 0;}
	return m_dataModel->operationToolBar();
}

void PreProcessorWindow::cameraFit()
{
	m_dataModel->fit();
}

void PreProcessorWindow::cameraResetRotation()
{
	m_dataModel->resetRotation();
}

void PreProcessorWindow::cameraRotate90()
{
	m_dataModel->rotate90();
}

void PreProcessorWindow::cameraZoomIn()
{
	m_dataModel->zoomIn();
}

void PreProcessorWindow::cameraZoomOut()
{
	m_dataModel->zoomOut();
}

void PreProcessorWindow::cameraMoveLeft()
{
	m_dataModel->moveLeft();
}

void PreProcessorWindow::cameraMoveRight()
{
	m_dataModel->moveRight();
}

void PreProcessorWindow::cameraMoveUp()
{
	m_dataModel->moveUp();
}

void PreProcessorWindow::cameraMoveDown()
{
	m_dataModel->moveDown();
}

ObjectBrowser* PreProcessorWindow::objectBrowser()
{
	return m_objectBrowser;
}

class PreProcessorWindowEditBackgroundColorCommand : public QUndoCommand
{
public:
	PreProcessorWindowEditBackgroundColorCommand(const QColor& newcolor, PreProcessorWindow* w)
		: QUndoCommand(QObject::tr("Edit Background Color"))
	{
		m_newColor = newcolor;
		m_oldColor = w->backgroundColor();
		m_window = w;
	}
	void undo()
	{
		m_window->setBackgroundColor(m_oldColor);
		m_window->m_graphicsView->update();
	}
	void redo()
	{
		m_window->setBackgroundColor(m_newColor);
		m_window->m_graphicsView->update();
	}
private:
	QColor m_oldColor;
	QColor m_newColor;
	PreProcessorWindow* m_window;
};

void PreProcessorWindow::editBackgroundColor()
{
	QColor oldcolor = backgroundColor();
	QColor newcolor = QColorDialog::getColor(oldcolor, this, tr("Background Color"));
	if (! newcolor.isValid()){return;}
	iRICUndoStack::instance().push(new PreProcessorWindowEditBackgroundColorCommand(newcolor, this));
}

class PreProcessorWindowCloseCommand : public QUndoCommand
{
public:
	PreProcessorWindowCloseCommand(PreProcessorWindow* pre)
		: QUndoCommand(QObject::tr("Close PreProcessor Window"))
	{
		m_preWindow = pre;
	}
	void undo()
	{
		m_preWindow->parentWidget()->show();
	}
	void redo()
	{
		m_preWindow->parentWidget()->hide();
	}
private:
	PreProcessorWindow* m_preWindow;
};

void PreProcessorWindow::closeEvent(QCloseEvent* e){
	iRICUndoStack::instance().push(new PreProcessorWindowCloseCommand(this));
//	parentWidget()->hide();
	e->ignore();
}

void PreProcessorWindow::showEvent(QShowEvent * /*e*/)
{
	ProjectMainFile* mainfile = dynamic_cast<ProjectMainFile*>(m_projectDataItem->parent());
	if (mainfile == 0){return;}
	mainfile->addRenderer(m_dataModel->graphicsView()->mainRenderer());
}

void PreProcessorWindow::hideEvent(QHideEvent * /*e*/)
{
	if (m_isFirstHiding){
		m_isFirstHiding = false;
		return;
	}
	if (m_isLastHiding){
		m_isLastHiding = false;
		return;
	}
	if (m_projectDataItem == 0){return;}
	ProjectMainFile* mainfile = dynamic_cast<ProjectMainFile*>(m_projectDataItem->parent());
	if (mainfile == 0){return;}
	mainfile->removeRenderer(m_dataModel->graphicsView()->mainRenderer());
}

void PreProcessorWindow::handleAdditionalMenusUpdate(const QList<QMenu*>& m)
{
	QList<QMenu*> menus;
	menus.append(m);
	menus.append(m_actionManager->calcCondMenu());
	emit additionalMenusUpdated(menus);
}

void PreProcessorWindow::addGridImportMenu(QMenu* menu)
{
	if (m_dataModel == 0){return;}
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->addGridImportMenu(menu);
}

void PreProcessorWindow::addGridExportMenu(QMenu* menu)
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->addGridExportMenu(menu);
}

void PreProcessorWindow::setupRawDataImportMenu()
{
	if (m_dataModel == 0){return;}
	QMenu* menu = dynamic_cast<QMenu*>(sender());
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->setupRawDataImportMenu(menu);
}

void PreProcessorWindow::setupRawDataExportMenu()
{
	QMenu* menu = dynamic_cast<QMenu*>(sender());
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->setupRawDataExportMenu(menu);
}

void PreProcessorWindow::setupHydraulicDataImportMenu()
{
	if (m_dataModel == 0){return;}
	QMenu* menu = dynamic_cast<QMenu*>(sender());
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->setupHydraulicDataImportMenu(menu);
}


void PreProcessorWindow::informUnfocusRiverCrosssectionWindows()
{
	if (m_dataModel == 0){return;}
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	model->informUnfocusRiverCrosssectionWindows();
}

bool PreProcessorWindow::isSetupCorrectly() const
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	return model->isSetupCorrectly();
}

const QColor PreProcessorWindow::backgroundColor() const
{
	double vtkColor[3];
	m_graphicsView->mainRenderer()->GetBackground(vtkColor);
	QColor qColor;
	iRIC::VTKColorToQColor(vtkColor, qColor);
	return qColor;
}

void PreProcessorWindow::setBackgroundColor(QColor& c)
{
	double vtkColor[3];
	iRIC::QColorToVTKColor(c, vtkColor);
	m_graphicsView->mainRenderer()->SetBackground(vtkColor);
}

bool PreProcessorWindow::checkMappingStatus()
{
	PreProcessorDataModel* model = dynamic_cast<PreProcessorDataModel*> (m_dataModel);
	return model->checkMappingStatus();
}