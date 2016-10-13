#include "EditImageCommand.h"
#include "dipsoftware.h"
#include "InputPreviewDialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QDebug>

#include <cmath>

#define PI 3.1415926

using namespace cv;
using namespace std;

DIPSoftware::DIPSoftware(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	showMaximized();

	imgWidget = new ImgWidget(this);
	originMat = NULL;
	mainLayout = new QHBoxLayout;
	centerWidget = new QWidget(this);
	actionObservers = new list<QAction*>();
	undoStack = new QUndoStack(this);

	openFileAction = new QAction(QStringLiteral("&打开..."), this);
	openFileAction->setShortcut(QKeySequence::Open);
	saveFileAction = new QAction(QStringLiteral("&保存..."), this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveAsFileAction = new QAction(QStringLiteral("&另存为..."), this);
	saveAsFileAction->setShortcut(QKeySequence::SaveAs);

	undoAction = undoStack->createUndoAction(this, QStringLiteral("&撤销"));
	undoAction->setShortcut(QKeySequence::Undo);
	redoAction = undoStack->createRedoAction(this, QStringLiteral("&重做"));
	redoAction->setShortcut(QKeySequence::Redo);

	cropAction = new QAction(QStringLiteral("&裁剪..."), this);
	rotate90Action = new QAction(QStringLiteral("&顺时针旋转90度"), this);
	rotate180Action = new QAction(QStringLiteral("&旋转180度"), this);
	rotate270Action = new QAction(QStringLiteral("&逆时针旋转90度"), this);
	rotateAction = new QAction(QStringLiteral("&任意角度..."), this);
	horizontalFlipAction = new QAction(QStringLiteral("&水平翻转"), this);
	verticalFlipAction = new QAction(QStringLiteral("&垂直翻转"), this);
	changeLightnessAction = new QAction(QStringLiteral("&亮度..."), this);
	changeSaturationAction = new QAction(QStringLiteral("&饱和度..."), this);
	changeHueAction = new QAction(QStringLiteral("&色相..."), this);

	actionObservers->push_back(saveFileAction);
	actionObservers->push_back(saveAsFileAction);
	actionObservers->push_back(cropAction);
	actionObservers->push_back(rotate90Action);
	actionObservers->push_back(rotate180Action);
	actionObservers->push_back(rotate270Action);
	actionObservers->push_back(rotateAction);
	actionObservers->push_back(horizontalFlipAction);
	actionObservers->push_back(verticalFlipAction);
	actionObservers->push_back(changeLightnessAction);
	actionObservers->push_back(changeSaturationAction);
	actionObservers->push_back(changeHueAction);

	QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("&文件"));
	fileMenu->addAction(openFileAction);
	fileMenu->addAction(saveFileAction);
	fileMenu->addAction(saveAsFileAction);
	QMenu *editMenu = menuBar()->addMenu(QStringLiteral("&操作"));
	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);
	QMenu *imageMenu = menuBar()->addMenu(QStringLiteral("&图像"));
	imageMenu->addAction(cropAction);
	imageMenu->addSeparator();
	QMenu *rotateMenu = imageMenu->addMenu(QStringLiteral("&图像旋转"));
	rotateMenu->addAction(rotate90Action);
	rotateMenu->addAction(rotate180Action);
	rotateMenu->addAction(rotate270Action);
	rotateMenu->addAction(rotateAction);
	rotateMenu->addSeparator();
	rotateMenu->addAction(horizontalFlipAction);
	rotateMenu->addAction(verticalFlipAction);
	QMenu *changeMenu = imageMenu->addMenu(QStringLiteral("&调整..."));
	changeMenu->addAction(changeLightnessAction);
	changeMenu->addAction(changeSaturationAction);
	changeMenu->addAction(changeHueAction);

	connect(openFileAction, &QAction::triggered, this, &DIPSoftware::openFile);
	connect(saveFileAction, &QAction::triggered, this, &DIPSoftware::saveFile);
	connect(saveAsFileAction, &QAction::triggered, this, &DIPSoftware::saveAsFile);
	connect(cropAction, &QAction::triggered, this, &DIPSoftware::cropImage);
	connect(rotate90Action, &QAction::triggered, this, [this]{ undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, Utils::rotateImageMat(*(imgWidget->imgMat), PI / 2))); });
	connect(rotate180Action, &QAction::triggered, this, [this]{ undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, Utils::rotateImageMat(*(imgWidget->imgMat), PI))); });
	connect(rotate270Action, &QAction::triggered, this, [this]{ undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, Utils::rotateImageMat(*(imgWidget->imgMat), PI * 3 / 2))); });
	connect(rotateAction, &QAction::triggered, this, &DIPSoftware::rotateImage);
	connect(horizontalFlipAction, &QAction::triggered, this, &DIPSoftware::horizontalFlipImage);
	connect(verticalFlipAction, &QAction::triggered, this, &DIPSoftware::verticalFlipImage);
	connect(changeLightnessAction, &QAction::triggered, this, [this](){
		function<Mat(int)> lambdaFunc = [this](int d){ return Utils::changeImageMatLightness(*originMat, d * 1.0f / 100); };
		changeImage(lambdaFunc, [this](function<Mat(int)> lambdaFunc, bool& ok){ return InputPreviewDialog::changeInt(this, imgWidget, lambdaFunc, QStringLiteral("亮度"), QStringLiteral("亮度："), 0, -150, 150, &ok); });
	});
	connect(changeSaturationAction, &QAction::triggered, this, [this](){
		function<Mat(int)> lambdaFunc = [this](int d){ return Utils::changeImageMatSaturation(*originMat, d * 1.0f / 100); };
		changeImage(lambdaFunc, [this](function<Mat(int)> lambdaFunc, bool& ok){ return InputPreviewDialog::changeInt(this, imgWidget, lambdaFunc, QStringLiteral("饱和度"), QStringLiteral("饱和度："), 0, -150, 150, &ok); });
	});
	connect(changeHueAction, &QAction::triggered, this, [this](){
		function<Mat(int)> lambdaFunc = [this](int d){ return Utils::changeImageMatHue(*originMat, d); };
		changeImage(lambdaFunc, [this](function<Mat(int)> lambdaFunc, bool& ok){ return InputPreviewDialog::changeInt(this, imgWidget, lambdaFunc, QStringLiteral("色相"), QStringLiteral("色相："), 0, -180, 180, &ok); });
	});

	setActionsEnabled(false);
	mainLayout->addWidget(imgWidget);
	centerWidget->setLayout(mainLayout);
	setCentralWidget(centerWidget);
}

