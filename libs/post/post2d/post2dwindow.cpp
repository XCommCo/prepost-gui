#include "datamodel/post2dwindowgridtypedataitem.h"
#include "datamodel/post2dwindownodescalargroupdataitem.h"
#include "datamodel/post2dwindownodevectorparticlegroupdataitem.h"
#include "datamodel/post2dwindowrootdataitem.h"
#include "datamodel/post2dwindowzonedataitem.h"
#include "post2dobjectbrowser.h"
#include "post2dpropertybrowser.h"
#include "post2dwindow.h"
#include "post2dwindowactionmanager.h"
#include "post2dwindowdatamodel.h"
#include "post2dwindowdatamodel.h"
#include "post2dwindowgraphicsview.h"
#include "post2dwindowprojectdataitem.h"

#include <guibase/colortool.h>
#include <guibase/graphicsmisc.h>
#include <guicore/postcontainer/postsolutioninfo.h>
#include <misc/iricundostack.h>

#include <QAction>
#include <QColorDialog>
#include <QLabel>
#include <QToolBar>
#include <QUndoCommand>

#include <vtkRenderer.h>

Post2dWindow::Post2dWindow(QWidget* parent, int index, Post2dWindowProjectDataItem* pdi)
	: PostProcessorWindow(parent, index, pdi)
{
	init();
}

Post2dWindow::~Post2dWindow()
{
	delete m_dataModel;
}

void Post2dWindow::init()
{
	setWindowTitle(tr("Post-processing (2D): %1").arg(m_index));

	m_icon = QIcon(":/libs/post/post2d/images/iconVis2D.png");
	// set default central widget.
	QLabel* label = new QLabel(this);
	setCentralWidget(label);

	m_actionManager = new Post2dWindowActionManager(this);

	m_objectBrowser = new Post2dObjectBrowser(this);
	addDockWidget(Qt::LeftDockWidgetArea, m_objectBrowser);

	m_propertyBrowser = new Post2dPropertyBrowser(this);
	m_propertyBrowser->hide();
	addDockWidget(Qt::LeftDockWidgetArea, m_propertyBrowser);

	connect(postSolutionInfo(), SIGNAL(updated()), this, SLOT(update()));

	m_initialState = saveState();
}

void Post2dWindow::setupDefaultGeometry(int index)
{
	QWidget* parent = parentWidget();
	if (parent->isMaximized()){return;}
	parent->move(index * 30, index * 30);
	parent->resize(700, 500);
	m_propertyBrowser->hide();
	restoreState(m_initialState);
}

QPixmap Post2dWindow::snapshot()
{
	Post2dWindowGraphicsView* view = m_dataModel->graphicsView();
	QImage img = view->getImage();
	QPixmap pixmap = QPixmap::fromImage(img);
	if (m_isTransparent) makeBackgroundTransparent(view, pixmap);

	return pixmap;
}

vtkRenderWindow* Post2dWindow::getVtkRenderWindow()
{
	return m_dataModel->graphicsView()->mainRenderer()->GetRenderWindow();
}

QList<QMenu*> Post2dWindow::getAdditionalMenus()
{
	QList<QMenu*> menus;
	menus.append(m_actionManager->drawMenu());
	menus.append(m_actionManager->measuredDataMenu());
	return menus;
}

void Post2dWindow::cameraFit()
{
	m_dataModel->fit();
}

void Post2dWindow::cameraResetRotation()
{
	m_dataModel->resetRotation();
}

void Post2dWindow::cameraRotate90()
{
	m_dataModel->rotate90();
}

void Post2dWindow::cameraZoomIn()
{
	m_dataModel->zoomIn();
}

void Post2dWindow::cameraZoomOut()
{
	m_dataModel->zoomOut();
}

void Post2dWindow::cameraMoveLeft()
{
	m_dataModel->moveLeft();
}

void Post2dWindow::cameraMoveRight()
{
	m_dataModel->moveRight();
}

void Post2dWindow::cameraMoveUp()
{
	m_dataModel->moveUp();
}

void Post2dWindow::cameraMoveDown()
{
	m_dataModel->moveDown();
}

ObjectBrowser* Post2dWindow::objectBrowser()
{
	return m_objectBrowser;
}

class Post2dWindowEditBackgroundColorCommand : public QUndoCommand
{
public:
	Post2dWindowEditBackgroundColorCommand(double oldc[3], double newc[3], Post2dWindow* w)
		: QUndoCommand(QObject::tr("Edit Background Color"))
	{
		for (int i = 0; i < 3; ++i){
			m_oldColor[i] = oldc[i];
			m_newColor[i] = newc[i];
		}
		m_window = w;
	}
	void undo()
	{
		m_window->m_dataModel->graphicsView()->mainRenderer()->SetBackground(m_oldColor);
	}
	void redo()
	{
		m_window->m_dataModel->graphicsView()->mainRenderer()->SetBackground(m_newColor);
	}
private:
	double m_oldColor[3];
	double m_newColor[3];
	Post2dWindow* m_window;
};

