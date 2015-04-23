#ifndef CGNSFILEINPUTCONDITIONPAGELIST_H
#define CGNSFILEINPUTCONDITIONPAGELIST_H

#include "../../guicore_global.h"

#include <QString>
#include <QTreeWidget>
#include <QMap>

class QDomElement;
class SolverDefinitionTranslator;
class QTreeWidgetItem;

/// This class handles the list of pages displayed in the left side of
/// CgnsFileInputConditionDialog.
class GUICOREDLL_EXPORT CgnsFileInputConditionPageList : public QTreeWidget
{
	Q_OBJECT
private:
	const static int WIDTH_MIN = 150;
	const static int WIDTH_MAX = 300;
public:
	/// Constructor
	CgnsFileInputConditionPageList(QWidget* w = 0);
	void setup(const QDomElement& elem, const SolverDefinitionTranslator& translator);
	void selectFirstItem();
signals:
	void selectChanged(const QString& pagename);
private slots:
	void handleSelectionChange();
private:
	QMap<QTreeWidgetItem*, QString> m_nameMap;
};

#endif // CGNSFILEINPUTCONDITIONPAGELIST_H