#ifndef POST2DWINDOWZONEDATAITEM_H
#define POST2DWINDOWZONEDATAITEM_H

#include "../post2dwindowdataitem.h"
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkDataSetMapper.h>

class QAction;
class QSignalMapper;

class Post2dWindowGridShapeDataItem;
class Post2dWindowNodeScalarGroupDataItem;
class Post2dWindowNodeVectorArrowGroupDataItem;
class Post2dWindowNodeVectorStreamlineGroupDataItem;
class Post2dWindowNodeVectorParticleGroupDataItem;
class Post2dWindowCellFlagGroupDataItem;
class Post2dWindowParticlesTopDataItem;
class PostZoneDataContainer;

class Post2dWindowZoneDataItem : public Post2dWindowDataItem
{
	Q_OBJECT
public:
	/// Constructor
	Post2dWindowZoneDataItem(QString zoneName, int zoneNumber, Post2dWindowDataItem* parent);
	~Post2dWindowZoneDataItem();
	void addCustomMenuItems(QMenu* menu);

	// Standard mouse event handlers
	void informSelection(VTKGraphicsView *v);
	void informDeselection(VTKGraphicsView *v);

	void informgridRelatedConditionChange(const QString& name);
	void updateZDepthRangeItemCount();

	PostZoneDataContainer* dataContainer();
	vtkPolyData* filteredData() const {return m_filteredData;}
	bool isMasked() const {return m_isMasked;}
	int zoneNumber(){return m_zoneNumber;}
	QString zoneName(){return m_zoneName;}
	void update(bool noparticle = false);
	Post2dWindowGridShapeDataItem* gridShapeDataItem(){return m_shapeDataItem;}
	Post2dWindowNodeScalarGroupDataItem* scalarGroupDataItem(){return m_scalarGroupDataItem;}
	Post2dWindowNodeVectorArrowGroupDataItem* arrowGroupDataItem(){return m_arrowGroupDataItem;}
	Post2dWindowNodeVectorStreamlineGroupDataItem* streamlineDataItem(){return m_streamlineGroupDataItem;}
	Post2dWindowNodeVectorParticleGroupDataItem* particleDataItem(){return m_particleGroupDataItem;}
	Post2dWindowCellFlagGroupDataItem* cellFlagGroupDataItem(){return m_cellFlagGroupDataItem;}
	Post2dWindowParticlesTopDataItem* particlesDataItem(){return m_particlesDataItem;}

	void initNodeAttributeBrowser();
	void clearNodeAttributeBrowser();
	void fixNodeAttributeBrowser(const QPoint& p, VTKGraphicsView* v);
	void updateNodeAttributeBrowser(const QPoint& p, VTKGraphicsView* v);
	void initCellAttributeBrowser();
	void clearCellAttributeBrowser();
	void fixCellAttributeBrowser(const QPoint& p, VTKGraphicsView* v);
	void updateCellAttributeBrowser(const QPoint& p, VTKGraphicsView* v);
	void updateRegionPolyData();

	QAction* showNodeAttributeBrowserAction() const {return m_showNodeAttributeBrowserAction;}
	QAction* showCellAttributeBrowserAction() const {return m_showCellAttributeBrowserAction;}

public slots:
	void showNodeAttributeBrowser();
	void showCellAttributeBrowser();

protected:
	void assignActionZValues(const ZDepthRange& range);
	void doLoadFromProjectMainFile(const QDomNode& node);
	void doSaveToProjectMainFile(QXmlStreamWriter& writer);
	virtual void doViewOperationEndedGlobal(VTKGraphicsView* v);

	Post2dWindowGridShapeDataItem* m_shapeDataItem;
	Post2dWindowNodeScalarGroupDataItem* m_scalarGroupDataItem;
	Post2dWindowNodeVectorArrowGroupDataItem* m_arrowGroupDataItem;
	Post2dWindowNodeVectorStreamlineGroupDataItem* m_streamlineGroupDataItem;
	Post2dWindowNodeVectorParticleGroupDataItem* m_particleGroupDataItem;
	Post2dWindowCellFlagGroupDataItem* m_cellFlagGroupDataItem;
	Post2dWindowParticlesTopDataItem* m_particlesDataItem;

private:
	void setupActors();

	vtkIdType findVertex(const QPoint& p, VTKGraphicsView* v);
	vtkIdType findCell(const QPoint& p, VTKGraphicsView* v);
	void updateNodeAttributeBrowser(vtkIdType vid, double x, double y, VTKGraphicsView* v);
	void updateCellAttributeBrowser(vtkIdType cellid, VTKGraphicsView* v);

	vtkSmartPointer<vtkPolyData> m_regionPolyData;
	vtkSmartPointer<vtkPolyDataMapper> m_regionMapper;
	vtkSmartPointer<vtkActor> m_regionActor;

	vtkSmartPointer<vtkPolyData> m_filteredData;

	QAction* m_showNodeAttributeBrowserAction;
	QAction* m_showCellAttributeBrowserAction;

	QString m_zoneName;
	int m_zoneNumber;
	bool m_attributeBrowserFixed;
	bool m_isMasked;
};

#endif // POST2DWINDOWZONEDATAITEM_H