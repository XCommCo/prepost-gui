#include "../post3dwindowdatamodel.h"
#include "post3dwindowgridtypedataitem.h"
#include "post3dwindownodevectorparticledataitem.h"
#include "post3dwindownodevectorparticlegroupdataitem.h"
#include "post3dwindowzonedataitem.h"

#include <guicore/base/iricmainwindowinterface.h>
#include <guicore/postcontainer/postsolutioninfo.h>
#include <guicore/postcontainer/posttimesteps.h>
#include <guicore/postcontainer/postzonedatacontainer.h>
#include <guicore/project/projectdata.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <misc/filesystemfunction.h>
#include <misc/iricundostack.h>
#include <misc/stringtool.h>

#include <QSettings>
#include <QStandardItem>
#include <QUndoCommand>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkRenderer.h>
#include <vtkRungeKutta4.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkVertex.h>

Post3dWindowNodeVectorParticleGroupDataItem::Post3dWindowNodeVectorParticleGroupDataItem(Post3dWindowDataItem* p)
	: Post3dWindowDataItem(tr("Particles"), QIcon(":/libs/guibase/images/iconFolder.png"), p)
{
	m_isDeletable = false;
	m_standardItem->setCheckable(true);
	m_standardItem->setCheckState(Qt::Checked);

	m_standardItemCopy = m_standardItem->clone();
	m_previousStep = -2;
	m_previousTime = 0;
	m_nextStepToAddParticles = 0;
	m_zScale = 1;

	setDefaultValues();
	setupClipper();
	informGridUpdate();

	PostZoneDataContainer* cont = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	SolverDefinitionGridType* gt = cont->gridType();
	vtkPointData* pd = cont->data()->GetPointData();
	int number = pd->GetNumberOfArrays();
	for (int i = 0; i < number; i++){
		vtkAbstractArray* tmparray = pd->GetArray(i);
		if (tmparray == 0){continue;}
		if (tmparray->GetNumberOfComponents() == 1){
			// vector attribute.
			continue;
		}
		QString name = pd->GetArray(i)->GetName();
		Post3dWindowNodeVectorParticleDataItem* item = new Post3dWindowNodeVectorParticleDataItem(name, gt->solutionCaption(name), this);
		m_childItems.append(item);
	}
}

Post3dWindowNodeVectorParticleGroupDataItem::~Post3dWindowNodeVectorParticleGroupDataItem()
{
	for (int i = 0; i < m_particleActors.count(); ++i){
		renderer()->RemoveActor(m_particleActors[i]);
		m_particleActors[i]->Delete();
	}
	for (int i = 0; i < m_particleMappers.count(); ++i){
		m_particleMappers[i]->Delete();
	}
	for (int i = 0; i < m_particleGrids.count(); ++i){
		m_particleGrids[i]->Delete();
	}
}

class Post3dWindowGridParticleSelectSolution : public QUndoCommand
{
public:
	Post3dWindowGridParticleSelectSolution(const QString& newsol, Post3dWindowNodeVectorParticleGroupDataItem* item)
		: QUndoCommand(QObject::tr("Particle Physical Value Change"))
	{
		m_newCurrentSolution = newsol;
		m_oldCurrentSolution = item->m_currentSolution;
		m_item = item;
	}
	void undo()
	{
		m_item->setIsCommandExecuting(true);
		m_item->setCurrentSolution(m_oldCurrentSolution);
		m_item->updateActorSettings();
		m_item->renderGraphicsView();
		m_item->setIsCommandExecuting(false);
	}
	void redo()
	{
		m_item->setIsCommandExecuting(true);
		m_item->setCurrentSolution(m_newCurrentSolution);
		m_item->updateActorSettings();
		m_item->renderGraphicsView();
		m_item->setIsCommandExecuting(false);
	}
private:
	QString m_oldCurrentSolution;
	QString m_newCurrentSolution;

	Post3dWindowNodeVectorParticleGroupDataItem* m_item;
};


void Post3dWindowNodeVectorParticleGroupDataItem::exclusivelyCheck(Post3dWindowNodeVectorParticleDataItem* item)
{
	if (m_isCommandExecuting){return;}
	iRICUndoStack& stack = iRICUndoStack::instance();
	if (item->standardItem()->checkState() != Qt::Checked){
		stack.push(new Post3dWindowGridParticleSelectSolution("", this));
	}else{
		stack.push(new Post3dWindowGridParticleSelectSolution(item->name(), this));
	}
}

