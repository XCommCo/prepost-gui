#include "../post3dwindowdatamodel.h"
#include "../post3dwindowgraphicsview.h"
#include "post3dwindowarrowgroupdataitem.h"
#include "post3dwindowcontourgroupdataitem.h"
#include "post3dwindowgridshapedataitem.h"
#include "post3dwindowgridtypedataitem.h"
#include "post3dwindownodescalargroupdataitem.h"
#include "post3dwindownodevectorparticlegroupstructureddataitem.h"
#include "post3dwindownodevectorstreamlinegroupstructureddataitem.h"
#include "post3dwindowparticlestopdataitem.h"
#include "post3dwindowzonedataitem.h"

#include <guicore/postcontainer/postsolutioninfo.h>
#include <guicore/postcontainer/postzonedatacontainer.h>
#include <guicore/project/projectdata.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <misc/xmlsupport.h>

#include <QAction>
#include <QDomNode>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalMapper>
#include <QStandardItem>
#include <QXmlStreamWriter>

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractGrid.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkStructuredGridOutlineFilter.h>
#include <vtkTriangle.h>
#include <vtkVertex.h>

#include <cgnslib.h>
#include <iriclib.h>

Post3dWindowZoneDataItem::Post3dWindowZoneDataItem(QString zoneName, int zoneNumber, Post3dWindowDataItem* parent)
	: Post3dWindowDataItem(zoneName, QIcon(":/images/iconGrid.png"), parent)
{
	m_standardItem->setCheckable(true);
	m_standardItem->setCheckState(Qt::Checked);

	m_standardItemCopy = m_standardItem->clone();

	m_zoneName = zoneName;
	m_zoneNumber = zoneNumber;

	m_isDeletable = false;

	PostZoneDataContainer* cont = dataContainer();

	m_shapeDataItem = new Post3dWindowGridShapeDataItem(this);
	m_contourGroupItem = new Post3dWindowContourGroupDataItem(this);
	m_scalarGroupDataItem = new Post3dWindowNodeScalarGroupDataItem(this);
	m_arrowGroupDataItem = new Post3dWindowArrowGroupDataItem(this);
	m_streamlineGroupDataItem = new Post3dWindowNodeVectorStreamlineGroupStructuredDataItem(this);
	m_particleGroupDataItem = new Post3dWindowNodeVectorParticleGroupStructuredDataItem(this);
	m_particlesDataItem = new Post3dWindowParticlesTopDataItem(this);

	m_childItems.append(m_shapeDataItem);
	m_childItems.append(m_contourGroupItem);
	m_childItems.append(m_scalarGroupDataItem);
	m_childItems.append(m_arrowGroupDataItem);
	m_childItems.append(m_streamlineGroupDataItem);
	m_childItems.append(m_particleGroupDataItem);
	m_childItems.append(m_particlesDataItem);
}

void Post3dWindowZoneDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomNode shapeNode = iRIC::getChildNode(node, "Shape");
	if (! shapeNode.isNull()){
		m_shapeDataItem->loadFromProjectMainFile(shapeNode);
	}
	QDomNode contourGroupNode = iRIC::getChildNode(node, "ContourGroup");
	if (! contourGroupNode.isNull() && m_contourGroupItem != 0){
		m_contourGroupItem->loadFromProjectMainFile(contourGroupNode);
	}
	QDomNode scalarGroupNode = iRIC::getChildNode(node, "ScalarGroup");
	if (! scalarGroupNode.isNull() && m_scalarGroupDataItem != 0){
		m_scalarGroupDataItem->loadFromProjectMainFile(scalarGroupNode);
	}
	QDomNode arrowGroupNode = iRIC::getChildNode(node, "ArrowGroup");
	if (! arrowGroupNode.isNull() && m_arrowGroupDataItem != 0){
		m_arrowGroupDataItem->loadFromProjectMainFile(arrowGroupNode);
	}
	QDomNode streamlineGroupNode = iRIC::getChildNode(node, "StreamlineGroup");
	if (! streamlineGroupNode.isNull() && m_streamlineGroupDataItem != 0){
		m_streamlineGroupDataItem->loadFromProjectMainFile(streamlineGroupNode);
	}
	QDomNode particleGroupNode = iRIC::getChildNode(node, "ParticleGroup");
	if (! particleGroupNode.isNull() && m_particleGroupDataItem != 0){
		m_particleGroupDataItem->loadFromProjectMainFile(particleGroupNode);
	}
	QDomNode particlesNode = iRIC::getChildNode(node, "SolverParticles");
	if (! particlesNode.isNull() && m_particlesDataItem != 0){
		m_particlesDataItem->loadFromProjectMainFile(particlesNode);
	}
}

void Post3dWindowZoneDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	writer.writeAttribute("name", m_zoneName);
	writer.writeStartElement("Shape");
	m_shapeDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();

	if (m_contourGroupItem != 0){
		writer.writeStartElement("ContourGroup");
		m_contourGroupItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	if (m_scalarGroupDataItem != 0){
		writer.writeStartElement("ScalarGroup");
		m_scalarGroupDataItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	if (m_arrowGroupDataItem != 0) {
		writer.writeStartElement("ArrowGroup");
		m_arrowGroupDataItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	if (m_streamlineGroupDataItem != 0) {
		writer.writeStartElement("StreamlineGroup");
		m_streamlineGroupDataItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	if (m_particleGroupDataItem != 0) {
		writer.writeStartElement("ParticleGroup");
		m_particleGroupDataItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	if (m_particlesDataItem != 0) {
		writer.writeStartElement("SolverParticles");
		m_particlesDataItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
}

void Post3dWindowZoneDataItem::informSelection(VTKGraphicsView *v)
{
	m_shapeDataItem->informSelection(v);
}

void Post3dWindowZoneDataItem::informDeselection(VTKGraphicsView *v)
{
	m_shapeDataItem->informDeselection(v);
}

PostZoneDataContainer* Post3dWindowZoneDataItem::dataContainer()
{
	return postSolutionInfo()->zoneContainer3D(m_zoneName);
}

void Post3dWindowZoneDataItem::update()
{
	m_shapeDataItem->update();
	m_contourGroupItem->update();
	m_scalarGroupDataItem->update();
	m_arrowGroupDataItem->update();
	m_streamlineGroupDataItem->update();
	m_particleGroupDataItem->update();
	m_particlesDataItem->update();
}