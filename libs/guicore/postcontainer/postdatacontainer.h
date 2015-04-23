#ifndef POSTDATACONTAINER_H
#define POSTDATACONTAINER_H

#include "../guicore_global.h"
#include "../project/projectdataitem.h"
#include <QObject>

class PostSolutionInfo;

class GUICOREDLL_EXPORT PostDataContainer : public ProjectDataItem
{
		Q_OBJECT
public:
	PostDataContainer(ProjectDataItem *parent);
	virtual bool handleCurrentStepUpdate(const int /*fn*/){return true;}
	PostSolutionInfo* solutionInfo();
signals:
	void dataUpdated();
};

#endif // POSTDATACONTAINER_H