#include "postzonedatacontainer.h"
#include "postsolutioninfo.h"
#include "../project/projectcgnsfile.h"
#include <misc/stringtool.h>
#include <misc/filesystemfunction.h>
#include "../solverdef/solverdefinitiongridtype.h"
#include "../solverdef/solverdefinitiongridrelatedcondition.h"
#include "../solverdef/solverdefinitiongridrelatedconditiont.h"
#include "../project/projectdata.h"
#include "../pre/grid/structured2dgrid.h"
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGridWriter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkDataSetAttributes.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkTriangle.h>
#include <vtkSmartPointer.h>
#include <vtkExtractGrid.h>
#include <vtkStringArray.h>
#include <vtkGeometryFilter.h>
#include <QRegExp>
#include <QTextStream>
#include <cgnslib.h>
#include <iriclib.h>

#define ELEMNODENAME "Element"

const QString PostZoneDataContainer::labelName("_LABEL");
const QString PostZoneDataContainer::IBC("IBC");
const double PostZoneDataContainer::IBCLimit(0.99);

PostZoneDataContainer::PostZoneDataContainer(const QString& baseName, const QString& zoneName, SolverDefinitionGridType* gridtype, ProjectDataItem* parent)
	: PostDataContainer(parent)
{
	m_baseName = baseName;
	m_zoneName = zoneName;
	m_gridType = gridtype;
	m_baseId = 0;
	m_zoneId = 0;
	m_cellDim = 0;
	m_loadedOnce = false;
}

bool PostZoneDataContainer::setBaseId(const int fn)
{
	// if m_baseID is already set, we do not have to do it again.
	if (m_baseId != 0){return true;}

	int ier;
	int numBases;
	ier = cg_nbases(fn, &numBases);
	if (ier != 0){return false;}
	for (int B = 1; B <= numBases; ++B){
		char basename[32];
		int phys_dim;
		ier = cg_base_read(fn, B, basename, &m_cellDim, &phys_dim);
		if (ier != 0){return false;}
		if (m_baseName == basename){
			m_baseId = B;
			return true;
		}
	}
	return false;
}

bool PostZoneDataContainer::setZoneId(const int fn)
{
	// if m_zoneID is already set, we do not have to do it again.
	if (m_zoneId != 0){return true;}

	int ier;
	int numZones;
	ier = cg_nzones(fn, m_baseId, &numZones);
	if (ier != 0){return false;}
	for (int Z = 1; Z <= numZones; ++Z){
		char zonename[32];
		ier = cg_zone_read(fn, m_baseId, Z, zonename, m_sizes);
		if (ier != 0){return false;}
		if (m_zoneName == zonename){
			m_zoneId = Z;
			return true;
		}
	}
	return false;
}

bool PostZoneDataContainer::loadZoneSize(const int fn)
{
	int ier;
	char zonename[32];
	ier = cg_zone_read(fn, m_baseId, m_zoneId, zonename, m_sizes);
	return (ier == 0);
}

bool PostZoneDataContainer::loadStructuredGrid(const int fn, const int currentStep)
{
	if (m_data != 0){
		m_data->Initialize();
		m_labelData->Initialize();
	}else{
		m_data = vtkSmartPointer<vtkStructuredGrid>::New();
		m_labelData = vtkSmartPointer<vtkUnstructuredGrid>::New();
	}
	vtkPointSet* p1 = m_data;
	vtkStructuredGrid* grid = dynamic_cast<vtkStructuredGrid*>(p1);
	int NVertexI, NVertexJ, NVertexK;
	if (m_cellDim == 1){
		NVertexI = m_sizes[0];
		NVertexJ = 1;
		NVertexK = 1;
	} else if (m_cellDim == 2){
		NVertexI = m_sizes[0];
		NVertexJ = m_sizes[1];
		NVertexK = 1;
	} else if (m_cellDim == 3){
		NVertexI = m_sizes[0];
		NVertexJ = m_sizes[1];
		NVertexK = m_sizes[2];
	}
	grid->SetDimensions(NVertexI, NVertexJ, NVertexK);
	// Find zone iterative data.
	char zoneItername[32];
	int ier;

	/// only for test!
//	if (currentStep == 1){return false;}

	ier = cg_ziter_read(fn, m_baseId, m_zoneId, zoneItername);
	bool iterativeCoordinates = false;
	if (ier == 0){
		// zone iterative data exists.
		// try to read the array information of GridCoordinagesPointers.
		ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, zoneItername, 0, "end");
		if (ier != 0){return false;}
		int narrays;
		ier = cg_narrays(&narrays);
		if (ier != 0){return false;}
		for (int i = 1; i <= narrays; ++i){
			char arrayname[ProjectCgnsFile::BUFFERLEN];
			DataType_t dataType;
			int dimension;
			cgsize_t dimVector[3];
			cg_array_info(i, arrayname, &dataType, &dimension, dimVector);
			if (QString(arrayname) == "GridCoordinatesPointers"){
				// GridCoordinatesPointers found.
				char *pointers;
				// dimension = 2, dimVector = [32, NumberOfSteps].
				pointers = new char[dimVector[0] * dimVector[1]];
				cg_array_read(i, pointers);
				char currentCoordinates[32];
				memcpy(currentCoordinates, &(pointers[32 * currentStep]), 32);
				// currentCoordinates is not null terminated.
				currentCoordinates[31] = '\0';
				QString curCoord = QString(currentCoordinates).trimmed();
				delete pointers;
				// now, goto the specified coordinates node.
				ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, iRIC::toStr(curCoord).c_str(), 0, "end");
				iterativeCoordinates = true;
			}
		}
	}
	if (! iterativeCoordinates){
		// The grid shape used as input grid is also used as result grid.
		ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, "GridCoordinates", 0, "end");
	}
	if (ier != 0){
		// grid data does not exists.
		return false;
	}
	// get array info in order to know what data type is used.
	// assume that x, y and z are the same type.
	char aName[ProjectCgnsFile::BUFFERLEN];
	DataType_t dType;
	int d;
	cgsize_t dVector[3];
	cg_array_info(1, aName, &dType, &d, dVector);

	double *dataX, *dataY, *dataZ;
	size_t numPoints = NVertexI * NVertexJ * NVertexK;
	dataX = new double[numPoints];
	dataY = new double[numPoints];
	dataZ = new double[numPoints];

	// RealSingle or RealDouble
	if (dType == RealSingle){
		float *tmpX, *tmpY, *tmpZ;
		tmpX = new float[numPoints];
		tmpY = new float[numPoints];
		tmpZ = new float[numPoints];

		// the first one must be X.
		ier = cg_array_read(1, tmpX);
		if (ier != 0){goto FREETMP;}
		if (m_cellDim >= 2){
			// the second one must be Y.
			ier = cg_array_read(2, tmpY);
			if (ier != 0){goto FREETMP;}
		} else {
			for (unsigned int i = 0; i < numPoints; ++i){*(tmpY + i) = 0;}
		}
		if (m_cellDim == 3){
			// the third one must be Z.
			ier = cg_array_read(3, tmpZ);
			if (ier != 0){goto FREETMP;}
		} else {
			for (unsigned int i = 0; i < numPoints; ++i){*(tmpZ + i) = 0;}
		}
		for (unsigned int i = 0; i < numPoints; i++){
			*(dataX + i) = *(tmpX + i);
			*(dataY + i) = *(tmpY + i);
			*(dataZ + i) = *(tmpZ + i);
		}

		FREETMP:

		delete[] tmpX;
		delete[] tmpY;
		delete[] tmpZ;

		if (ier != 0){
			goto FREEDATA;
		}
	} else {
		// the first one must be X.
		ier = cg_array_read(1, dataX);
		if (ier != 0){goto FREEDATA;}

		if (m_cellDim >= 2){
			// the second one must be Y.
			ier = cg_array_read(2, dataY);
			if (ier != 0){goto FREEDATA;}
		}else{
			for (unsigned int i = 0; i < numPoints; ++i){*(dataY + i) = 0;}
		}
		if (m_cellDim == 3){
			// the third one must be Z.
			ier = cg_array_read(3, dataZ);
			if (ier != 0){goto FREEDATA;}
		}else{
			for (unsigned int i = 0; i < numPoints; ++i){*(dataZ + i) = 0;}
		}
	}

	vtkPoints* points = vtkPoints::New();
	for (int k = 0; k < NVertexK; ++k){
		for (int j = 0; j < NVertexJ; ++j){
			for (int i = 0; i < NVertexI; ++i){
				points->InsertNextPoint(
						*(dataX + i + NVertexI * (j + NVertexJ * k)),
						*(dataY + i + NVertexI * (j + NVertexJ * k)),
						*(dataZ + i + NVertexI * (j + NVertexJ * k)));
			}
		}
	}
	grid->SetPoints(points);
	grid->Modified();
	points->Delete();

	FREEDATA:

	delete[] dataX;
	delete[] dataY;
	delete[] dataZ;
	return (ier == 0);
}

