#ifndef POSTDUMMY3DZONEDATACONTAINER_H
#define POSTDUMMY3DZONEDATACONTAINER_H

#include "../guicore_global.h"
#include "postzonedatacontainer.h"

class GUICOREDLL_EXPORT PostDummy3DZoneDataContainer : public PostZoneDataContainer
{

public:
	PostDummy3DZoneDataContainer(const std::string& baseName, const std::string& zoneName, SolverDefinitionGridType* gridtype, ProjectDataItem* parent) :
		PostZoneDataContainer(baseName, zoneName, gridtype, parent)
	{}

protected:
	bool loadStructuredGrid(const int fn, const int currentStep) override;
	bool loadGridScalarData(const int fn, const int solid) override;
	bool loadGridVectorData(const int fn, const int solid) override;

public:
	static const int KMAX = 11;
	static const int ZWIDTH = 1;
};

#endif // POSTDUMMY3DZONEDATACONTAINER_H
