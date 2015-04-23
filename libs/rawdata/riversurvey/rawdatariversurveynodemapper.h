#ifndef RAWDATARIVERSURVEYNODEMAPPER_H
#define RAWDATARIVERSURVEYNODEMAPPER_H

#include <guicore/pre/rawdata/rawdatanodemappert.h>
#include "rawdatariversurvey.h"
#include <vtkCell.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <misc/doublemappingsetting.h>

class RawDataRiverSurveyNodeMapper : public RawDataNodeMapperT<double, vtkDoubleArray>
{
public:
	RawDataRiverSurveyNodeMapper(RawDataCreator* parent)
		: RawDataNodeMapperT<double, vtkDoubleArray>(parent)
	{
		RawDataNodeMapperT<double, vtkDoubleArray>::m_caption = "River survey node mapper";
	}
	RawDataMapperSetting* initialize(bool* boolMap);
	void map(bool* boolMap, RawDataMapperSetting* s);
	void terminate(RawDataMapperSetting* s);
};

#endif // RAWDATARIVERSURVEYNODEMAPPER_H