#include "geometryutil.h"

#include <QPointF>
#include <QVector2D>

bool GeometryUtil::intersectionPoint(const QPointF& p1, const QPointF& p2, const QPointF& q1, const QPointF& q2, QPointF* interSection, double* r, double* s)
{
	double M =
			(p2.x() - p1.x()) * (q2.y() - q1.y()) -
			(p2.y() - p1.y()) * (q2.x() - q1.x());
	if (M == 0) {return false;}

	QPointF p1q1 = q1 - p1;

	*r = ((q2.y() - q1.y()) * p1q1.x() - (q2.x() - q1.x()) * p1q1.y()) / M;
	*s = ((p2.y() - p1.y()) * p1q1.x() - (p2.x() - p1.x()) * p1q1.y()) / M;

	*interSection = p1 + *r * (p2 - p1);

	return true;
}

QPointF GeometryUtil::nearestPoint(const QPointF& a, const QPointF& b, const QPointF& p, int* posType)
{
	QVector2D ab(b.x() - a.x(), b.y() - a.y());
	QVector2D ap(p.x() - a.x(), p.y() - a.y());

	QVector2D ab_unit = ab / ab.length();
	double r = (ap.x() * ab_unit.x() + ap.y() * ab_unit.y()) / ab.length();
	if (posType != nullptr) {*posType = 1;}
	if (r < 0) {return a;}
	if (posType != nullptr) {*posType = 2;}
	if (r > 1) {return b;}

	if (posType != nullptr) {*posType = 3;}
	return QPointF(a.x() + r * ab.x(), a.y() + r * ab.y());
}

QPointF GeometryUtil::mappedPoint(const QPointF& a, const QPointF& b, const QPointF& p)
{
	QVector2D ab(b.x() - a.x(), b.y() - a.y());
	QVector2D ap(p.x() - a.x(), p.y() - a.y());

	QVector2D ab_unit = ab / ab.length();
	double r = (ap.x() * ab_unit.x() + ap.y() * ab_unit.y()) / ab.length();

	return QPointF(a.x() + r * ab.x(), a.y() + r * ab.y());
}