void Post2dWindow::editBackgroundColor()
{
	double vtkOldColor[3];

	m_dataModel->graphicsView()->mainRenderer()->GetBackground(vtkOldColor);
	QColor oldcolor;
	iRIC::VTKColorToQColor(vtkOldColor, oldcolor);
	QColor newcolor = QColorDialog::getColor(oldcolor, this, tr("Background Color"));
	if (! newcolor.isValid()){return;}
	double vtkNewColor[3];
	iRIC::QColorToVTKColor(newcolor, vtkNewColor);
	iRICUndoStack::instance().push(new Post2dWindowEditBackgroundColorCommand(vtkOldColor, vtkNewColor, this));
}

bool Post2dWindow::exportParticles(const QString &filePrefix, int fileIndex, double time, const QString& zonename)
{
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	Post2dWindowZoneDataItem* zItem = rItem->zoneDataItem(zonename);
	Post2dWindowNodeVectorParticleGroupDataItem* pItem = zItem->particleDataItem();
	return pItem->exportParticles(filePrefix, fileIndex, time);
}

QList<QString> Post2dWindow::particleDrawingZones()
{
	QList<QString> ret;
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	QList<Post2dWindowGridTypeDataItem*> gtItems = rItem->gridTypeDataItems();
	for (int i = 0; i < gtItems.count(); ++i){
		Post2dWindowGridTypeDataItem* gtItem = gtItems.at(i);
		QList<Post2dWindowZoneDataItem*> zItems = gtItem->zoneDatas();
		for (int j = 0; j < zItems.count(); ++j){
			Post2dWindowZoneDataItem* zItem = zItems.at(j);
			Post2dWindowNodeVectorParticleGroupDataItem* pItem = zItem->particleDataItem();
			if (pItem->standardItem()->checkState() == Qt::Checked && pItem->currentSolution() != ""){
				ret.append(zItem->zoneName());
			}
		}
	}
	return ret;
}

bool Post2dWindow::exportKMLHeader(QXmlStreamWriter& writer, const QString& zonename)
{
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	Post2dWindowZoneDataItem* zItem = rItem->zoneDataItem(zonename);
	Post2dWindowNodeScalarGroupDataItem* sItem = zItem->scalarGroupDataItem();
	return sItem->exportKMLHeader(writer);
}

bool Post2dWindow::exportKMLFooter(QXmlStreamWriter& writer, const QString& zonename)
{
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	Post2dWindowZoneDataItem* zItem = rItem->zoneDataItem(zonename);
	Post2dWindowNodeScalarGroupDataItem* sItem = zItem->scalarGroupDataItem();
	return sItem->exportKMLFooter(writer);
}

bool Post2dWindow::exportKMLForTimestep(QXmlStreamWriter& writer, int index, double time, const QString& zonename)
{
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	Post2dWindowZoneDataItem* zItem = rItem->zoneDataItem(zonename);
	Post2dWindowNodeScalarGroupDataItem* sItem = zItem->scalarGroupDataItem();
	return sItem->exportKMLForTimestep(writer, index, time);
}

QList<QString> Post2dWindow::contourDrawingZones()
{
	QList<QString> ret;
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	QList<Post2dWindowGridTypeDataItem*> gtItems = rItem->gridTypeDataItems();
	for (int i = 0; i < gtItems.count(); ++i){
		Post2dWindowGridTypeDataItem* gtItem = gtItems.at(i);
		QList<Post2dWindowZoneDataItem*> zItems = gtItem->zoneDatas();
		for (int j = 0; j < zItems.count(); ++j){
			Post2dWindowZoneDataItem* zItem = zItems.at(j);
			Post2dWindowNodeScalarGroupDataItem* sItem = zItem->scalarGroupDataItem();
			if (sItem->standardItem()->checkState() == Qt::Checked && sItem->currentSolution() != ""){
				ret.append(zItem->zoneName());
			}
		}
	}
	return ret;
}

bool Post2dWindow::hasTransparentPart()
{
	Post2dWindowRootDataItem* rItem = dynamic_cast<Post2dWindowRootDataItem*>(m_dataModel->m_rootDataItem);
	return rItem->hasTransparentPart();
}

const QColor Post2dWindow::backgroundColor() const
{
	double vtkColor[3];
	m_dataModel->graphicsView()->mainRenderer()->GetBackground(vtkColor);
	QColor qColor;
	iRIC::VTKColorToQColor(vtkColor, qColor);
	return qColor;
}

void Post2dWindow::setBackgroundColor(QColor& c)
{
	double vtkColor[3];
	iRIC::QColorToVTKColor(c, vtkColor);
	m_dataModel->graphicsView()->mainRenderer()->SetBackground(vtkColor);
}