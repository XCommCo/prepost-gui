#ifndef SOLVERCONSOLEWINDOWPROJECTDATAITEM_H
#define SOLVERCONSOLEWINDOWPROJECTDATAITEM_H

#include "solverconsole_global.h"
#include <guicore/project/projectdataitem.h>
#include <misc/windowgeometrycontainer.h>
#include <QFile>
#include <QStringList>

class ProjectData;
class SolverConsoleWindow;

/// This class load or save the status of SolverConsoleWindow into ProjectMainFile .
class SOLVERCONSOLEDLL_EXPORT SolverConsoleWindowProjectDataItem : public ProjectDataItem
{
	Q_OBJECT

public:
	const static int MAXLINES = 200;
	SolverConsoleWindowProjectDataItem(SolverConsoleWindow* w, ProjectDataItem* parent);
	~SolverConsoleWindowProjectDataItem();

	void initForSolverDefinition();
	void loadExternalData(const QString& filename) override;

	void open();
	void close();
	void clear();
	void append(const QString& line);

public slots:
	void exportConsoleLog();

private:
	void appendToLines(const QString& line);

	void doLoadFromProjectMainFile(const QDomNode& element) override;
	void doSaveToProjectMainFile(QXmlStreamWriter& writer) override;

	SolverConsoleWindow* m_solverConsoleWindow;
	WindowGeometryContainer m_geometry;
	QStringList m_lines;
	QFile m_file;
};

#endif // SOLVERCONSOLEWINDOWPROJECTDATAITEM_H