DIPSoftware::~DIPSoftware() {

}

void DIPSoftware::setOriginMat() {
	if (originMat) {
		delete originMat;
	}
	originMat = new Mat(*imgWidget->imgMat);
}

void DIPSoftware::openFile() {
	QString inputFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开文件"), " ", QStringLiteral("图像文件(*.bmp;*.png;*.jpg;*.jpeg)"));
	if (!inputFileName.size()) {
		return;
	}
	currentFileName = String((const char *) inputFileName.toLocal8Bit());
	Mat imageMat = imread(currentFileName);
	imgWidget->setImageMat(imageMat);
	setActionsEnabled(true);
}

void DIPSoftware::saveFile() {
	imwrite(currentFileName, *(imgWidget->imgMat));
}

void DIPSoftware::saveAsFile() {
	QString inputFileName = QFileDialog::getSaveFileName(this, QStringLiteral("另存为"), "", QStringLiteral("位图文件(*.bmp);;PNG文件(*.png);;JPEG文件(*.jpg;*.jpeg)"));
	currentFileName = String((const char *) inputFileName.toLocal8Bit());
	imwrite(currentFileName, *(imgWidget->imgMat));
}

void DIPSoftware::cropImage() {
	QRect cropRect = imgWidget->getCropRect();
	cv::Rect cvCropRect(cropRect.topLeft().x(), cropRect.topLeft().y(), cropRect.width(), cropRect.height());
	undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, (*imgWidget->imgMat)(cvCropRect)));
	imgWidget->removeLastItem();
}

void DIPSoftware::rotateImage() {
	bool ok;
	float theta = QInputDialog::getDouble(this, QStringLiteral("旋转图像"), QStringLiteral("旋转角度（顺时针）"), 0.0, -180.0, 180.0, 2, &ok);
	if (ok) {
		Mat rotatedImg = Utils::rotateImageMat(*(imgWidget->imgMat), theta * PI / 180);
		undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, rotatedImg));
	}
}

void DIPSoftware::horizontalFlipImage() {
	Mat image = Mat(imgWidget->imgMat->rows, imgWidget->imgMat->cols, CV_8UC3);
	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			image.at<Vec3b>(i, j) = imgWidget->imgMat->at<Vec3b>(i, image.cols - j - 1);
		}
	}

	undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::verticalFlipImage() {
	Mat image = Mat(imgWidget->imgMat->rows, imgWidget->imgMat->cols, CV_8UC3);
	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			image.at<Vec3b>(i, j) = imgWidget->imgMat->at<Vec3b>(image.rows - i - 1, j);
		}
	}

	undoStack->push(new EditImageCommand(imgWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::changeImage(function<Mat(int)> lambdaFunc, function<int(function<Mat(int)>, bool&)> changeFunc) {
	setOriginMat();

	bool ok;
	int delta = changeFunc(lambdaFunc, ok);
	if (!ok) {
		imgWidget->setImageMat(*originMat);
	} else if (delta != -2147483647) {
		undoStack->push(new EditImageCommand(imgWidget, *originMat, lambdaFunc(delta)));
	} else {
		undoStack->push(new EditImageCommand(imgWidget, *originMat, *imgWidget->imgMat));
	}
}

void DIPSoftware::setActionsEnabled(bool enabled) {
	for (auto action : *actionObservers) {
		action->setEnabled(enabled);
	}
}