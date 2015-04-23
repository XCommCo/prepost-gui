#ifndef STRUCTURED15DGRIDWITHCROSSSECTIONCROSSSECTIONWINDOWGRAPHICSVIEW_H
#define STRUCTURED15DGRIDWITHCROSSSECTIONCROSSSECTIONWINDOWGRAPHICSVIEW_H

#include <QAbstractItemView>

class Structured15DGridWithCrossSectionCrossSection;
class Structured15DGridWithCrossSectionCrossSectionWindow;

class Structured15DGridWithCrossSectionCrossSectionWindowGraphicsView : public QAbstractItemView
{
	Q_OBJECT
public:
	enum MouseEventMode {
		meNormal,
		meSelecting,
		meZooming,
		meTranslating
	};
	Structured15DGridWithCrossSectionCrossSectionWindowGraphicsView(QWidget* w);
	void setParentWindow(Structured15DGridWithCrossSectionCrossSectionWindow* w){
		m_parentWindow = w;
	}
	void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
	void paintEvent(QPaintEvent* event);
	QRect visualRect(const QModelIndex&) const {return QRect();}
	void scrollTo(const QModelIndex & /*index*/, ScrollHint /*hint*/){}
	QModelIndex indexAt(const QPoint&) const {
		viewport()->update();
		return QModelIndex();
	}
	QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers){
		viewport()->update();
		return QModelIndex();
	}
	int horizontalOffset() const {return 0;}
	int verticalOffset() const {return 0;}
	bool isIndexHidden(const QModelIndex&) const {return false;}
	void setSelection(const QRect & /*rect*/, QItemSelectionModel::SelectionFlags /*command*/){
		viewport()->update();
	}
	void selectionChanged ( const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ ){
		updateActionStatus();
		viewport()->update();
	}
	QRegion visualRegionForSelection(const QItemSelection & /*selection*/) const {return QRegion();}
	void cameraFit();
	void cameraMoveLeft();
	void cameraMoveRight();
	void cameraMoveUp();
	void cameraMoveDown();
	void cameraZoomIn();
	void cameraZoomOut();
	void cameraZoomInX();
	void cameraZoomOutX();
	void cameraZoomInY();
	void cameraZoomOutY();
private slots:
	void moveSelectedRows();
protected:
	int moveWidth();
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
private:
	void setupActions();
	void setupMenu();
	void updateActionStatus();
	void drawCircle(QPainter& painter);
	void drawSelectionCircle(QPainter& painter);
	void drawLine(Structured15DGridWithCrossSectionCrossSection* section, QPainter& painter);
	void zoom(double scaleX, double scaleY);
	void selectPoints(const QPoint& from, const QPoint& to);
	void translate(int x, int y);
	void updateMouseCursor();

	const static int iLeftMargin = 20;
	const static int iRightMargin = 20;
	const static int iTopMargin = 20;
	const static int iBottomMargin = 20;

	const static int ellipseR = 2;
	const static int selectedEllipseR = 4;
	const static int fontRectWidth = 200;
	const static int fontRectHeight = 10;
	const static int bankHOffset = 10;
	const static int bankVOffset = 30;

	double fLeftMargin;
	double fRightMargin;
	double fTopMargin;
	double fBottomMargin;

	QPixmap m_zoomPixmap;
	QPixmap m_movePixmap;

	QCursor m_zoomCursor;
	QCursor m_moveCursor;

	QPointF getPointF(double x, double y, const QRect& viewport);
	QRectF getRegion();
	QMatrix getMatrix(QRect& viewport);
	void drawScales(QPainter& painter, const QMatrix& matrix);

	QMenu* m_rightClickingMenu;
	QAction* m_moveAction;

	Structured15DGridWithCrossSectionCrossSectionWindow* m_parentWindow;
	QRubberBand* m_rubberBand;
	QRectF m_drawnRegion;
	MouseEventMode m_mouseEventMode;
	QRectF m_shownRegion;
	QPoint m_oldPosition;
	QPoint m_rubberOrigin;
	QPoint m_dragStartPoint;
	QMatrix m_matrix;
};

#endif // STRUCTURED15DGRIDWITHCROSSSECTIONCROSSSECTIONWINDOWGRAPHICSVIEW_H