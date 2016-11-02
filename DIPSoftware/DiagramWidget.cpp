#include "DiagramWidget.h"
#include "Utils.h"

#include <QPainter>

using namespace std;

pair<float, float> DiagramWidget::point2Pair(const QPoint &point) {
	return make_pair(point.x() * 1.0 / width(), (height() - point.y()) * 1.0 / height());
}

void DiagramWidget::paintEvent(QPaintEvent *event) {
	int w, h;
	w = width();
	h = height();

	QPainter p(this);
	p.setBrush(QBrush(QColor(255, 255, 255)));
	p.drawRect(0, 0, w - 1, h - 1);

	p.setPen(QPen(QColor(0, 0, 0)));
	auto it = vertices.begin();
	auto nextIt = it;
	++nextIt;
	for (; nextIt != vertices.end(); ++it, ++nextIt) {
		p.drawLine(w * it->first, h * (1.0f - it->second), w * nextIt->first, h * (1.0f - nextIt->second));
	}
}

void DiagramWidget::mousePressEvent(QMouseEvent *event) {
	isPressed = true;
	pair<float, float> pos = point2Pair(event->pos());

	for (const auto &vertex : vertices) {
		if (fabs(pos.first - vertex.first) < 5e-2) {
			pos.first = vertex.first;
			break;
		}
	}

	auto it = vertices.begin();
	auto nextIt = vertices.begin();
	++nextIt;
	for (; nextIt != vertices.end(); ++it, ++nextIt) {
		if (pos.first == it->first) {
			it->second = pos.second;
			emit(valueChanged(vertices));
			vertexIt = it;
			break;
		} else if (pos.first > it->first && pos.first < nextIt->first) {
			vertices.insert(++it, pos);
			emit(valueChanged(vertices));
			vertexIt = --it;
			break;
		} else if (pos.first == nextIt->first) {
			nextIt->second = pos.second;
			emit(valueChanged(vertices));
			vertexIt = nextIt;
			break;
		}
	}
	repaint();
}

void DiagramWidget::mouseMoveEvent(QMouseEvent *event) {
	if (isPressed) {
		pair<float, float> pos = point2Pair(event->pos());

		if (vertexIt->first >= 1.0f - FLT_EPSILON || vertexIt->first <= FLT_EPSILON) {
			vertexIt->second = pos.second;
		} else {
			*vertexIt = pos;
		}

		emit(valueChanged(vertices));
	}
	repaint();
}

void DiagramWidget::mouseReleaseEvent(QMouseEvent *event) {
	isPressed = false;
}