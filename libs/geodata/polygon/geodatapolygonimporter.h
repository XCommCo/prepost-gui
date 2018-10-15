#ifndef GEODATAPOLYGONIMPORTER_H
#define GEODATAPOLYGONIMPORTER_H

#include "geodatapolygonimportersettingdialog.h"

#include <guicore/pre/geodata/geodataimporter.h>

#include <QVariant>

#include <vector>

class GeoDataPolygonImporter : public GeoDataImporter
{
	Q_OBJECT
private:
	struct PolygonShapeInfo {
		int item;
		int region;
		std::vector<int> holes;
	};

public:
	GeoDataPolygonImporter(GeoDataCreator* creator);
	virtual ~GeoDataPolygonImporter();

	const QStringList fileDialogFilters() override;
	const QStringList acceptableExtensions() override;
	bool importData(GeoData* data, int index, QWidget* w) override;

protected:
	bool doInit(const QString& filename, const QString& /*selectedFilter*/, int* count, SolverDefinitionGridAttribute* condition, PreProcessorGeoDataGroupDataItemInterface* item, QWidget* w) override;

private:
	static std::vector<PolygonShapeInfo> buildPolygonShapeInfos(const std::string& shpFileName);

	GeoDataPolygonImporterSettingDialog::NameSetting m_nameSetting;
	int m_nameAttribute;
	GeoDataPolygonImporterSettingDialog::ValueSetting m_valueSetting;
	std::vector<PolygonShapeInfo> m_shapeInfos;
	int m_valueAttribute;
	QVariant m_specifiedValue;
};

#endif // GEODATAPOLYGONIMPORTER_H
