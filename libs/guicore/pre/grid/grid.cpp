#include "grid.h"
#include "../../project/projectcgnsfile.h"
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include "../../solverdef/solverdefinitiontranslator.h"
#include <misc/xmlsupport.h>
#include <misc/stringtool.h>

#include "../gridcond/base/gridrelatedconditioncontainer.h"
#include "../../solverdef/solverdefinitiongridrelatedcondition.h"

#include <QDomElement>

#include <vtkGeometryFilter.h>
#include <vtkMaskPoints.h>
#include <vtkMaskPolyData.h>

#include <cgnslib.h>
#include <iriclib.h>

const int Grid::MAX_DRAWCELLCOUNT = 40000;
const int Grid::MAX_DRAWINDEXCOUNT = 300;
const char Grid::LABEL_NAME[] = "_LABEL";

Grid::Grid(const QString& zonename, SolverDefinitionGridType::GridType type, ProjectDataItem* parent)
	: ProjectDataItem(parent)
{
	initPointers();
	m_gridType = type;
	m_zoneName = zonename;
}

Grid::Grid(SolverDefinitionGridType::GridType type, ProjectDataItem* parent)
	: ProjectDataItem(parent)
{
	initPointers();
	m_gridType = type;
}

void Grid::initPointers()
{
//	m_importerFactory = 0;
//	m_exporterFactory = 0;
	m_vtkGrid = 0;
	m_isModified = false;
	m_isMasked = false;
}

Grid::~Grid()
{
	if (m_vtkGrid != 0){
		m_vtkGrid->Delete();
	}
}

void Grid::addGridRelatedCondition(GridRelatedConditionContainer* cond)
{
	m_gridRelatedConditions.append(cond);
	m_gridRelatedConditionNameMap.insert(cond->name(), cond);
}

int Grid::zoneId(const QString& zonename, int fn, int B, cgsize_t sizes[9])
{
	char zn[ProjectCgnsFile::BUFFERLEN];
	// get the number of zones;
	int nzones;
	cg_nzones(fn, B, &nzones);
	for (int zoneid = 1; zoneid <= nzones; ++zoneid){
		cg_zone_read(fn, B, zoneid, zn, sizes);
		if (zonename == zn){
			return zoneid;
		}
	}
	return 0;
}

void Grid::loadFromCgnsFile(const int fn){
	int B;
	// goto Base.
	cg_iRIC_GotoBase(fn, &B);

	cgsize_t sizes[9];
	int zoneid = zoneId(m_zoneName, fn, B, sizes);
	if (zoneid == 0){
		// Error. No corresponding zone found.
		return;
	}
	loadFromCgnsFile(fn, B, zoneid);
	cg_iRIC_Set_ZoneId(zoneid);
	m_isModified = false;
}

void Grid::saveToCgnsFile(const int fn)
{
	// if not modified, do nothing.
	if (! m_isModified){return;}

	int B;
	// goto Base.
	cg_iRIC_GotoBase(fn, &B);
	saveToCgnsFile(fn, B, const_cast<char*>(iRIC::toStr(m_zoneName).c_str()));
	cgsize_t size[9];
	int Z = zoneId(m_zoneName, fn, B, size);
	cg_ziter_write(fn, B, Z, "ZoneIterativeData");
	cg_iRIC_Set_ZoneId(Z);
	m_isModified = false;
}

bool Grid::loadGridRelatedConditions(int fn, int B, int Z)
{
	// Grid coordinates are loaded.
	// Next, grid related condition data is loaded.
	QList<GridRelatedConditionContainer*>::iterator it;
	bool allok = true;
	for (it = m_gridRelatedConditions.begin(); it != m_gridRelatedConditions.end(); ++it){
		(*it)->allocate();
		bool ret = (*it)->loadFromCgnsFile(fn, B, Z);
		allok = allok && ret;
	}
	return allok;
}

bool Grid::saveGridRelatedConditions(int fn, int B, int Z)
{
	// Grid coordinates are saved.
	// Next grid related condition data is saved.
	// Create "GridConditions" node under the zone node.
	cg_goto(fn, B, "Zone_t", Z, "end");
	cg_user_data_write("GridConditions");

	QList<GridRelatedConditionContainer*>::iterator it;
	bool allok = true;
	for (it = m_gridRelatedConditions.begin(); it != m_gridRelatedConditions.end(); ++it){
		bool ret = (*it)->saveToCgnsFile(fn, B, Z);
		allok = allok && ret;
	}
	return allok;
}

bool Grid::isCustomModified()
{
	bool modified = false;
	for (int i = 0; i < m_gridRelatedConditions.count(); ++i){
		GridRelatedConditionContainer* c = m_gridRelatedConditions[i];
		modified = modified || c->isCustomModified();
	}
	return modified;
}
void Grid::setCustomModified(bool modified)
{
	for (int i = 0; i < m_gridRelatedConditions.count(); ++i){
		GridRelatedConditionContainer* c = m_gridRelatedConditions[i];
		c->setCustomModified(modified);
	}
}

const QStringList Grid::checkShape(QTextStream& /*stream*/)
{
	QStringList ret;
	return ret;
}

void Grid::updateSimplifiedGrid(double xmin, double xmax, double ymin, double ymax)
{
	double xwidth = xmax - xmin;
	double ywidth = ymax - ymin;
	m_isMasked = false;
	vtkSmartPointer<vtkGeometryFilter> gfilter = vtkSmartPointer<vtkGeometryFilter>::New();
	gfilter->SetExtent(xmin - xwidth * 0.2, xmax + xwidth * 0.2, ymin - ywidth * 0.2, ymax + ywidth * 0.2, -1, 1);
	gfilter->ExtentClippingOn();
	gfilter->SetInputData(m_vtkGrid);
	gfilter->Update();
	vtkSmartPointer<vtkPolyData> clippedGrid = gfilter->GetOutput();

	int ccounts = clippedGrid->GetNumberOfCells();
	if (ccounts > MAX_DRAWCELLCOUNT){
		vtkSmartPointer<vtkMaskPolyData> maskPoly = vtkSmartPointer<vtkMaskPolyData>::New();
		int ratio = static_cast<int>(ccounts / MAX_DRAWCELLCOUNT);
		if (ratio == 1){ratio = 2;}
		maskPoly->SetOnRatio(ratio);
		maskPoly->SetInputConnection(gfilter->GetOutputPort());

		m_vtkFilteredShapeAlgorithm = maskPoly;
		m_vtkFilteredPointsAlgorithm = maskPoly;
		m_vtkFilteredCellsAlgorithm = maskPoly;
		m_isMasked = true;
	} else {
		m_vtkFilteredShapeAlgorithm = gfilter;
		m_vtkFilteredPointsAlgorithm = gfilter;
		m_vtkFilteredCellsAlgorithm = gfilter;
	}
}

void Grid::setParent(QObject *parent)
{
	QObject::setParent(parent);
	for (int i = 0; i < m_gridRelatedConditions.count(); ++i){
		GridRelatedConditionContainer* c = m_gridRelatedConditions[i];
		c->updateConnections();
	}
}