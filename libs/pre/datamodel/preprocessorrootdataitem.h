#ifndef PREPROCESSORROOTDATAITEM_H
#define PREPROCESSORROOTDATAITEM_H

#include <guicore/datamodel/graphicswindowrootdataitem.h>
#include "../preprocessordatamodel.h"
#include <QList>
#include <QMap>

class QAction;
class RawData;

class PreProcessorBackgroundImagesDataItem;
class PreProcessorGridTypeDataItem;
class PreProcessorGridDataItem;
class PreProcessorInputConditionDataItem;
class Axis2dDataItem;
class QStandardItemModel;
class PreProcessorWindow;
class PreProcessorRootDataItemSetMappingSettingCommand;
class PreProcessorMeasuredDataTopDataItem;
class DistanceMeasureGroupDataItem;
class AttributeBrowserTargetDataItem;

class PreProcessorRootDataItem : public GraphicsWindowRootDataItem
{
	Q_OBJECT
public:
	/// Constructor
	PreProcessorRootDataItem(PreProcessorWindow* window, ProjectDataItem* parent);
	~PreProcessorRootDataItem();

	/// Background image data item
	PreProcessorBackgroundImagesDataItem* backgroundImagesDataItem(){
		return m_backgroundImagesDataItem;
	}
	/// Measured values data item;
	PreProcessorMeasuredDataTopDataItem* measuredDataTopDataItem(){
		return m_measuredDataTopDataItem;
	}
	void setupStandardModel(QStandardItemModel* model);
	PreProcessorGridTypeDataItem* gridTypeDataItem(const QString& name);
	const QList<PreProcessorGridTypeDataItem*>& gridTypeDataItems() const {return m_gridTypeDataItems;}
	PreProcessorInputConditionDataItem* inputConditionDataItem(){return m_inputConditionDataItem;}
	AttributeBrowserTargetDataItem* attributeBrowserTargetDataItem() const {return m_attributeBrowserTargetDataItem;}

//	GridAttributeMappingMode gridAttributeMappingMode(){return m_mappingMode;}
//	GeoGraphicDataMappingMode geodataMappingMode(){return m_geodataMappingMode;}
	QAction* editGridAttributeMappingSettingAction(){return m_editGridAttributeMappingSettingAction;}
	bool gridEdited();
	void toggleGridEditFlag();
	void deleteItem(QStandardItem* item);
	void saveToCgnsFile(const int fn);

//	bool tempAutoMode;
//	double tempStreamWiseLength;
//	double tempCrossStreamLength;
//	int tempNumExpansion;
//	double tempWeightExponent;

private slots:
	void editGridAttributeMappingSetting();
protected:
	void doLoadFromProjectMainFile(const QDomNode& node);
	void doSaveToProjectMainFile(QXmlStreamWriter& writer);
	PreProcessorDataModel* dataModel(){return dynamic_cast<PreProcessorDataModel*>(m_dataModel);}
private:
	PreProcessorBackgroundImagesDataItem* m_backgroundImagesDataItem;
	PreProcessorMeasuredDataTopDataItem* m_measuredDataTopDataItem;
	PreProcessorInputConditionDataItem* m_inputConditionDataItem;
	Axis2dDataItem* m_axesDataItem;
	DistanceMeasureGroupDataItem* m_distanceMeasureGroupDataItem;
	AttributeBrowserTargetDataItem* m_attributeBrowserTargetDataItem;
	QList<PreProcessorGridTypeDataItem*> m_gridTypeDataItems;
//	GridAttributeMappingMode m_mappingMode;
//	GeoGraphicDataMappingMode m_geodataMappingMode;

	QAction* m_editGridAttributeMappingSettingAction;
public:
	friend class PreProcessorWindowProjectDataItem;
	friend class PreProcessorDataModel;
	friend class PreProcessorRootDataItemSetMappingSettingCommand;
	friend class RawData;
};

#endif // PREPROCESSORROOTDATAITEM_H