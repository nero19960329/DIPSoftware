#pragma once

#include <QUndoCommand>

#include <opencv2/opencv.hpp>

#include "HistogramWidget.h"
#include "ImgWidget.h"

class EditImageCommand : public QUndoCommand {
public:
	explicit EditImageCommand(ImgWidget *_imgWidget = 0, HistogramWidget *_histogramWidget = 0, cv::Mat _originMat = {}, cv::Mat _newMat = {}, QUndoCommand *parent = 0);
	void undo();
	void redo();

signals:
	void modifyWidgetStates(const cv::Mat& mat);

private:
	cv::Mat originMat, newMat;
	ImgWidget *imgWidget;
	HistogramWidget *histogramWidget;
};