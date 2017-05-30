#include "rivexporter.h"
#include "../data/crosssections/crosssections.h"
#include "../data/project/project.h"
#include "../data/crosssection/crosssection.h"
#include "../data/crosssections/crosssections.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QVector2D>

#include <string>

bool RivExporter::exportData(const Project& project, QWidget* w)
{
	QString fname = QFileDialog::getSaveFileName(w, tr("Input File name to export"), "", tr("River Survey Data(*.riv)"));
	if (fname.isNull()) {return false;}

	QFile file(fname);
	if (! file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(w, tr("Error"), tr("%1 could not be opened.").arg(QDir::toNativeSeparators(fname)));
		return false;
	}

	QTextStream ts(&file);
	QPointF offset = project.offset();
	const auto& cs = project.crossSections();

	ts << "#survey" << endl;
	for (CrossSection* s : cs.crossSectionVector()) {
		ts
				<< s->name() << " "
				<< s->point1().x() + offset.x() << " "
				<< s->point1().y() + offset.y() << " "
				<< s->point2().x() + offset.x() << " "
				<< s->point2().y() + offset.y() << endl;
	}
	ts << endl;

	ts << "#x-section" << endl;
	for (CrossSection* s : cs.crossSectionVector()) {
		auto points = s->mappedPoints();
		ts << s->name() << " " << points.size() << endl;

		int index = 0;
		for (auto p : points) {
			ts << " " << p.x() << " " << p.y();
			if (index % 4 == 3) {ts << endl;}
			++ index;
		}

		ts << endl;
	}
	file.close();

	QFileInfo finfo(fname);
	QFile file2(QString("%1/%2_wse.csv").arg(finfo.absolutePath()).arg(finfo.baseName()));
	if (! file2.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(w, tr("Error"), tr("%1 could not be opened.").arg(QDir::toNativeSeparators(file2.fileName())));
		return false;
	}

	QTextStream ts2(&file2);
	ts2 << "CrossSection" << "," << "Elevation" << endl;
	for (CrossSection* s : cs.crossSectionVector()) {
		ts2 << s->name() << "," << s->waterElevation() << endl;
	}
	file2.close();

	return true;
}

RivExporter::RivExporter()
{}
