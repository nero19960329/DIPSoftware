#include "EditImageCommand.h"

#include <iostream>
using namespace std;

EditImageCommand::EditImageCommand(ImgWidget *_imgWidget, HistogramWidget *_histogramWidget, cv::Mat _originMat, cv::Mat _newMat, QUndoCommand *parent) : QUndoCommand(parent), imgWidget(_imgWidget), histogramWidget(_histogramWidget), originMat(_originMat), newMat(_newMat) {}

void EditImageCommand::undo() {
	imgWidget->setImageMat(originMat);
	histogramWidget->setImageMat(originMat);
}

void EditImageCommand::redo() {
	imgWidget->setImageMat(newMat);
	histogramWidget->setImageMat(newMat);
}