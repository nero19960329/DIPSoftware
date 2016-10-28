#pragma once

#include <QPaintEvent>
#include <QWidget>

#include <opencv2/opencv.hpp>

#include <array>

class HistogramWidget : public QWidget {
	Q_OBJECT

public:
	HistogramWidget(QWidget *parent = 0) : QWidget(parent) {}
	virtual ~HistogramWidget() {}

	void setImageMat(const cv::Mat& mat);

	void paintEvent(QPaintEvent *);

private:
	std::array<int, 256> histogramArr = {};
};