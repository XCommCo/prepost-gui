#ifndef MEASUREDDATA_H
#define MEASUREDDATA_H

#include "projectdataitem.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

/// This class stores the information about the specified workfolder.
class MeasuredData : public ProjectDataItem
{
	Q_OBJECT
public:
	enum ArrowLengthMode {
		almAuto,
		almCustom
	};

	enum ArrowColorMode {
		acmSpecific,
		acmScalar
	};

	/// Constructor
	MeasuredData(ProjectDataItem* parent);
	/// Execute importing.
	void importFromFile(const QString& filename);
	/// Execute exporting.
	void exportToFile(const QString& filename);
	/// Name of this data. default value is the full path name of the imported file.
	const QString& name() const {return m_name;}
	vtkPolyData* pointData(){return m_pointData;}
	vtkPolyData* polyData(){return m_polyData;}
	const QStringList& pointNames() const {return m_pointNames;}
	const QStringList& vectorNames() const {return m_vectorNames;}
	int index(){return m_index;}
	void setIndex(int index){
		m_index = index;
		m_filename = QString("MeasuredData_%1.vtk").arg(m_index);
	}
	void setupPolyData();
	bool noPolyData(){return m_noPolyData;}
	void applyOffset(double x, double y){doApplyOffset(x, y);}
	void doApplyOffset(double x, double y);

protected:
	void doLoadFromProjectMainFile(const QDomNode& node);
	void doSaveToProjectMainFile(QXmlStreamWriter& writer);
	void loadExternalData(const QString& filename);
	void saveExternalData(const QString& filename);

private:
	bool m_noPolyData;
	int m_index;
	vtkSmartPointer<vtkPolyData> m_pointData;
	vtkSmartPointer<vtkPolyData> m_polyData;
	QString m_name;
	QStringList m_pointNames;
	QStringList m_vectorNames;
};

#endif // MEASUREDDATA_H