void Post3dWindowNodeVectorParticleGroupDataItem::setDefaultValues()
{
	m_currentSolution= "";

	m_timeMode = tmNormal;
	m_timeSamplingRate = 2;
	m_timeDivision = 2;
	m_regionMode = StructuredGridRegion::rmFull;
}

void Post3dWindowNodeVectorParticleGroupDataItem::updateActorSettings()
{
	for (int i = 0; i < m_particleActors.count(); ++i){
		renderer()->RemoveActor(m_particleActors[i]);
		m_particleActors[i]->Delete();
		m_particleMappers[i]->Delete();
	}
	m_actorCollection->RemoveAllItems();
	m_particleActors.clear();
	m_particleMappers.clear();
	m_particleGrids.clear();

	PostZoneDataContainer* cont = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	if (cont == 0 || cont->data() == 0){return;}
	if (m_currentSolution == ""){return;}
	vtkPointSet* ps = cont->data();
	vtkPointData* pd = ps->GetPointData();
	if (pd->GetNumberOfArrays() == 0){return;}

	setupActors();

	applyZScale();
	setupStreamTracer();
	setupParticleSources();
	resetParticles();
	updateVisibilityWithoutRendering();
}

void Post3dWindowNodeVectorParticleGroupDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomElement elem = node.toElement();
	setCurrentSolution(elem.attribute("solution"));
	m_timeMode = static_cast<TimeMode>(elem.attribute("timeMode").toInt());
	m_timeSamplingRate = elem.attribute("timeSamplingRate").toInt();
	m_timeDivision = elem.attribute("timeDivision").toInt();
	m_regionMode = static_cast<StructuredGridRegion::RegionMode>(elem.attribute("regionMode").toInt());
}

void Post3dWindowNodeVectorParticleGroupDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	writer.writeAttribute("solution", m_currentSolution);
	writer.writeAttribute("timeMode", QString::number(static_cast<int>(m_timeMode)));
	writer.writeAttribute("timeSamplingRate", QString::number(m_timeSamplingRate));
	writer.writeAttribute("timeDivision", QString::number(m_timeDivision));
	writer.writeAttribute("regionMode", QString::number(static_cast<int>(m_regionMode)));
}

void Post3dWindowNodeVectorParticleGroupDataItem::setupClipper(){
	m_IBCClipper = vtkSmartPointer<vtkClipPolyData>::New();
	m_IBCClipper->SetValue(PostZoneDataContainer::IBCLimit);
	m_IBCClipper->InsideOutOff();
	m_IBCClipper->SetInputArrayToProcess(0, 0, 0, 0, iRIC::toStr(PostZoneDataContainer::IBC).c_str());
}

void Post3dWindowNodeVectorParticleGroupDataItem::updateZDepthRangeItemCount()
{
	m_zDepthRange.setItemCount(3);
}

void Post3dWindowNodeVectorParticleGroupDataItem::assignActionZValues(const ZDepthRange& range)
{
	if (m_particleActors.count() == 0){return;}
	if (m_particleActors.count() == 1){
		m_particleActors[0]->SetPosition(0, 0, range.max());
		return;
	}
	for (int i = 0; i < m_particleActors.count(); ++i){
		double depth = range.min() + static_cast<double>(i) / (m_particleActors.count() - 1) * (range.max() - range.min());
		m_particleActors[i]->SetPosition(0, 0, depth);
	}
}


void Post3dWindowNodeVectorParticleGroupDataItem::setupStreamTracer()
{
	m_streamTracer = vtkSmartPointer<vtkStreamPoints>::New();
	m_streamPoints = vtkSmartPointer<vtkCustomStreamPoints>::New();

	m_streamTracer->SetInputData(getRegion());
	m_streamTracer->SetIntegrationDirectionToForward();

	m_streamPoints->SetInputData(getRegion());
	m_streamPoints->SetIntegrationDirectionToForward();
}

void Post3dWindowNodeVectorParticleGroupDataItem::informGridUpdate()
{
	for (int i = 0; i < m_particleActors.count(); ++i){
		renderer()->RemoveActor(m_particleActors[i]);
		m_particleActors[i]->Delete();
		m_particleMappers[i]->Delete();
	}
	m_actorCollection->RemoveAllItems();
	m_particleActors.clear();
	m_particleMappers.clear();

	if (m_standardItem->checkState() == Qt::Unchecked){return;}
	if (m_currentSolution == ""){return;}
	PostZoneDataContainer* zoneContainer = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	if (zoneContainer == 0){return;}
	unsigned int currentStep = 0;
	if (zoneContainer != 0){
		currentStep = zoneContainer->solutionInfo()->currentStep();
	}
	setupActors();
	applyZScale();
	if (zoneContainer == 0 || zoneContainer->data() == 0){
		resetParticles();
		goto TIMEHANDLING;
	}
	setupStreamTracer();
	setupParticleSources();
	if (currentStep != 0 && (currentStep == m_previousStep + 1 || projectData()->mainWindow()->continuousSnapshotInProgress())){
		// one increment add particles!
		addParticles();
	} else {
		// reset particles.
		resetParticles();
	}
	updateVisibilityWithoutRendering();

TIMEHANDLING:

	m_previousStep = currentStep;
	PostTimeSteps* tSteps = zoneContainer->solutionInfo()->timeSteps();
	if (m_previousStep < tSteps->timesteps().count()){
		m_previousTime = tSteps->timesteps().at(m_previousStep);
	}else{
		m_previousTime = 0;
	}
}