bool PostZoneDataContainer::loadUnstructuredGrid(const int fn, const int currentStep)
{
	if (m_data != 0){
		m_data->Initialize();
	}else{
		m_data = vtkSmartPointer<vtkUnstructuredGrid>::New();
		m_labelData = m_data;
	}
	vtkPointSet* p1 = m_data;
	vtkUnstructuredGrid* grid = dynamic_cast<vtkUnstructuredGrid*>(p1);
	int NVertex = m_sizes[0];
	// Find zone iterative data.
	char zoneItername[32];
	int ier;
	ier = cg_ziter_read(fn, m_baseId, m_zoneId, zoneItername);
	bool iterativeCoordinates = false;
	if (ier == 0){
		// zone iterative data exists.
		// try to read the array information of GridCoordinagesPointers.
		ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, zoneItername, 0, "end");
		if (ier != 0){return false;}
		int narrays;
		ier = cg_narrays(&narrays);
		if (ier != 0){return false;}
		for (int i = 1; i <= narrays; ++i){
			char arrayname[ProjectCgnsFile::BUFFERLEN];
			DataType_t dataType;
			int dimension;
			cgsize_t dimVector[3];
			cg_array_info(i, arrayname, &dataType, &dimension, dimVector);
			if (QString(arrayname) == "GridCoordinatesPointers"){
				// GridCoordinatesPointers found.
				char *pointers;
				// dimension = 2, dimVector = [32, NumberOfSteps].
				pointers = new char[dimVector[0] * dimVector[1]];
				cg_array_read(i, pointers);
				char currentCoordinates[32];
				memcpy(currentCoordinates, &(pointers[32 * currentStep]), 32);
				// currentCoordinates is not null terminated.
				currentCoordinates[31] = '\0';
				QString curCoord = QString(currentCoordinates).trimmed();
				delete pointers;
				// now, goto the specified coordinates node.
				ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, iRIC::toStr(curCoord).c_str(), 0, "end");
				iterativeCoordinates = true;
			}
		}
	}
	if (! iterativeCoordinates){
		// The grid shape used as input grid is also used as result grid.
		ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, "GridCoordinates", 0, "end");
	}
	if (ier != 0){
		// grid data does not exists.
		return false;
	}
	// get array info in order to know what data type is used.
	// assume that x, y and z are the same type.
	char aName[ProjectCgnsFile::BUFFERLEN];
	DataType_t dType;
	int d;
	cgsize_t dVector[3];
	cg_array_info(1, aName, &dType, &d, dVector);

	double *dataX, *dataY, *dataZ;
	size_t numPoints = NVertex;
	dataX = new double[numPoints];
	dataY = new double[numPoints];
	dataZ = new double[numPoints];

	// RealSingle or RealDouble
	if (dType == RealSingle){
		float *tmpX, *tmpY, *tmpZ;
		tmpX = new float[numPoints];
		tmpY = new float[numPoints];
		tmpZ = new float[numPoints];

		// the first one must be X.
		ier = cg_array_read(1, tmpX);
		if (m_cellDim >= 2){
			// the second one must be Y.
			ier = cg_array_read(2, tmpY);
		} else {
			for (unsigned int i = 0; i < numPoints; ++i){*(tmpY + i) = 0;}
		}
		if (m_cellDim == 3){
			// the third one must be Z.
			ier = cg_array_read(3, tmpZ);
		} else {
			for (unsigned int i = 0; i < numPoints; ++i){*(tmpZ + i) = 0;}
		}
		for (unsigned int i = 0; i < numPoints; i++){
			*(dataX + i) = *(tmpX + i);
			*(dataY + i) = *(tmpY + i);
			*(dataZ + i) = *(tmpZ + i);
		}
		delete[] tmpX;
		delete[] tmpY;
		delete[] tmpZ;
	} else {
		// the first one must be X.
		ier = cg_array_read(1, dataX);

		if (m_cellDim >= 2){
			// the second one must be Y.
			ier = cg_array_read(2, dataY);
		}else{
			for (unsigned int i = 0; i < numPoints; ++i){*(dataY + i) = 0;}
		}
		if (m_cellDim == 3){
			// the third one must be Z.
			ier = cg_array_read(3, dataZ);
		}else{
			for (unsigned int i = 0; i < numPoints; ++i){*(dataZ + i) = 0;}
		}
	}

	vtkPoints* points = vtkPoints::New();
	for (int i = 0; i < NVertex; ++i){
		points->InsertNextPoint(
				*(dataX + i),
				*(dataY + i),
				*(dataZ + i));
	}
	grid->SetPoints(points);

	grid->Modified();
	points->Delete();
	delete[] dataX;
	delete[] dataY;
	delete[] dataZ;

	// Grid coordinates are loaded.
	// load grid node connectivity data.
	// Unstructured grid that consists of triangles is supported.

	int numSections;
	cg_nsections(fn, m_baseId, m_zoneId, &numSections);
	for (int S = 1; S <= numSections; ++S){
		ElementType_t eType;
		cgsize_t startIndex, endIndex;
		int nBndry, parent_flag;
		char buffer[32];
		cg_section_read(fn, m_baseId, m_zoneId, S, buffer, &eType, &startIndex, &endIndex, &nBndry, &parent_flag);
		if (QString(buffer) == ELEMNODENAME){
			// the target element node found!
			// eType must be TRI3.
			cgsize_t numCells;
			cg_ElementDataSize(fn, m_baseId, m_zoneId, S, &numCells);
			numCells = numCells / 3;
			cgsize_t* elements;
			elements = new cgsize_t[3 * numCells];
			cg_elements_read(fn, m_baseId, m_zoneId, S, elements, NULL);
			grid->Allocate(numCells);
			for (int i = 0; i < numCells; ++i){
				vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
				int id0 = *(elements + i * 3) - 1;
				int id1 = *(elements + i * 3 + 1) - 1;
				int id2 = *(elements + i * 3 + 2) - 1;
				triangle->GetPointIds()->SetId(0, id0);
				triangle->GetPointIds()->SetId(1, id1);
				triangle->GetPointIds()->SetId(2, id2);
				grid->InsertNextCell(triangle->GetCellType(), triangle->GetPointIds());
			}
			delete elements;
		}
	}
	return true;
}

