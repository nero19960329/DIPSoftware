#include "EditImageCommand.h"

EditImageCommand::EditImageCommand(ImgWidget *imgWidget, cv::Mat originMat, cv::Mat newMat, QUndoCommand *parent) : QUndoCommand(parent) {
	this->imgWidget = imgWidget;
	this->originMat = originMat;
	this->newMat = newMat;
}

void EditImageCommand::undo() {
	imgWidget->setImageMat(originMat);
}

void EditImageCommand::redo() {
	imgWidget->setImageMat(newMat);
}