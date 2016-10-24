#ifndef DIPSOFTWARE_H
#define DIPSOFTWARE_H

#include <QtWidgets/QMainWindow>
#include "ui_dipsoftware.h"

#include "ImgWidget.h"
#include "Utils.h"

#include <QAction>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QUndoStack>

#include <functional>
#include <memory>
#include <vector>

class DIPSoftware : public QMainWindow {
	Q_OBJECT

public:
	DIPSoftware(QWidget *parent = 0);
	~DIPSoftware();

private:
	void setOriginMat();

	void openFile();
	void saveFile();
	void saveAsFile();
	void cropImage();
	void rotateImage(float theta);
	void rotateImageAnyAngle();
	void horizontalFlipImage();
	void verticalFlipImage();
	void changeImage(std::function<cv::Mat(int)> lambdaFunc, std::function<int(std::function<cv::Mat(int)>, bool&)> changeFunc);

	void setActionsEnabled(bool enabled);

private:
	Ui::DIPSoftwareClass ui;

	cv::String currentFileName;

	QAction *openFileAction;
	QAction *saveFileAction;
	QAction *saveAsFileAction;

	QAction *undoAction;
	QAction *redoAction;

	QAction *cropAction;
	QAction *rotate90Action;
	QAction *rotate180Action;
	QAction *rotate270Action;
	QAction *rotateAction;
	QAction *horizontalFlipAction;
	QAction *verticalFlipAction;

	QAction *changeLightnessAction;
	QAction *changeSaturationAction;
	QAction *changeHueAction;

	std::shared_ptr<std::vector<QAction*>> actionObservers;

	QUndoStack *undoStack;

	QHBoxLayout *mainLayout;
	QWidget *centerWidget;

	ImgWidget *imgWidget;
	std::shared_ptr<cv::Mat> originMat;
};

#endif // DIPSOFTWARE_H