bool PostZoneDataContainer::loadParticle(const int fn, const int currentStep)
{
	int ier;
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> verts = vtkSmartPointer<vtkCellArray>::New();

	if (m_particleData != 0){
		m_particleData->Initialize();
	}else{
		m_particleData = vtkSmartPointer<vtkPolyData>::New();
	}
	// Find ParticleResult node.
	char nodeName[ProjectCgnsFile::BUFFERLEN];
	QString tmpNodeName = QString("ParticleSolution%1").arg(currentStep + 1);
	strcpy(nodeName, iRIC::toStr(tmpNodeName).c_str());
	ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, nodeName, 0, "end");
	if (ier != 0){
		// particle data does not exists.
		// and it is not an error, because particle is optional data.
		return true;
	}
	// get array info in order to know the number of particles.
	char aName[ProjectCgnsFile::BUFFERLEN];
	DataType_t dType;
	int d;
	cgsize_t dVector[3];

	// get the number of particles.
	cg_array_info(1, aName, &dType, &d, dVector);

	double *dataX, *dataY, *dataZ;
	size_t numParticles = dVector[0];
	dataX = new double[numParticles];
	dataY = new double[numParticles];
	dataZ = new double[numParticles];

	int firstAttId = 4;

	// Read X
	ier = cg_array_info(1, aName, &dType, &d, dVector);
	if (ier != 0 || QString(aName) != "CoordinateX" || dVector[0] != numParticles){
		goto ERROR;
	}
	ier = cg_array_read(1, dataX);
	if (ier != 0){goto ERROR;}

	// Read Y
	ier = cg_array_info(2, aName, &dType, &d, dVector);
	if (ier != 0 || QString(aName) != "CoordinateY" || dVector[0] != numParticles){
		goto ERROR;
	}
	ier = cg_array_read(2, dataY);
	if (ier != 0){goto ERROR;}

	// Read Z (optional)
	ier = cg_array_info(3, aName, &dType, &d, dVector);
	if (ier != 0 || QString(aName) != "CoordinateZ"){
		// Z data does not exist;
		for (size_t i = 0; i < numParticles; ++i){
			*(dataZ + i) = 0;
		}
		firstAttId = 3;
	} else {
		if (dVector[0] != numParticles){
			goto ERROR;
		}
		ier = cg_array_read(3, dataZ);
		if (ier != 0){goto ERROR;}
	}

	// X, Y, Z are setup.

	for (int i = 0; i < numParticles; ++i){
		points->InsertNextPoint(
				*(dataX + i),
				*(dataY + i),
				*(dataZ + i));
		vtkIdType pId = i;
		verts->InsertNextCell(1, &pId);
	}
	points->Modified();
	m_particleData->SetPoints(points);
	m_particleData->SetVerts(verts);

	// load attributes
	int numArrays;
	ier = cg_narrays(&numArrays);
	if (ier != 0){goto ERROR;}

	for (int i = firstAttId; i <= numArrays; ++i){
		ier = cg_array_info(i, aName, &dType, &d, dVector);
		if (ier != 0){goto ERROR;}
		if (dVector[0] != numParticles){
			goto ERROR;
		}
		if (dType == Integer){
			int *att = new int[numParticles];
			ier = cg_array_read(i, att);
			if (ier != 0){
				delete[] att;
				goto ERROR;
			}
			vtkSmartPointer<vtkIntArray> attArray = vtkSmartPointer<vtkIntArray>::New();
			attArray->SetName(aName);
			attArray->Allocate(numParticles);
			for (int j = 0; j < numParticles; ++j){
				attArray->InsertNextValue(*(att + j));
			}
			m_particleData->GetPointData()->AddArray(attArray);
			delete[] att;
		} else if (dType == RealDouble){
			double *att = new double[numParticles];
			ier = cg_array_read(i, att);
			if (ier != 0){
				delete[] att;
				goto ERROR;
			}
			vtkSmartPointer<vtkDoubleArray> attArray = vtkSmartPointer<vtkDoubleArray>::New();
			attArray->SetName(aName);
			attArray->Allocate(numParticles);
			for (int j = 0; j < numParticles; ++j){
				attArray->InsertNextValue(*(att + j));
			}
			m_particleData->GetPointData()->AddArray(attArray);
			delete[] att;
		}
	}
	m_particleData->Modified();

	return true;

ERROR:
	delete[] dataX;
	delete[] dataY;
	delete[] dataZ;
	m_particleData->Initialize();

	return false;
}


bool PostZoneDataContainer::getSoluionId(const int fn, const int currentStep, int *solid)
{
	int ier;
	char zoneItername[32];
	ier = cg_ziter_read(fn, m_baseId, m_zoneId, zoneItername);
	if (ier == 0){
		ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, zoneItername, 0, "end");
		if (ier != 0){return false;}
		int narrays;
		ier = cg_narrays(&narrays);
		if (ier != 0){return false;}
		for (int i = 1; i <= narrays; ++i){
			char arrayname[32];
			DataType_t dataType;
			int dimension;
			cgsize_t dimVector[3];
			cg_array_info(i, arrayname, &dataType, &dimension, dimVector);
			if (QString(arrayname) == "FlowSolutionPointers"){
				// FlowSolutionPointers found.
				char *pointers;
				// dimension = 2, dimVector = [32, NumberOfSteps].
				pointers = new char[dimVector[0] * dimVector[1]];
				cg_array_read(i, pointers);
				char curSol[32];
				memcpy(curSol, &(pointers[32 * currentStep]), 32);
				// the currentSolution is not null terminated.
				curSol[31] = '\0';
				QString currentSolution = QString(curSol).trimmed();
				delete pointers;
				// now, find the solution data and set solId.
				int nsols;
				cg_nsols(fn, m_baseId, m_zoneId, &nsols);
				for (int j = 1; j <= nsols; ++j){
					char solname[32];
					GridLocation_t location;
					ier = cg_sol_info(fn, m_baseId, m_zoneId, j, solname, &location);
					if (ier != 0){return false;}
					if (currentSolution == solname){
						*solid = j;
						return true;
					}
				}
			}
		}
	} else {
		*solid = currentStep + 1;
		return true;
	}
	return false;
}

