#include "structured15dgridwithcrosssection.h"
#include "../../project/projectcgnsfile.h"
#include <misc/stringtool.h>
#include "structured15dgrid/structured15dgridwithcrosssectioncrosssection.h"
#include "../gridcond/base/gridrelatedconditioncontainer.h"

#include <vtkLine.h>
#include <vtkVertex.h>

#include <cgnslib.h>
#include <iriclib.h>

Structured15DGridWithCrossSection::Structured15DGridWithCrossSection(ProjectDataItem* parent)
	: Grid2D(SolverDefinitionGridType::gtNormal1_5DGridWithCrosssection, parent)
{
	init();
}

Structured15DGridWithCrossSection::Structured15DGridWithCrossSection(const QString& zonename, ProjectDataItem* parent)
	: Grid2D(zonename, SolverDefinitionGridType::gtNormal1_5DGridWithCrosssection, parent)
{
	init();
}

void Structured15DGridWithCrossSection::init()
{
	m_vtkGrid = vtkUnstructuredGrid::New();
	m_vertexGrid = vtkUnstructuredGrid::New();
//	m_importerFactory = new Structured15DGridWithCrossSectionImporterFactory(this);
//	m_exporterFactory = new Structured15DGridWithCrossSectionExporterFactory(this);
}

const QVector2D Structured15DGridWithCrossSection::vertex(unsigned int index) const
{
	double points[3];
	m_vtkGrid->GetPoints()->GetPoint(index, points);

	return QVector2D(points[0], points[1]);
}

void Structured15DGridWithCrossSection::setVertex(unsigned int index, const QVector2D &v)
{
	m_vtkGrid->GetPoints()->SetPoint(index, v.x(), v.y(), 0.0);
}

void Structured15DGridWithCrossSection::setPoints(vtkPoints *points)
{
	vtkIdType num = points->GetNumberOfPoints();
	vtkUnstructuredGrid* edges = dynamic_cast<vtkUnstructuredGrid*>(m_vtkGrid);
	vtkUnstructuredGrid* vertices = dynamic_cast<vtkUnstructuredGrid*>(m_vertexGrid);

	edges->SetPoints(points);
	vertices->SetPoints(points);

	// for edges
	for (vtkIdType i = 0; i < num - 1; i++){
		vtkLine* line = vtkLine::New();
		line->GetPointIds()->InsertId(0, i);
		line->GetPointIds()->InsertId(1, i + 1);
		edges->InsertNextCell(line->GetCellType(), line->GetPointIds());
	}
	edges->BuildLinks();

	// for vertices
	for (vtkIdType i = 0; i < num; i++){
		vtkVertex* vertex = vtkVertex::New();
		vertex->GetPointIds()->InsertId(0, i);
		vertices->InsertNextCell(vertex->GetCellType(), vertex->GetPointIds());
	}
	vertices->BuildLinks();
}

bool Structured15DGridWithCrossSection::loadFromCgnsFile(const int fn, int B, int Z)
{
	int ier;
	cgsize_t size[9];
	char buffer[ProjectCgnsFile::BUFFERLEN];
	ier = cg_zone_read(fn, B, Z, buffer, size);
	if (ier != 0){return false;}
	int riversize = size[0];

	ier = cg_goto(fn, B, "Zone_t", Z, "GridCoordinates", 0, "end");
	if (ier != 0){
		// grid data does not exists.
		return false;
	}
	// the first one must be X.
	DataType_t dType;
	int narrays;
	int dim;
	cgsize_t dimV[3];
	cg_narrays(&narrays);
	ier = cg_array_info(1, buffer, &dType, &dim, dimV);
	double* dataX = new double[riversize];
	if (dType == RealSingle){
		float* tmpFloat = new float[riversize];
		ier = cg_array_read(1, tmpFloat);
		for (int i = 0; i < riversize; i++){
			*(dataX + i) = *(tmpFloat + i);
		}
		delete[] tmpFloat;
	} else {
		ier = cg_array_read(1, dataX);
	}
	// the second one must be Y;
	ier = cg_array_info(2, buffer, &dType, &dim, dimV);
	double* dataY = new double[riversize];
	if (dType == RealSingle){
		float* tmpFloat = new float[riversize];
		ier = cg_array_read(2, tmpFloat);
		for (int i = 0; i < riversize; i++){
			*(dataY + i) = *(tmpFloat + i);
		}
		delete[] tmpFloat;
	} else {
		ier = cg_array_read(2, dataY);
	}
	vtkPoints* points = vtkPoints::New();
		points->SetDataTypeToDouble();
		for (int i = 0; i < riversize; i++){
		points->InsertNextPoint(*(dataX + i), *(dataY + i), 0);
	}
	setPoints(points);
	points->Delete();
	delete[] dataX;
	delete[] dataY;

	// Grid coordinates are loaded.
	// Next, grid related condition data is loaded.
	loadGridRelatedConditions(fn, B, Z);

	// Grid related conditions are loaded.
	// Next, cross section data is loaded.
	for (int i = 1; i <= riversize; i++){
		QString str = QString::number(i);
		Structured15DGridWithCrossSectionCrossSection* crossSection = new Structured15DGridWithCrossSectionCrossSection(str, this);
		crossSection->loadFromCgnsFile(fn, B, Z, i);
		m_crossSections << crossSection;
	}
	return true;
}

bool Structured15DGridWithCrossSection::saveToCgnsFile(const int fn, int B, char* zonename)
{
	int ier;
	cgsize_t sizes[9];

	// Check whether the corresponding zone already exists.
	int zoneid = zoneId(zonename, fn, B, sizes);
	if (zoneid != 0){
		// Zone already exists! remove this zone first!
		ier = cg_delete_node(zonename);
		if (ier != 0){return false;}
	}
	// Now, create new zone.
	int num = m_vtkGrid->GetNumberOfPoints();
	sizes[0] = num;
	sizes[1] = 1;
	sizes[2] = num - 1;
	sizes[3] = 0;
	sizes[4] = 0;
	sizes[5] = 0;
	cg_zone_write(fn, B, zonename, sizes, Structured, &zoneid);

	// save grid coordinates.
	int G;
	ier = cg_grid_write(fn, B, zoneid, "GridCoordinates", &G);
	if (ier != 0){return false;}
	int C;
	// save coordinates.
	double* dataX = new double[num];
	double* dataY = new double[num];
	double points[3];

	for (int i = 0; i < num; i++){
		m_vtkGrid->GetPoints()->GetPoint(i, points);
		*(dataX + i) = points[0];
		*(dataY + i) = points[1];
	}
	ier = cg_coord_write(fn, B, zoneid, RealDouble, "CoordinateX", dataX, &C);
	if (ier != 0){return false;}
	ier = cg_coord_write(fn, B, zoneid, RealDouble, "CoordinateY", dataY, &C);
	if (ier != 0){return false;}

	delete[] dataX;
	delete[] dataY;

	// Grid coordinates are saved.
	// Next grid related condition data is saved.
	// Create "GridConditions" node under the zone node.
	saveGridRelatedConditions(fn, B, zoneid);

	// Grid related conditions are saved.
	// Next cross section data is saved.
	cg_goto(fn, B, "Zone_t", zoneid, "end");
	cg_user_data_write("GridCrosssections");
	int index = 1;
	QList<Structured15DGridWithCrossSectionCrossSection*>::iterator it2;
	for (it2 = m_crossSections.begin(); it2 != m_crossSections.end(); it2++){
		(*it2)->saveToCgnsFile(fn, B, zoneid, index);
		index++;
	}
	return true;
}