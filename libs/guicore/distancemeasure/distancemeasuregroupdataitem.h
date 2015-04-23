#ifndef DISTANCEMEASUREGROUPDATAITEM_H
#define DISTANCEMEASUREGROUPDATAITEM_H

#include "../guicore_global.h"

#include "../datamodel/graphicswindowdataitem.h"

class GUICOREDLL_EXPORT DistanceMeasureGroupDataItem : public GraphicsWindowDataItem
{
	Q_OBJECT

public:
	DistanceMeasureGroupDataItem(GraphicsWindowDataItem* parent);
	void updateZDepthRangeItemCount(){m_zDepthRange.setItemCount(2);}
	void addCustomMenuItems(QMenu* menu);

public slots:
	void addMeasure();

protected:
	void doLoadFromProjectMainFile(const QDomNode& /*node*/);
	void doSaveToProjectMainFile(QXmlStreamWriter& /*writer*/);

	QAction* m_addAction;
};

#endif // DISTANCEMEASUREGROUPDATAITEM_H