bool PostZoneDataContainer::loadScalarData(const int fn, const int solid)
{
	int ier;
	char solname[32];
	GridLocation_t location;
	ier = cg_sol_info(fn, m_baseId, m_zoneId, solid, solname, &location);
	if (ier != 0){return false;}
	ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, "FlowSolution_t", solid, "end");
	if (ier != 0){return false;}
	vtkDataSetAttributes* data;
	if (location == Vertex){
		// vertex.
		data = m_data->GetPointData();
	}else{
		// cell center.
		data = m_data->GetCellData();
	}
	int narrays;
	cg_narrays(&narrays);
	for (int i = 1; i <= narrays; ++i){
		DataType_t datatype;
		int dimension;
		cgsize_t dimVector[3];
		char arrayname[30];
		cg_array_info(i, arrayname, &datatype, &dimension, dimVector);
		QString name(arrayname);

		// skip vector values.
		QRegExp rx;
		// For example, "VelocityX"
		rx = QRegExp("(.*)X$");
		if (rx.indexIn(name) != -1){continue;}
		// For example, "VelocityY"
		rx = QRegExp("(.*)Y$");
		if (rx.indexIn(name) != -1){continue;}
		// For example, "VelocityZ"
		rx = QRegExp("(.*)Z$");
		if (rx.indexIn(name) != -1){continue;}

		int datalen = 1;
		for (int j = 0; j < dimension; ++j){
			datalen *= dimVector[j];
		}
		if (datatype == Integer){
			int* data2 = new int[datalen];
			cg_array_read(i, data2);
			vtkSmartPointer<vtkIntArray> tmpArray = vtkSmartPointer<vtkIntArray>::New();
			tmpArray->SetName(arrayname);
			tmpArray->Allocate(datalen);
			if (IBC == arrayname){
				// for IBC values, special handling is done: 0 is inactive the others are all active.
				for (int j = 0; j < datalen; ++j){
					int val = *(data2 + j);
					if (val != 0){val = 1;}
					tmpArray->InsertNextValue(val);
				}
			} else {
				for (int j = 0; j < datalen; ++j){
					tmpArray->InsertNextValue(*(data2 + j));
				}
			}
			delete data2;
			data->AddArray(tmpArray);
		} else if (datatype == RealSingle){
			float* data2 = new float[datalen];
			cg_array_read(i, data2);
			vtkSmartPointer<vtkFloatArray> tmpArray = vtkSmartPointer<vtkFloatArray>::New();
			tmpArray->SetName(arrayname);
			tmpArray->Allocate(datalen);
			for (int j = 0; j < datalen; ++j){
				tmpArray->InsertNextValue(*(data2 + j));
			}
			delete data2;
			data->AddArray(tmpArray);
		} else if (datatype == RealDouble){
			double* data2 = new double[datalen];
			cg_array_read(i, data2);
			vtkSmartPointer<vtkDoubleArray> tmpArray = vtkSmartPointer<vtkDoubleArray>::New();
			tmpArray->SetName(arrayname);
			tmpArray->Allocate(datalen);
			for (int j = 0; j < datalen; ++j){
				tmpArray->InsertNextValue(*(data2 + j));
			}
			delete data2;
			data->AddArray(tmpArray);
		}
	}
	data->Modified();
	return true;
}

bool PostZoneDataContainer::loadVectorData(const int fn, const int solid)
{
	int ier;
	char solname[32];
	GridLocation_t location;
	ier = cg_sol_info(fn, m_baseId, m_zoneId, solid, solname, &location);
	if (ier != 0){return false;}
	ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, "FlowSolution_t", solid, "end");
	if (ier != 0){return false;}
	vtkDataSetAttributes* data;
	if (location == Vertex){
		// vertex.
		data = m_data->GetPointData();
	}else{
		// cell center.
		data = m_data->GetCellData();
	}
	int narrays;
	cg_narrays(&narrays);

	// try to find vector attributes.
	for (int i = 1; i <= narrays; ++i){
		char arrayname[30];
		DataType_t datatype;
		int dimension;
		cgsize_t dimVector[3];
		cg_array_info(i, arrayname, &datatype, &dimension, dimVector);
		QString name(arrayname);

		QRegExp rx;
		// find solution with name "VelocityX", for example.
		rx = QRegExp("(.*)X$");
		if (rx.indexIn(name) != -1){
			int indexX, indexY, indexZ;
			// vector data found.
			QString vectorName = rx.cap(1);

			indexX = i;
			indexY = 0;
			indexZ = 0;
			// try to find Y component. i.e. "VelocityY"
			QString yname = vectorName;
			yname.append("Y");
			for (int j = 1; j <= narrays; ++j){
				char arrayname2[30];
				DataType_t datatype2;
				int dimension2;
				cgsize_t dimVector2[3];
				cg_array_info(j, arrayname2, &datatype2, &dimension2, dimVector2);
				if (yname == arrayname2 && datatype == datatype2 && dimension == dimension2){
					indexY = j;
				}
			}
			if (indexY != 0){
				// try to find Z component. i.e. "VelocityZ"
				QString zname = vectorName;
				zname.append("Z");
				for (int j = 1; j <= narrays; ++j){
					char arrayname2[30];
					DataType_t datatype2;
					int dimension2;
					cgsize_t dimVector2[3];
					cg_array_info(j, arrayname2, &datatype2, &dimension2, dimVector2);
					if (zname == arrayname2 && datatype == datatype2 && dimension == dimension2){
						indexZ = j;
					}
				}
			}
			// OK, indexX, indexY, indexZ are set correctly.
			unsigned int datalen = 1;
			for (int j = 0; j < dimension; ++j){
				datalen *= dimVector[j];
			}
			if (datatype == Integer){
				int* dataX = new int[datalen];
				int* dataY = new int[datalen];
				int* dataZ = new int[datalen];
				double* dataMag = new double[datalen];
				// read x.
				cg_array_read(indexX, dataX);
				// read y.
				if (indexY != 0){
					cg_array_read(indexY, dataY);
				}else{
					for (unsigned int j = 0; j < datalen; ++j){*(dataY + j) = 0;}
				}
				// read z
				if (indexZ != 0){
					cg_array_read(indexZ, dataZ);
				}else{
					for (unsigned int j = 0; j < datalen; ++j){*(dataZ + j) = 0;}
				}
				for (unsigned int j = 0; j < datalen; ++j){
					double sum = 0;
					sum += *(dataX + j) * (*(dataX + j));
					sum += *(dataY + j) * (*(dataY + j));
					sum += *(dataZ + j) * (*(dataZ + j));
					double v = sqrt(sum);
					*(dataMag + j) = v;
				}
				vtkSmartPointer<vtkIntArray> tmpArray = vtkSmartPointer<vtkIntArray>::New();
				tmpArray->SetName(iRIC::toStr(vectorName).c_str());
				tmpArray->SetNumberOfComponents(3);
				tmpArray->Allocate(datalen);
				for (unsigned int j = 0; j < datalen; ++j){
					tmpArray->InsertNextTuple3(*(dataX + j), *(dataY + j), *(dataZ + j));
				}
				delete dataX;
				delete dataY;
				delete dataZ;
				data->AddArray(tmpArray);

				vtkSmartPointer<vtkDoubleArray> magArray = vtkSmartPointer<vtkDoubleArray>::New();
				magArray->SetName(iRIC::toStr(vectorName.append(" (magnitude)")).c_str());
				magArray->Allocate(datalen);
				for (unsigned int j = 0; j < datalen; ++j){
					magArray->InsertNextValue(*(dataMag + j));
				}
				delete dataMag;
				data->AddArray(magArray);
			}else if (datatype == RealSingle){
				float* dataX = new float[datalen];
				float* dataY = new float[datalen];
				float* dataZ = new float[datalen];
				double* dataMag = new double[datalen];
				// read x.
				cg_array_read(indexX, dataX);
				// read y.
				if (indexY != 0){
					cg_array_read(indexY, dataY);
				}else{
					for (unsigned int j = 0; j < datalen; ++j){*(dataY + j) = 0;}
				}
				// read z
				if (indexZ != 0){
					cg_array_read(indexZ, dataZ);
				}else{
					for (unsigned int j = 0; j < datalen; ++j){*(dataZ + j) = 0;}
				}
				for (unsigned int j = 0; j < datalen; ++j){
					double sum = 0;
					sum += *(dataX + j) * (*(dataX + j));
					sum += *(dataY + j) * (*(dataY + j));
					sum += *(dataZ + j) * (*(dataZ + j));
					double v = sqrt(sum);
					*(dataMag + j) = v;
				}
				vtkSmartPointer<vtkFloatArray> tmpArray = vtkSmartPointer<vtkFloatArray>::New();
				tmpArray->SetName(iRIC::toStr(vectorName).c_str());
				tmpArray->SetNumberOfComponents(3);
				tmpArray->Allocate(datalen);
				for (unsigned int j = 0; j < datalen; ++j){
					tmpArray->InsertNextTuple3(*(dataX + j), *(dataY + j), *(dataZ + j));
				}
				delete dataX;
				delete dataY;
				delete dataZ;
				data->AddArray(tmpArray);

				vtkSmartPointer<vtkDoubleArray> magArray = vtkSmartPointer<vtkDoubleArray>::New();
				magArray->SetName(iRIC::toStr(vectorName.append(" (magnitude)")).c_str());
				magArray->Allocate(datalen);
				for (unsigned int j = 0; j < datalen; ++j){
					magArray->InsertNextValue(*(dataMag + j));
				}
				delete dataMag;
				data->AddArray(magArray);
			}else if (datatype == RealDouble){
				double* dataX = new double[datalen];
				double* dataY = new double[datalen];
				double* dataZ = new double[datalen];
				double* dataMag = new double[datalen];
				// read x.
				cg_array_read(indexX, dataX);
				// read y.
				if (indexY != 0){
					cg_array_read(indexY, dataY);
				}else{
					for (unsigned int j = 0; j < datalen; ++j){*(dataY + j) = 0;}
				}
				// read z
				if (indexZ != 0){
					cg_array_read(indexZ, dataZ);
				}else{
					for (unsigned int j = 0; j < datalen; ++j){*(dataZ + j) = 0;}
				}
				for (unsigned int j = 0; j < datalen; ++j){
					double sum = 0;
					sum += *(dataX + j) * (*(dataX + j));
					sum += *(dataY + j) * (*(dataY + j));
					sum += *(dataZ + j) * (*(dataZ + j));
					double v = sqrt(sum);
					*(dataMag + j) = v;
				}
				vtkSmartPointer<vtkDoubleArray> tmpArray = vtkSmartPointer<vtkDoubleArray>::New();
				tmpArray->SetName(iRIC::toStr(vectorName).c_str());
				tmpArray->SetNumberOfComponents(3);
				tmpArray->Allocate(datalen);
				for (unsigned int j = 0; j < datalen; ++j){
					tmpArray->InsertNextTuple3(*(dataX + j), *(dataY + j), *(dataZ + j));
				}
				delete dataX;
				delete dataY;
				delete dataZ;
				data->AddArray(tmpArray);

				vtkSmartPointer<vtkDoubleArray> magArray = vtkSmartPointer<vtkDoubleArray>::New();
				magArray->SetName(iRIC::toStr(vectorName.append(" (magnitude)")).c_str());
				magArray->Allocate(datalen);
				for (unsigned int j = 0; j < datalen; ++j){
					magArray->InsertNextValue(*(dataMag + j));
				}
				delete dataMag;
				data->AddArray(magArray);
			}
		}
	}
	return true;
}

