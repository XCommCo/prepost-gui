#include "polylinecontroller.h"

#include "private/polylinecontroller_impl.h"

#include <QPointF>
#include <QVector2D>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

PolyLineController::Impl::Impl() :
	m_lineActor {}
{}

PolyLineController::Impl::~Impl()
{}

PolyLineController::PolyLineController() :
	impl {new Impl {}}
{}

PolyLineController::~PolyLineController()
{
	delete impl;
}

std::vector<QPointF> PolyLineController::polyLine() const
{
	return impl->m_lineActor.line();
}

void PolyLineController::setPolyLine(const std::vector<QPointF>& polyLine)
{
	impl->m_lineActor.setLine(polyLine);
}

bool PolyLineController::isVertexSelectable(const QPointF& pos, double limitDistance, int* vid)
{
	auto polydata = impl->m_lineActor.pointsPolyData();
	*vid = polydata->FindPoint(pos.x(), pos.y(), 0.0);

	double v[3];
	polydata->GetPoint(static_cast<vtkIdType>(*vid), v);
	QVector2D diff(pos.x() - v[0], pos.y() - v[1]);

	return diff.lengthSquared() < limitDistance * limitDistance;
}

bool PolyLineController::isEdgeSelectable(const QPointF& pos, double limitDistance, int* edgeId)
{
	double x[3] = {pos.x(), pos.y(), 0};
	int subId;
	double pcoords[3];
	double weights[32];

	double d2 = limitDistance * limitDistance;

	vtkIdType id = impl->m_lineActor.linesPolyData()->FindCell(x, NULL, 0, d2, subId, pcoords, weights);
	if (id < 0) {return false;}

	*edgeId = id;
	return true;
}

vtkPolyData* PolyLineController::polyData() const
{
	return impl->m_lineActor.pointsPolyData();
}

vtkActor* PolyLineController::pointsActor() const
{
	return impl->m_lineActor.pointsActor();
}

vtkActor* PolyLineController::linesActor() const
{
	return impl->m_lineActor.lineActor();
}
