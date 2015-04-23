#ifndef POST3DWINDOW_H
#define POST3DWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <guicore/post/postprocessorwindow.h>
#include <guicore/base/additionalmenuwindow.h>
#include <guicore/base/windowwithobjectbrowser.h>
#include <guicore/base/particleexportwindow.h>

class QAction;
class QToolBar;
class Post3dObjectBrowser;
class Post3dWindowDataModel;
class Post3dWindowProjectDataItem;
class Post3dWindowActionManager;
class Post3dWindowEditBackgroundColorCommand;

/// This class represents the two-dimensional post-processing window.
class Post3dWindow :
		public PostProcessorWindow,
		public AdditionalMenuWindow,
		public WindowWithObjectBrowser,
		public ParticleExportWindow
{
	Q_OBJECT
public:
	/// Constructor
	Post3dWindow(QWidget* parent, int index, Post3dWindowProjectDataItem* pdi);
	/// Destructor
	~Post3dWindow();
	/// Informed that CGNS file is switched.
	void handleCgnsSwitch(){}
	/// switch to the new index.
	void changeIndex(uint /*newindex*/){}
	QPixmap snapshot();
	vtkRenderWindow* getVtkRenderWindow();
	QList<QMenu*> getAdditionalMenus();
	ObjectBrowser* objectBrowser();
	int index(){return m_index;}
	bool exportParticles(const QString &filename, int fileIndex, double time, const QString& zonename);
	QList<QString> particleDrawingZones();
	bool hasTransparentPart();
public slots:
	void cameraFit();
	void cameraResetRotation();
	void cameraRotate90();
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
	void editZScale();
private:
	void init();
	/// Background color
	const QColor backgroundColor() const;
	/// Set background color;
	void setBackgroundColor(QColor& c);
	void setupDefaultGeometry(int index);
	Post3dObjectBrowser* m_objectBrowser;
	Post3dWindowDataModel* m_dataModel;
	Post3dWindowActionManager* m_actionManager;
	QByteArray m_initialState;
public:
	friend class Post3dWindowProjectDataItem;
	friend class Post3dWindowActionManager;
	friend class Post3dWindowDataModel;
	friend class Post3dWindowEditBackgroundColorCommand;
};

#endif // POST3DWINDOW_H