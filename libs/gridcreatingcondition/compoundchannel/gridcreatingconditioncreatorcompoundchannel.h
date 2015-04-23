#ifndef GRIDCREATINGCONDITIONCREATORCOMPOUNDCHANNEL_H
#define GRIDCREATINGCONDITIONCREATORCOMPOUNDCHANNEL_H

#include "gcc_compoundchannel_global.h"
#include <guicore/pre/gridcreatingcondition/gridcreatingconditioncreator.h>

class GCC_COMPOUNDCHANNEL_EXPORT GridCreatingConditionCreatorCompoundChannel : public GridCreatingConditionCreator
{
	Q_OBJECT
public:
	GridCreatingConditionCreatorCompoundChannel();
	~GridCreatingConditionCreatorCompoundChannel(){}
	// temporary
	SolverDefinitionGridType::GridType gridType() const {return SolverDefinitionGridType::gtStructured2DGrid;}
	GridCreatingCondition* create(ProjectDataItem* parent);
};

#endif // GRIDCREATINGCONDITIONCREATORCOMPOUNDCHANNEL_H