void Post3dWindowNodeVectorParticleGroupDataItem::update()
{
	informGridUpdate();
}

void Post3dWindowNodeVectorParticleGroupDataItem::setCurrentSolution(const QString& currentSol)
{
	QList<GraphicsWindowDataItem*>::iterator it;
	Post3dWindowNodeVectorParticleDataItem* current = 0;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		Post3dWindowNodeVectorParticleDataItem* tmpItem = dynamic_cast<Post3dWindowNodeVectorParticleDataItem*>(*it);
		if (tmpItem->name() == currentSol){
			current = tmpItem;
		}
		tmpItem->standardItem()->setCheckState(Qt::Unchecked);
	}
	if (current != 0){
		current->standardItem()->setCheckState(Qt::Checked);
	}
	m_currentSolution = currentSol;
}

void Post3dWindowNodeVectorParticleGroupDataItem::innerUpdateZScale(double zscale)
{
	m_zScale = zscale;
	applyZScale();
}

void Post3dWindowNodeVectorParticleGroupDataItem::applyZScale()
{
	for (int i = 0; i < m_particleActors.count(); ++i){
		m_particleActors[i]->SetScale(1, 1, m_zScale);
	}
}

void Post3dWindowNodeVectorParticleGroupDataItem::resetParticles()
{
	for (int i = 0; i < m_particleGrids.count(); ++i){
		m_particleGrids[i]->Delete();
	}
	m_particleGrids.clear();
	for (int i = 0; i < m_particleActors.count(); ++i){
		vtkPolyData* grid = vtkPolyData::New();
		vtkPointSet* pointsGrid = newParticles(i);
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		points->SetDataTypeToDouble();
		if (pointsGrid != 0){
			for (vtkIdType i = 0; i < pointsGrid->GetNumberOfPoints(); ++i){
				double p[3];
				pointsGrid->GetPoint(i, p);
				points->InsertNextPoint(p);
			}
		}
		grid->SetPoints(points);
		grid->Allocate(points->GetNumberOfPoints());
		vtkSmartPointer<vtkVertex> vertex = vtkSmartPointer<vtkVertex>::New();
		for (vtkIdType j = 0; j < points->GetNumberOfPoints(); ++j){
			vertex->GetPointIds()->SetId(0, j);
			grid->InsertNextCell(vertex->GetCellType(), vertex->GetPointIds());
		}
		grid->BuildLinks();
		grid->Modified();
		m_particleMappers[i]->SetInputData(grid);
		m_particleGrids.append(grid);
	}
	PostZoneDataContainer* zoneContainer = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	unsigned int currentStep = zoneContainer->solutionInfo()->currentStep();
	if (m_timeMode == tmSkip){
		m_nextStepToAddParticles = currentStep + m_timeSamplingRate;
	}else{
		m_nextStepToAddParticles = currentStep + 1;
	}
}

