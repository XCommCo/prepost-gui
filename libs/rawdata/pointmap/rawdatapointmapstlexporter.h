#ifndef RAWDATAPOINTMAPSTLEXPORTER_H
#define RAWDATAPOINTMAPSTLEXPORTER_H

#include <guicore/pre/rawdata/rawdataexporter.h>

class RawDataPointmapSTLExporter : public RawDataExporter
{
	Q_OBJECT
public:
	RawDataPointmapSTLExporter(RawDataCreator* creator);
	bool doEmport(RawData* data, const QString& filename, const QString& selectedFilter, QWidget* w, ProjectData* pd);
	const QStringList fileDialogFilters();
};

#endif // RAWDATAPOINTMAPSTLEXPORTER_H