#ifndef EDITIMAGECOMMAND_H
#define EDITIMAGECOMMAND_H

#include <QUndoCommand>

#include <opencv2/opencv.hpp>

#include "ImgWidget.h"

class EditImageCommand : public QUndoCommand {
public:
	explicit EditImageCommand(ImgWidget *imgWidget = 0, cv::Mat originMat = {}, cv::Mat newMat = {}, QUndoCommand *parent = 0);
	void undo();
	void redo();

private:
	cv::Mat originMat, newMat;
	ImgWidget *imgWidget;
};

#endif