#ifndef GEODATAPOINTMAPREALIMPORTER_H
#define GEODATAPOINTMAPREALIMPORTER_H

#include <guicore/pre/geodata/geodataimporter.h>
#include <QMap>

class GDALDataset;
class GDALRasterBand;

class GeoDataPointmapRealImporter : public GeoDataImporter
{
	Q_OBJECT

private:
	enum filterString {
		dotTopo,      ///< Topography
		dotDat,       ///< RIC-Nays DEM
		dotAdf,       ///< USGS NED(*.adf)
		dotStl        ///< STL (*.stl)
	};

public:
	/// Constructor
	GeoDataPointmapRealImporter(GeoDataCreator* creator);
	bool importData(GeoData* data, int index, QWidget* w) override;
	const QStringList fileDialogFilters() override;
	const QStringList acceptableExtensions() override;
	GDALDataset* poDataset;
	GDALRasterBand* poBand;

protected:
	bool doInit(const QString& filename, const QString& selectedFilter, int* count, SolverDefinitionGridAttribute* condition, PreProcessorGeoDataGroupDataItemInterface* item, QWidget* w) override;

private:
	int m_filterValue;
};

#endif // GEODATAPOINTMAPREALIMPORTER_H