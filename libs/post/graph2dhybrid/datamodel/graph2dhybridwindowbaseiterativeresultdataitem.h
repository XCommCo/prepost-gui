#ifndef GRAPH2DHYBRIDWINDOWBASEITERATIVERESULTDATAITEM_H
#define GRAPH2DHYBRIDWINDOWBASEITERATIVERESULTDATAITEM_H

#include "../graph2dhybridwindowdataitem.h"
#include "../graph2dhybridwindowresultsetting.h"
#include "graph2dhybridwindowresultdataitem.h"

class PostBaseIterativeSeriesDataContainer;
class QColor;

class Graph2dHybridWindowBaseIterativeResultDataItem : public Graph2dHybridWindowResultDataItem
{
public:
	Graph2dHybridWindowBaseIterativeResultDataItem(const Graph2dHybridWindowResultSetting::Setting& setting, int index, Graph2dWindowDataItem* parent);
	virtual ~Graph2dHybridWindowBaseIterativeResultDataItem();
	void doLoadFromProjectMainFile(const QDomNode& node);
	void doSaveToProjectMainFile(QXmlStreamWriter& writer);
	Graph2dHybridWindowResultCopyDataItem* copy(Graph2dHybridWindowResultCopyGroupDataItem* parent);
protected:
	void updateValues(int fn);
private:
	PostBaseIterativeSeriesDataContainer* m_dataContainer;
};

#endif // GRAPH2DHYBRIDWINDOWBASEITERATIVERESULTDATAITEM_H