void Post3dWindowNodeVectorParticleGroupDataItem::addParticles()
{
	PostZoneDataContainer* zoneContainer = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	vtkPointSet* ps = zoneContainer->data();
	ps->GetPointData()->SetActiveVectors(iRIC::toStr(m_currentSolution).c_str());

	unsigned int currentStep = zoneContainer->solutionInfo()->currentStep();

	PostTimeSteps* tSteps = zoneContainer->solutionInfo()->timeSteps();
	QList<double> timeSteps = tSteps->timesteps();
	double timeDiv = timeSteps[currentStep] - m_previousTime;

	for (int i = 0; i < m_particleActors.count(); ++i){
		// Find the new positions of points already exists.
		m_streamPoints->SetSourceData(m_particleGrids[i]);
		m_streamPoints->SetMaximumPropagationTime(timeDiv * 1.1);
		m_streamPoints->SetTimeIncrement(timeDiv);
		m_streamPoints->Update();

		vtkPolyData* p = m_streamPoints->GetOutput();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		points->SetDataTypeToDouble();
		for (vtkIdType j = 0; j < p->GetNumberOfPoints(); ++j){
			double v[3];
			p->GetPoint(j, v);
			points->InsertNextPoint(v);
		}
		// add new particles.
		if (currentStep == m_nextStepToAddParticles){
			vtkPointSet* pointsGrid = newParticles(i);
			if (m_timeMode == tmSubdivide){
				for (int j = 0; j < m_timeDivision - 1; ++j){
					m_streamTracer->SetSourceData(pointsGrid);
					m_streamTracer->SetMaximumPropagationTime(timeDiv * (1 - 0.5 / m_timeDivision));
					m_streamTracer->SetTimeIncrement(timeDiv / m_timeDivision);
					m_streamTracer->Update();
					vtkPolyData* p = m_streamTracer->GetOutput();
					for (vtkIdType k = 0; k < p->GetNumberOfPoints(); ++k){
						double v[3];
						p->GetPoint(k, v);
						points->InsertNextPoint(v);
					}
				}
			} else {
				for (vtkIdType j = 0; j < pointsGrid->GetNumberOfPoints(); ++j){
					double v[3];
					pointsGrid->GetPoint(j, v);
					points->InsertNextPoint(v);
				}
			}
		}
		points->Modified();

		vtkPolyData* newPoints = vtkPolyData::New();
		newPoints->SetPoints(points);
		vtkIdType numPoints = points->GetNumberOfPoints();
		vtkSmartPointer<vtkCellArray> ca = vtkSmartPointer<vtkCellArray>::New();
		for (vtkIdType j = 0; j < numPoints; ++j){
			ca->InsertNextCell(1, &j);
		}
		newPoints->SetVerts(ca);
		newPoints->Modified();
		m_particleMappers[i]->SetInputData(newPoints);
		m_particleGrids[i]->Delete();
		m_particleGrids[i] = newPoints;
	}
	if (m_timeMode == tmSkip){
		if (currentStep == m_nextStepToAddParticles){
			m_nextStepToAddParticles = currentStep + m_timeSamplingRate;
		}
	} else {
		m_nextStepToAddParticles = currentStep + 1;
	}
}

bool Post3dWindowNodeVectorParticleGroupDataItem::exportParticles(const QString& filePrefix, int fileIndex, double time)
{
	for (int i = 0; i < m_particleGrids.count(); ++i){
		QString tempPath = QDir::tempPath();
		QString tmpFile = iRIC::getTempFileName(tempPath);

		vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
		QString header("iRIC particle output t = %1");
		writer->SetHeader(iRIC::toStr(header.arg(time)).c_str());
		writer->SetInputData(m_particleGrids[i]);
		writer->SetFileTypeToASCII();
		writer->SetFileName(iRIC::toStr(tmpFile).c_str());

		// export data.
		writer->Update();
		writer->Delete();

		QString filename = filePrefix;
		if (m_particleGrids.count() == 1){
			filename.append(QString("%1.vtk").arg(fileIndex));
		} else {
			filename.append(QString("Group%1_%2.vtk").arg(i + 1).arg(fileIndex));
		}
		// rename the temporary file to the target file.
		if (QFile::exists(filename)){
			// remove first.
			if (! QFile::remove(filename)){
				// unable to remove. fail.
				QFile::remove(tmpFile);
				return false;
			}
		}
		bool ok = QFile::rename(tmpFile, filename);
		if (! ok){
			// rename failed.
			QFile::remove(tmpFile);
			return false;
		}
	}
	return true;
}

vtkPointSet* Post3dWindowNodeVectorParticleGroupDataItem::getRegion()
{
	vtkPointSet* ps = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer()->data();
	if (m_regionMode == StructuredGridRegion::rmFull){
		return ps;
	} else if (m_regionMode == StructuredGridRegion::rmActive){
		vtkSmartPointer<vtkStructuredGridGeometryFilter> geoFilter = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
		geoFilter->SetInputData(ps);
		geoFilter->Update();
		ps->GetPointData()->SetActiveScalars(iRIC::toStr(PostZoneDataContainer::IBC).c_str());
		m_IBCClipper->SetInputConnection(geoFilter->GetOutputPort());
		m_IBCClipper->Update();
		m_regionClippedPolyData = vtkSmartPointer<vtkPolyData>::New();
		m_regionClippedPolyData->DeepCopy(m_IBCClipper->GetOutput());
		m_regionClippedPolyData->GetPointData()->SetActiveScalars("");
		return m_regionClippedPolyData;
	}
	return 0;
}