#ifndef GRIDBIRDEYEWINDOW_H
#define GRIDBIRDEYEWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <guicore/base/additionalmenuwindow.h>
#include <guicore/base/snapshotenabledwindow.h>
#include "../../datamodel/preprocessorgriddataitem.h"

class QAction;
class GridBirdEyeWindowGraphicsView;
class GridBirdEyeWindowActionManager;
class GridBirdEyeWindowDataModel;
class GridBirdEyeWindowEditBackgroundColorCommand;

/// This class represents the two-dimensional post-processing window.
class GridBirdEyeWindow :
		public QMainWindow,
		public SnapshotEnabledWindow,
		public AdditionalMenuWindow
{
	Q_OBJECT
public:
	/// Constructor
	GridBirdEyeWindow(QWidget* parent, PreProcessorGridDataItem* item);
	/// Destructor
	~GridBirdEyeWindow();
	QPixmap snapshot();
	vtkRenderWindow* getVtkRenderWindow();
	QList<QMenu*> getAdditionalMenus();
	void updateGrid();
	const QIcon& icon() const {return m_icon;}
public slots:
	void cameraFit();
	void cameraZoomIn();
	void cameraZoomOut();
	void cameraMoveLeft();
	void cameraMoveRight();
	void cameraMoveUp();
	void cameraMoveDown();
	void cameraXYPlane();
	void cameraYZPlane();
	void cameraZXPlane();
	void editBackgroundColor();
	void displaySetting();
	void editZScale();
private:
	void init(PreProcessorGridDataItem* item);
	GridBirdEyeWindowGraphicsView* m_graphicsView;
	GridBirdEyeWindowActionManager* m_actionManager;
	GridBirdEyeWindowDataModel* m_dataModel;
	QIcon m_icon;
public:
	friend class GridBirdEyeWindowActionManager;
//	friend class GridBirdEyeWindowDataModel;
	friend class GridBirdEyeWindowEditBackgroundColorCommand;
};

#endif // GRIDBIRDEYEWINDOW_H