bool PostZoneDataContainer::loadCellFlagData(const int fn)
{
	if (m_cellDim != 2){
		// cell flag data can be loaded only when m_cellDim == 2.
		return true;
	}

	const QList<SolverDefinitionGridRelatedCondition*>& conds = m_gridType->gridRelatedConditions();
	QList<SolverDefinitionGridRelatedCondition*>::const_iterator it;
	for (it = conds.begin(); it != conds.end(); ++it){
		const SolverDefinitionGridRelatedCondition* cond = *it;
		if (cond->position() != SolverDefinitionGridRelatedCondition::CellCenter){continue;}
		if (! cond->isOption()){continue;}
		const SolverDefinitionGridRelatedIntegerCondition* icond = dynamic_cast<const SolverDefinitionGridRelatedIntegerCondition*>(cond);
		if (icond == 0){continue;}

		// this is a cell flag to load.
		int ier;
		ier = cg_goto(fn, m_baseId, "Zone_t", m_zoneId, "GridConditions", 0, iRIC::toStr(cond->name()).c_str(), 0, "end");
		if (ier != 0){
			// Corresponding node does not exists.
			return false;
		}
		// Find "Value" array.
		int narrays;
		cg_narrays(&narrays);
		for (int i = 1; i <= narrays; ++i){
			char arrayName[ProjectCgnsFile::BUFFERLEN];
			DataType_t dt;
			int dataDimension;
			cgsize_t dimensionVector[3];
			cg_array_info(i, arrayName, &dt, &dataDimension, dimensionVector);
			if (QString(arrayName) == "Value"){
				// We've found the array!
				// load data.
				vtkSmartPointer<vtkIntArray> iarray = vtkSmartPointer<vtkIntArray>::New();
				unsigned int count = m_data->GetNumberOfCells();
				int* data = new int[count];
				ier = cg_array_read(i, data);
				for (unsigned int j = 0; j < count; ++j){
					iarray->InsertNextValue(*(data + j));
				}
				delete[] data;
				iarray->SetName(iRIC::toStr(cond->name()).c_str());

				m_data->GetCellData()->AddArray(iarray);
			}
		}
	}
	return true;
}

bool PostZoneDataContainer::setupIndexData()
{
	vtkStructuredGrid* sGrid = vtkStructuredGrid::SafeDownCast(m_data);
	vtkSmartPointer<vtkStringArray> indexArray = vtkSmartPointer<vtkStringArray>::New();
	indexArray->SetName(iRIC::toStr(labelName).c_str());
	vtkSmartPointer<vtkDoubleArray> newElevArray;
	vtkSmartPointer<vtkDoubleArray> origElevArray;
	if (elevationName() != ""){
		origElevArray = vtkDoubleArray::SafeDownCast(m_data->GetPointData()->GetArray(iRIC::toStr(elevationName()).c_str()));
		newElevArray = vtkSmartPointer<vtkDoubleArray>::New();
		newElevArray->SetName(origElevArray->GetName());
	}

	if (sGrid != 0){
		// structured grid.
		int dims[3];
		sGrid->GetDimensions(dims);
		QString label;
		if (dims[2] == 1){
			label = QString("(%1,%2)");
		} else {
			label = QString("(%1,%2,%3)");
		}
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		points->SetDataTypeToDouble();
		for (int i = 0; i < dims[0]; ++i){
			double p[3];
			sGrid->GetPoint(nodeIndex(i, 0, 0), p);
			points->InsertNextPoint(p);
			indexArray->InsertNextValue(iRIC::toStr(label.arg(i + 1).arg(1).arg(1)).c_str());
			if (newElevArray != 0){
				newElevArray->InsertNextValue(origElevArray->GetValue(nodeIndex(i, 0, 0)));
			}
		}
		for (int j = 1; j < dims[1]; ++j){
			double p[3];
			sGrid->GetPoint(nodeIndex(0, j, 0), p);
			points->InsertNextPoint(p);
			indexArray->InsertNextValue(iRIC::toStr(label.arg(1).arg(j + 1).arg(1)).c_str());
			if (newElevArray != 0){
				newElevArray->InsertNextValue(origElevArray->GetValue(nodeIndex(0, j, 0)));
			}
		}
		for (int k = 1; k < dims[2]; ++k){
			double p[3];
			sGrid->GetPoint(nodeIndex(0, 0, k), p);
			points->InsertNextPoint(p);
			indexArray->InsertNextValue(iRIC::toStr(label.arg(1).arg(1).arg(k + 1)).c_str());
			if (newElevArray != 0){
				newElevArray->InsertNextValue(origElevArray->GetValue(nodeIndex(0, 0, k)));
			}
		}
		m_labelData->SetPoints(points);
		m_labelData->GetPointData()->AddArray(indexArray);
		if (newElevArray != 0){
			m_labelData->GetPointData()->AddArray(newElevArray);
		}
	} else {
		// unstructured grid.
		QString label("(%1)");
		for (int i = 0; i < m_data->GetNumberOfPoints(); ++i){
			indexArray->InsertNextValue(iRIC::toStr(label.arg(i + 1)).c_str());
		}
		m_labelData->GetPointData()->AddArray(indexArray);
	}
	return true;
}

