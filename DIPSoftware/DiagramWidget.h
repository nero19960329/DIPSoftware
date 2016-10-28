#pragma once

#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

#include <list>

class DiagramWidget : public QWidget {
	Q_OBJECT

public:
	std::list<std::pair<float, float>> vertices;

	DiagramWidget(QWidget *parent = 0) : QWidget(parent), vertices(std::list<std::pair<float, float>>{ std::make_pair(0, 0), std::make_pair(1, 1) }), isPressed(false) {}
	~DiagramWidget() {}

	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

signals:
	void valueChanged(const std::list<std::pair<float, float>>& vertices);

private:
	bool isPressed;
	std::list<std::pair<float, float>>::iterator vertexIt;
	
	std::pair<float, float> point2Pair(const QPoint &pos);
};