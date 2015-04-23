#include "rawdatapolygonregionpolygon.h"

#include <vtkProperty.h>

RawDataPolygonRegionPolygon::RawDataPolygonRegionPolygon(RawDataPolygon* parent)
	: RawDataPolygonAbstractPolygon(parent)
{
	m_paintActor->GetProperty()->SetColor(0, 0, 0);
	m_paintActor->GetProperty()->SetOpacity(0.);
	m_edgeActor->GetProperty()->SetColor(0, 0, 0);
	m_vertexActor->GetProperty()->SetColor(0, 0, 0);
}