void PostZoneDataContainer::loadFromCgnsFile(const int fn)
{
	m_loadOK = true;
	int currentStep = dynamic_cast<PostSolutionInfo*>(parent())->currentStep();

	bool ret;
	// set baseId.
	ret = setBaseId(fn);
	if (ret == false){goto ERROR;}
	// set zoneId.
	ret = setZoneId(fn);
	if (ret == false){goto ERROR;}
	// first, check whether structured or unstructured.
	ZoneType_t type;
	cg_zone_type(fn, m_baseId, m_zoneId, &type);
	ret = loadZoneSize(fn);
	if (ret == false){goto ERROR;}
	if (type == Structured){
		ret = loadStructuredGrid(fn, currentStep);
		if (ret == false){goto ERROR;}
	}else{
		ret = loadUnstructuredGrid(fn, currentStep);
		if (ret == false){goto ERROR;}
	}
	// load solution data.
	int solid;
	ret = getSoluionId(fn, currentStep, &solid);
	if (ret == false){goto ERROR;}
	ret = loadScalarData(fn, solid);
	if (ret == false){goto ERROR;}
	ret = loadVectorData(fn, solid);
	if (ret == false){goto ERROR;}
	ret = loadCellFlagData(fn);
	if (ret == false){goto ERROR;}
	ret = setupIndexData();

	// load particles
	ret = loadParticle(fn, currentStep);

	m_data->Modified();

	m_loadedOnce = true;
	emit dataUpdated();
	return;
ERROR:
	m_loadOK = false;
	m_data = 0;
}

void PostZoneDataContainer::loadIfEmpty(const int fn)
{
	if (m_data != 0){return;}
	loadFromCgnsFile(fn);
}

int PostZoneDataContainer::nodeIndex(int i, int j, int k) const
{
	int dim[3];
	vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(m_data);
	grid->GetDimensions(dim);
	return i + dim[0] * (j + dim[1] * k);
}

void PostZoneDataContainer::getNodeIJKIndex(int index, int *i, int *j, int *k) const
{
	int dim[3];
	vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(m_data);
	grid->GetDimensions(dim);

	*i = index % (dim[0]);
	*j = ((index - *i) / dim[0]) % dim[1];
	*k = index / (dim[0] * dim[1]);
}

int PostZoneDataContainer::cellIndex(int i, int j, int k) const
{
	int dim[3];
	vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(m_data);
	grid->GetDimensions(dim);
	return i + (dim[0] - 1) * (j + (dim[1] - 1) * k);
}

void PostZoneDataContainer::getCellIJKIndex(int index, int *i, int *j, int *k) const
{
	int dim[3];
	vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(m_data);
	grid->GetDimensions(dim);

	*i = index % (dim[0] - 1);
	*j = ((index - *i) / dim[0] - 1) % (dim[1] - 1);
	*k = index / ((dim[0] - 1) * (dim[1] - 1));
}

bool PostZoneDataContainer::scalarValueExists() const
{
	vtkPointData* data = m_data->GetPointData();
	int n = data->GetNumberOfArrays();
	for (int i = 0; i < n; ++i){
		vtkDataArray* array = data->GetArray(i);
		if (array == 0){continue;}
		if (array->GetNumberOfComponents() == 1){
			// This is a scalar value.
			return true;
		}
	}
	return false;
}

bool PostZoneDataContainer::vectorValueExists() const
{
	vtkPointData* data = m_data->GetPointData();
	int n = data->GetNumberOfArrays();
	for (int i = 0; i < n; ++i){
		vtkDataArray* array = data->GetArray(i);
		if (array == 0){continue;}
		if (array->GetNumberOfComponents() == 3){
			// This is a vector value.
			return true;
		}
	}
	return false;
}

bool PostZoneDataContainer::IBCExists()
{
	vtkPointData* data = m_data->GetPointData();
	int n = data->GetNumberOfArrays();
	for (int i = 0; i < n; ++i){
		vtkDataArray* array = data->GetArray(i);
		if (array == 0){continue;}
		if (IBC == array->GetName()){return true;}
	}
	return false;
}

bool PostZoneDataContainer::saveToVTKFile(const QString& filename, double time, int imin, int imax, int jmin, int jmax, int kmin, int kmax)
{
	QString dir = projectData()->workDirectory();
	// export to a temporary file first.
	QString tmpFile = iRIC::getTempFileName(dir);

	vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast(m_data);
	if (sgrid != 0){
		vtkStructuredGridWriter* writer = vtkStructuredGridWriter::New();
		vtkExtractGrid* extract = vtkExtractGrid::New();
		extract->SetVOI(imin, imax, jmin, jmax, kmin, kmax);
		extract->SetInputData(sgrid);
		QString header("iRIC output t = %1");
		writer->SetHeader(iRIC::toStr(header.arg(time)).c_str());
		writer->SetInputConnection(extract->GetOutputPort());
		writer->SetFileName(iRIC::toStr(tmpFile).c_str());
		writer->Update();
		writer->Delete();
		extract->Delete();
	}
	vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(m_data);
	if (ugrid != 0){
		vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
		QString header("iRIC output t = %1");
		writer->SetHeader(iRIC::toStr(header.arg(time)).c_str());
		writer->SetInputData(ugrid);
		writer->SetFileName(iRIC::toStr(tmpFile).c_str());
		writer->Update();
		writer->Delete();
	}
	if (QFile::exists(filename)){
		bool ok = QFile::remove(filename);
		if (! ok){
			// error occured while removing old file.
			QFile::remove(tmpFile);
			return false;
		}
	}
	bool ok = QFile::rename(tmpFile, filename);
	if (! ok){
		QFile::remove(tmpFile);
		return false;
	}
	return true;
}

