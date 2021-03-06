#ifndef RIGHTBANKHWMPREPROCESSORVIEW_H
#define RIGHTBANKHWMPREPROCESSORVIEW_H

#include "../points/pointspreprocessorview.h"

class RightBankHWM;

class RightBankHWMPreProcessorView : public PointsPreProcessorView
{
public:
	RightBankHWMPreProcessorView(Model* model, RightBankHWM* item);
	~RightBankHWMPreProcessorView();

private:
	void drawMarker(const QPointF& position, QPainter* painter) const override;
	void doDraw(QPainter *painter) const override;
};

#endif // RIGHTBANKHWMPREPROCESSORVIEW_H
