#include "HistogramWidget.h"
#include "Utils.h"
#include "DebugUtils.h"

#include <QPainter>

using namespace cv;
using namespace std;

void HistogramWidget::setImageMat(const Mat& mat) {
	histogramArr = Utils::getHistogram(mat);
	repaint();
}

void HistogramWidget::paintEvent(QPaintEvent *event) {
	int w, h;
	w = width();
	h = height();

	QPainter p(this);
	p.setBrush(QBrush(QColor(255, 255, 255)));
	p.drawRect(0, 0, w - 1, h - 1);

	if (histogramArr == array<int, 256>{}) {
		return;
	}

	int maxCount = 0;
	for (const auto& elem : histogramArr) {
		updateMax(maxCount, elem);
	}

	float xStep, yStep;
	xStep = (w - 2) * 1.0f / 256;
	yStep = (h - 2) * 1.0f / maxCount;

	rep(i, 256) {
		p.fillRect(QRectF(1 + i * xStep, 1 + yStep * (maxCount - histogramArr[i]), xStep, yStep * histogramArr[i]), QBrush(QColor(150, 150, 150)));
	}
}