bool PostZoneDataContainer::saveToCSVFile(const QString& filename, double time, int imin, int imax, int jmin, int jmax, int kmin, int kmax)
{
	if (QFile::exists(filename)){
		bool ok = QFile::remove(filename);
		if (! ok){
			return false;
		}
	}

	QFile f(filename);
	bool ok = f.open(QIODevice::WriteOnly);
	if (! ok){return false;}
	QTextStream stream(&f);

	vtkPointData* pData = m_data->GetPointData();
	vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast(m_data);

	if (sgrid != 0){
		int dim[3];
		sgrid->GetDimensions(dim);
		stream << "iRIC output t = " << time << "\r\n";
		stream << dim[0];
		if (dim[1] != 1){stream << "," << dim[1];}
		if (dim[2] != 1){stream << "," << dim[2];}
		stream << "\r\n";
		// header
		stream << "I";
		if (dim[1] != 1){stream << ",J";}
		if (dim[2] != 1){stream << ",K";}
		stream << ",X";
		stream << ",Y";
		if (dim[2] != 1){stream << ",Z";}
		for (int i = 0; i < pData->GetNumberOfArrays(); ++i){
			vtkDataArray* array = pData->GetArray(i);
			int comps = array->GetNumberOfComponents();
			QString name = pData->GetArrayName(i);
			if (comps == 1){
				stream << "," << name;
			} else if (comps == 2){
				stream << "," << name << "X," << name << "Y";
			} else if (comps == 3){
				stream << "," << name << "X," << name << "Y";
				if (dim[2] != 1){
					stream << "," << name << "Z";
				}
			}
		}
		stream << "\r\n";

		stream.setRealNumberNotation(QTextStream::ScientificNotation);
		// data
		for (int k = kmin; k <= kmax; ++k){
			for (int j = jmin; j <= jmax; ++j){
				for (int i = imin; i <= imax; ++i){
					stream << (i + 1);
					if (dim[1] != 1){stream << "," << (j + 1);}
					if (dim[2] != 1){stream << "," << (k + 1);}
					double pos[3];
					sgrid->GetPoint(nodeIndex(i, j, k), pos);
					stream.setRealNumberPrecision(15);
					stream << "," << pos[0] << "," << pos[1];
					if (dim[2] != 1){stream << "," << pos[2];}
					stream.setRealNumberPrecision(8);
					for (int l = 0; l < pData->GetNumberOfArrays(); ++l){
						vtkDataArray* array = pData->GetArray(l);
						int comps = array->GetNumberOfComponents();
						if (comps == 1){
							stream << "," << array->GetTuple1(nodeIndex(i, j, k));
						} else if (comps == 2){
							double* tuples = array->GetTuple2(nodeIndex(i, j, k));
							stream << "," << *tuples << "," << *(tuples + 1);
						} else if (comps == 3){
							double* tuples = array->GetTuple3(nodeIndex(i, j, k));
							stream << "," << *tuples << "," << *(tuples + 1);
							if (dim[2] != 1){
								stream << "," << *(tuples + 2);
							}
						}
					}
					stream << "\r\n";
				}
			}
		}
	}
	vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(m_data);
	if (ugrid != 0){
		stream << "iRIC output t = " << time << "\r\n";
		stream << ugrid->GetNumberOfPoints();
		stream << "\r\n";
		// header
		stream << "I";
		stream << ",X";
		stream << ",Y";
		for (int i = 0; i < pData->GetNumberOfArrays(); ++i){
			vtkDataArray* array = pData->GetArray(i);
			if (array == 0){continue;}
			int comps = array->GetNumberOfComponents();
			QString name = pData->GetArrayName(i);
			if (comps == 1){
				stream << "," << name;
			} else if (comps == 2){
				stream << "," << name << "X," << name << "Y";
			} else if (comps == 3){
				stream << "," << name << "X," << name << "Y";
			}
		}
		stream << "\r\n";
		// data
		for (int i = 0; i < ugrid->GetNumberOfPoints(); ++i){
			stream << i;
			double pos[3];
			ugrid->GetPoint(i, pos);
			// unstructured grid is limited to two-dimensional.
			stream << "," << pos[0] << "," << pos[1];
			for (int l = 0; l < pData->GetNumberOfArrays(); ++l){
				vtkDataArray* array = pData->GetArray(l);
				if (array == 0){continue;}
				int comps = array->GetNumberOfComponents();
				if (comps == 1){
					stream << "," << array->GetTuple1(i);
				} else if (comps == 2){
					double* tuples = array->GetTuple2(i);
					stream << "," << *tuples << "," << *(tuples + 1);
				} else if (comps == 3){
					double* tuples = array->GetTuple3(i);
					stream << "," << *tuples << "," << *(tuples + 1);
				}
			}
			stream << "\r\n";
		}
	}
	f.close();
	return true;
}

const QString PostZoneDataContainer::elevationName()
{
	int num = m_data->GetPointData()->GetNumberOfArrays();
	for (int i = 0; i < num; ++i){
		QString tmpname = m_data->GetPointData()->GetAbstractArray(i)->GetName();
		if (tmpname.toLower().left(9) == "elevation"){
			return tmpname;
		}
	}
	return "";
}

vtkPolyData* PostZoneDataContainer::filteredData(double xmin, double xmax, double ymin, double ymax, bool& masked) const
{
	if (vtkStructuredGrid::SafeDownCast(m_data) != 0){
		// Structured Data
		return filteredDataStructured(xmin, xmax, ymin, ymax, masked);
	} else {
		// Unstructured Data
		return filteredDataUnstructured(xmin, xmax, ymin, ymax, masked);
	}
}

vtkPolyData* PostZoneDataContainer::filteredDataStructured(double xmin, double xmax, double ymin, double ymax, bool& masked) const
{
	masked = false;

	double xcenter = (xmin + xmax) * 0.5;
	double ycenter = (ymin + ymax) * 0.5;

	double xwidth = (xmax - xmin);
	double ywidth = (ymax - ymin);

	xmin -= xwidth * 0.2;
	xmax += xwidth * 0.2;
	ymin -= ywidth * 0.2;
	ymax += ywidth * 0.2;

	// 1. Find the grid vertex that is the nearest to the region center.
	vtkIdType vid = m_data->FindPoint(xcenter, ycenter, 0);
	double *cv = m_data->GetPoint(vid);
	if (*cv < xmin || *cv > xmax || *(cv + 1) < ymin || *(cv + 1) > ymax){
		// 2. If the point is out of the region, the whole grid is out of the region.
		vtkSmartPointer<vtkPolyData> emptyPoly = vtkSmartPointer<vtkPolyData>::New();
		emptyPoly->SetPoints(m_data->GetPoints());
		emptyPoly->Register(0);
		return emptyPoly;
	}

	RectRegion region(xmin, xmax, ymin, ymax);

	int centerI, centerJ, centerK;

	getNodeIJKIndex(vid, &centerI, &centerJ, &centerK);
	int lineLimitIMin, lineLimitIMax, lineLimitJMin, lineLimitJMax;
	double tmpv[3];

	// test I = 0
	m_data->GetPoint(nodeIndex(0, centerJ, 0), tmpv);
	if (region.pointIsInside(tmpv[0], tmpv[1])){
		lineLimitIMin = 0;
	} else {
		lineLimitIMin = lineLimitI(centerJ, centerI, 0, region);
	}
	// test I = imax
	m_data->GetPoint(nodeIndex(m_sizes[0] - 1, centerJ, 0), tmpv);
	if (region.pointIsInside(tmpv[0], tmpv[1])){
		lineLimitIMax = m_sizes[0] - 1;
	} else {
		lineLimitIMax = lineLimitI(centerJ, centerI, m_sizes[0] - 1, region);
	}
	// test J = 0
	m_data->GetPoint(nodeIndex(centerI, 0, 0), tmpv);
	if (region.pointIsInside(tmpv[0], tmpv[1])){
		lineLimitJMin = 0;
	} else {
		lineLimitJMin = lineLimitJ(centerI, centerJ, 0, region);
	}
	// test J = jmax
	m_data->GetPoint(nodeIndex(centerI, m_sizes[1] - 1, 0), tmpv);
	if (region.pointIsInside(tmpv[0], tmpv[1])){
		lineLimitJMax = m_sizes[1] - 1;
	} else {
		lineLimitJMax = lineLimitJ(centerI, centerJ, m_sizes[1] - 1, region);
	}

	int lineLimitIMin2, lineLimitIMax2, lineLimitJMin2, lineLimitJMax2;

	// test I min direction
	if (lineLimitIMin == 0){
		lineLimitIMin2 = 0;
	} else {
		if (lineAtIIntersect(0, region)){
			lineLimitIMin2 = 0;
		} else {
			lineLimitIMin2 = lineLimitI2(lineLimitIMin, 0, region);
		}
	}
	// test I max direction
	if (lineLimitIMax == m_sizes[0] - 1){
		lineLimitIMax2 = m_sizes[0] - 1;
	} else {
		if (lineAtIIntersect(m_sizes[0] - 1, region)){
			lineLimitIMax2 = m_sizes[0] - 1;
		} else {
			lineLimitIMax2 = lineLimitI2(lineLimitIMax, m_sizes[0] - 1, region);
		}
	}

	// test J min direction
	if (lineLimitJMin == 0){
		lineLimitJMin2 = 0;
	} else {
		if (lineAtJIntersect(0, region)){
			lineLimitJMin2 = 0;
		} else {
			lineLimitJMin2 = lineLimitJ2(lineLimitJMin, 0, region);
		}
	}
	// test J max direction
	if (lineLimitJMax == m_sizes[1] - 1){
		lineLimitJMax2 = m_sizes[1] - 1;
	} else {
		if (lineAtJIntersect(m_sizes[1] - 1, region)){
			lineLimitJMax2 = m_sizes[1] - 1;
		} else {
			lineLimitJMax2 = lineLimitI2(lineLimitJMax, m_sizes[1] - 1, region);
		}
	}

	vtkSmartPointer<vtkExtractGrid> exGrid = vtkSmartPointer<vtkExtractGrid>::New();
	exGrid->SetVOI(lineLimitIMin2, lineLimitIMax2, lineLimitJMin2, lineLimitJMax2, 0, 0);
	exGrid->SetInputData(vtkStructuredGrid::SafeDownCast(m_data));
	exGrid->Update();
	vtkSmartPointer<vtkStructuredGrid> extractedGrid = exGrid->GetOutput();
	int exRate = 1;
	while (extractedGrid->GetNumberOfCells() > Grid::MAX_DRAWCELLCOUNT){
		exRate *= 2;
		exGrid->SetSampleRate(exRate, exRate, 1);
		exGrid->Update();
		extractedGrid = exGrid->GetOutput();
		masked = true;
	}

	vtkSmartPointer<vtkGeometryFilter> geo = vtkSmartPointer<vtkGeometryFilter>::New();
	geo->SetInputConnection(exGrid->GetOutputPort());

	geo->Update();
	vtkPolyData* data = geo->GetOutput();
	data->Register(0);
	return data;
//	vtkSmartPointer<vtkPolyData> clippedGrid = geo->GetOutput();

	/*
	m_drawnIMin = lineLimitIMin2;
	m_drawnIMax = lineLimitIMax2;
	m_drawnJMin = lineLimitJMin2;
	m_drawnJMax = lineLimitJMax2;

	m_vtkFilteredShape = clippedGrid;
	m_vtkFilteredPoints = clippedGrid;
	m_vtkFilteredCells = clippedGrid;

	int tmpIMin = (m_drawnIMin / exRate) * exRate;
	int tmpIMax = qMin((m_drawnIMax / exRate + 1) * exRate, m_dimensionI - 1);
	int tmpJMin = (m_drawnJMin / exRate) * exRate;
	int tmpJMax = qMin((m_drawnJMax / exRate + 1) * exRate, m_dimensionJ - 1);

	m_vtkFilteredIndexGrid = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPoints> igPoints = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> ca = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkStringArray> sa = vtkSmartPointer<vtkStringArray>::New();
	sa->SetName(Grid::LABEL_NAME);
	vtkIdType cellid = 0;
	double tmpp[3];
	QString label("(%1,%2)");
	for (int i = tmpIMin; i <= tmpIMax; i += exRate){
		m_vtkGrid->GetPoint(vertexIndex(i, 0), tmpp);
		igPoints->InsertNextPoint(tmpp);
		ca->InsertNextCell(1, &cellid);
		sa->InsertNextValue(iRIC::toStr(label.arg(i + 1).arg(1)));
		++ cellid;
	}
	for (int j = tmpJMin; j <= tmpJMax; j += exRate){
		m_vtkGrid->GetPoint(vertexIndex(0, j), tmpp);
		igPoints->InsertNextPoint(tmpp);
		ca->InsertNextCell(1, &cellid);
		sa->InsertNextValue(iRIC::toStr(label.arg(1).arg(j + 1)));
		++ cellid;
	}
	m_vtkFilteredIndexGrid->SetPoints(igPoints);
	m_vtkFilteredIndexGrid->SetVerts(ca);
	m_vtkFilteredIndexGrid->GetPointData()->AddArray(sa);
	*/
}

