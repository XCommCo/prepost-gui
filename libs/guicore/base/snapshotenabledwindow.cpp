#include "snapshotenabledwindow.h"
#include "../datamodel/vtkgraphicsview.h"

#include <vtkFloatArray.h>
#include <vtkRenderWindow.h>

#include <QBitmap>

SnapshotEnabledWindow::SnapshotEnabledWindow()
{
	m_isTransparent = false;
}

void SnapshotEnabledWindow::makeBackgroundTransparent(VTKGraphicsView* view, QPixmap& pixmap)
{
	int w = pixmap.size().width();
	int h = pixmap.size().height();
	QImage ret = QImage(w, h, QImage::Format_ARGB32);
	QImage tmp = pixmap.toImage();
	tmp.convertToFormat(QImage::Format_ARGB32);
	vtkRenderWindow* renWin = view->GetRenderWindow();
	vtkFloatArray* zbuf = vtkFloatArray::New();
	renWin->GetZbufferData(0, 0, w - 1, h - 1, zbuf);
	for (vtkIdType id = 0; id < zbuf->GetNumberOfTuples(); id++) {
		int x = id % w;
		int y = h - 1 - id / w;
		QRgb pixel = tmp.pixel(x, y);
		if (zbuf->GetValue(id) >= 1) {
			QColor pixCol = QColor::fromRgba(pixel);
			pixel = qRgba(pixCol.red(), pixCol.green(), pixCol.blue(), 0);
		}
		ret.setPixel(x, y, pixel);
	}
	pixmap = QPixmap::fromImage(ret);
	zbuf->Delete();
}