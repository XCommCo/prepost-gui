#ifndef PROPERTYBROWSERVIEW_H
#define PROPERTYBROWSERVIEW_H

#include "../guicore_global.h"
#include "propertybrowserattribute.h"

#include <QWidget>
#include <QList>

#include <vtkIdList.h>

namespace Ui {
class PropertyBrowserView;
}

class AttributeBrowserTargetDataItem;

class GUICOREDLL_EXPORT PropertyBrowserView : public QWidget
{
	Q_OBJECT

public:
	static const int ROWHEIGHT = 24;

	explicit PropertyBrowserView(QWidget *parent = 0);
	~PropertyBrowserView();
	void setTargetDataItem(AttributeBrowserTargetDataItem* item);
	void resetForVertex(bool structured);
	void resetForCell(bool structured);
	void hideAll();

	void resetAttributes(bool internal = false);
	void setVertexAttributes(vtkIdType index, double x, double y, const QList<PropertyBrowserAttribute>& attr);
	void setVertexAttributes(unsigned int i, unsigned int j, double x, double y, const QList<PropertyBrowserAttribute>& attr);
	void setCellAttributes(vtkIdType index, const QPolygonF& polygon, const QList<PropertyBrowserAttribute>& attr);
	void setCellAttributes(unsigned int i, unsigned int j, const QPolygonF& polygon, const QList<PropertyBrowserAttribute>& attr);

private:
	void resetBase(bool structured);
	void updateIndex(vtkIdType index);
	void updateIJ(unsigned int i, unsigned int j);
	void updateCoords(double x, double y);
	void updateAttributes(const QList<PropertyBrowserAttribute>& attr);
	AttributeBrowserTargetDataItem* m_targetDataItem;
	Ui::PropertyBrowserView *ui;
};

#endif // PROPERTYBROWSERVIEW_H