vtkPolyData* PostZoneDataContainer::filteredDataUnstructured(double xmin, double xmax, double ymin, double ymax, bool& masked) const
{
	double xwidth = xmax - xmin;
	double ywidth = ymax - ymin;
	masked = false;

	vtkSmartPointer<vtkGeometryFilter> gfilter = vtkSmartPointer<vtkGeometryFilter>::New();
	gfilter->SetExtent(xmin - xwidth * 0.2, xmax + xwidth * 0.2, ymin - ywidth * 0.2, ymax + ywidth * 0.2, -1, 1);
	gfilter->ExtentClippingOn();
	gfilter->SetInputData(m_data);
	gfilter->Update();
	vtkPolyData* data = gfilter->GetOutput();
	data->Register(0);
	return data;
}

int PostZoneDataContainer::lineLimitI(int j, int iIn, int iOut, const RectRegion& region) const
{
	if (qAbs(iOut - iIn) == 1){
		return iIn;
	}
	int i = (iIn + iOut) / 2;
	double tmpv[3];
	m_data->GetPoint(nodeIndex(i, j, 0), tmpv);
	if (region.pointIsInside(tmpv[0], tmpv[1])){
		return lineLimitI(j, i, iOut, region);
	} else {
		return lineLimitI(j, iIn, i, region);
	}
}

int PostZoneDataContainer::lineLimitJ(int i, int jIn, int jOut, const RectRegion& region) const
{
	if (qAbs(jOut - jIn) == 1){
		return jIn;
	}
	int j = (jIn + jOut) / 2;
	double tmpv[3];
	m_data->GetPoint(nodeIndex(i, j, 0), tmpv);
	if (region.pointIsInside(tmpv[0], tmpv[1])){
		return lineLimitJ(i, j, jOut, region);
	} else {
		return lineLimitJ(i, jIn, j, region);
	}
}

int PostZoneDataContainer::lineLimitI2(int iIn, int iOut, const RectRegion& region) const
{
	if (qAbs(iOut - iIn) == 1){
		return iIn;
	}
	int i = (iIn + iOut) / 2;
	if (lineAtIIntersect(i, region)){
		return lineLimitI2(i, iOut, region);
	} else {
		return lineLimitI2(iIn, i, region);
	}
}

int PostZoneDataContainer::lineLimitJ2(int jIn, int jOut, const RectRegion& region) const
{
	if (qAbs(jOut - jIn) == 1){
		return jIn;
	}
	int j = (jIn + jOut) / 2;
	if (lineAtJIntersect(j, region)){
		return lineLimitJ2(j, jOut, region);
	} else {
		return lineLimitJ2(jIn, j, region);
	}
}

bool PostZoneDataContainer::lineAtIIntersect(int i, const RectRegion& region) const
{
	QPointF p1, p2;
	double tmpv[3];
	m_data->GetPoint(nodeIndex(i, 0, 0), tmpv);
	p1 = QPointF(tmpv[0], tmpv[1]);
	for (int j = 1; j < m_sizes[1]; ++j){
		m_data->GetPoint(nodeIndex(i, j, 0), tmpv);
		p2 = QPointF(tmpv[0], tmpv[1]);
		QLineF line(p1, p2);
		if (region.intersect(line)){return true;}
	}
	return false;
}

bool PostZoneDataContainer::lineAtJIntersect(int j, const RectRegion& region) const
{
	QPointF p1, p2;
	double tmpv[3];
	m_data->GetPoint(nodeIndex(0, j, 0), tmpv);
	p1 = QPointF(tmpv[0], tmpv[1]);
	for (int i = 1; i < m_sizes[0]; ++i){
		m_data->GetPoint(nodeIndex(i, j, 0), tmpv);
		p2 = QPointF(tmpv[0], tmpv[1]);
		QLineF line(p1, p2);
		if (region.intersect(line)){return true;}
	}
	return false;
}