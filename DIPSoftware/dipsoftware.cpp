#include "EditImageCommand.h"
#include "dipsoftware.h"

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
	histogramWidget = new HistogramWidget(this);
	originMat = nullptr;
	mainLayout = new QHBoxLayout;
	centerWidget = new QWidget(this);
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
	cropAction->setEnabled(false);
	rotate90Action = new QAction(QStringLiteral("&顺时针旋转90度"), this);
	rotate180Action = new QAction(QStringLiteral("&旋转180度"), this);
	rotate270Action = new QAction(QStringLiteral("&逆时针旋转90度"), this);
	rotateAction = new QAction(QStringLiteral("&任意角度..."), this);
	horizontalFlipAction = new QAction(QStringLiteral("&水平翻转"), this);
	verticalFlipAction = new QAction(QStringLiteral("&垂直翻转"), this);
	changeLightnessAction = new QAction(QStringLiteral("&亮度..."), this);
	changeSaturationAction = new QAction(QStringLiteral("&饱和度..."), this);
	changeHueAction = new QAction(QStringLiteral("&色相..."), this);
	changeGammaAction = new QAction(QStringLiteral("&Gamma校正"), this);
	changeLogAction = new QAction(QStringLiteral("&对数变换"), this);
	changePowAction = new QAction(QStringLiteral("&指数变换"), this);
	histEquAction = new QAction(QStringLiteral("&直方图均衡化"), this);

	actionObservers = make_shared<vector<QAction*>>(initializer_list<QAction*>{
		saveFileAction, saveAsFileAction, rotate90Action,
		rotate180Action, rotate270Action, rotateAction,
		horizontalFlipAction, verticalFlipAction, changeLightnessAction,
		changeSaturationAction, changeHueAction, changeGammaAction,
		changeLogAction, changePowAction, histEquAction
	});

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
	imageMenu->addSeparator();
	QMenu *nonLinearMenu = imageMenu->addMenu(QStringLiteral("&非线性变换"));
	nonLinearMenu->addAction(changeGammaAction);
	nonLinearMenu->addAction(changeLogAction);
	nonLinearMenu->addAction(changePowAction);
	imageMenu->addAction(histEquAction);

	connect(imgWidget, &ImgWidget::setCropActionEnabled, this, bind(&QAction::setEnabled, cropAction, placeholders::_1));
	connect(openFileAction, &QAction::triggered, this, &DIPSoftware::openFile);
	connect(saveFileAction, &QAction::triggered, this, &DIPSoftware::saveFile);
	connect(saveAsFileAction, &QAction::triggered, this, &DIPSoftware::saveAsFile);
	connect(cropAction, &QAction::triggered, this, &DIPSoftware::cropImage);
	connect(rotate90Action, &QAction::triggered, this, bind(&DIPSoftware::rotateImage, this, PI / 2));
	connect(rotate180Action, &QAction::triggered, this, bind(&DIPSoftware::rotateImage, this, PI));
	connect(rotate270Action, &QAction::triggered, this, bind(&DIPSoftware::rotateImage, this, PI * 3 / 2));
	connect(rotateAction, &QAction::triggered, this, &DIPSoftware::rotateImageAnyAngle);
	connect(horizontalFlipAction, &QAction::triggered, this, &DIPSoftware::horizontalFlipImage);
	connect(verticalFlipAction, &QAction::triggered, this, &DIPSoftware::verticalFlipImage);
	connect(changeLightnessAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatLightness, QStringLiteral("亮度"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return exp(-d / 100); }, [](float d){ return -100 * log(d); }, QStringLiteral("亮度："), 0, -150, 150)
	}));
	connect(changeSaturationAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatSaturation, QStringLiteral("饱和度"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return 100 * d; }, QStringLiteral("饱和度："), 0, -100, 100)
	}));
	connect(changeHueAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatHue, QStringLiteral("色调"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d; }, [](float d){ return d; }, QStringLiteral("色调："), 0, -180, 180)
	}));
	connect(changeGammaAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatGamma, QStringLiteral("Gamma校正"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return pow(10, d / 1000); }, [](float d){ return 1000 * log10(d); }, QStringLiteral("γ："), 0, -1400, 1400),
		ParameterInfo([](float d){ return pow(10, d / 100); }, [](float d){ return 100 * log10(d); }, QStringLiteral("c："), 0, -100, 100)
	}));
	connect(changeLogAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatLog, QStringLiteral("对数变换"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return d * 100; }, QStringLiteral("a："), 0, -100, 100),
		ParameterInfo([](float d){ return pow(10, d / 100); }, [](float d){ return 100 * log10(d); }, QStringLiteral("b："), 0, -100, 100),
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return 100 * d; }, QStringLiteral("c："), 200, 105, 1000)
	}));
	connect(changePowAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatPow, QStringLiteral("指数变换"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return d * 100; }, QStringLiteral("a："), 0, -100, 100),
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return 100 * d; }, QStringLiteral("b："), 230, 101, 1000),
		ParameterInfo([](float d){ return pow(10, d / 100); }, [](float d){ return 100 * log10(d); }, QStringLiteral("c："), 0, -100, 100)
	}));
	connect(histEquAction, &QAction::triggered, this, &DIPSoftware::histEquImage);

	histogramWidget->setFixedSize(400, 300);
	mainLayout->addWidget(imgWidget);
	mainLayout->addWidget(histogramWidget, 0, Qt::AlignTop);

	setActionsEnabled(false);
	centerWidget->setLayout(mainLayout);
	setCentralWidget(centerWidget);
}

DIPSoftware::~DIPSoftware() {

}

void DIPSoftware::setOriginMat() {
	originMat = imgWidget->imgMat;
}

void DIPSoftware::openFile() {
	QString inputFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开文件"), " ", QStringLiteral("图像文件(*.bmp;*.png;*.jpg;*.jpeg)"));
	if (!inputFileName.size()) {
		return;
	}
	currentFileName = String((const char *) inputFileName.toLocal8Bit());
	Mat imageMat = imread(currentFileName);
	imgWidget->setImageMat(Utils::mat8U2Mat32F(imageMat));
	histogramWidget->setImageMat(Utils::mat8U2Mat32F(imageMat));
	setActionsEnabled(true);
}

void DIPSoftware::saveFile() {
	imwrite(currentFileName, Utils::mat32F2Mat8U(*(imgWidget->imgMat)));
}

void DIPSoftware::saveAsFile() {
	QString inputFileName = QFileDialog::getSaveFileName(this, QStringLiteral("另存为"), "", QStringLiteral("位图文件(*.bmp);;PNG文件(*.png);;JPEG文件(*.jpg;*.jpeg)"));
	currentFileName = String((const char *) inputFileName.toLocal8Bit());
	imwrite(currentFileName, Utils::mat32F2Mat8U(*(imgWidget->imgMat)));
}

void DIPSoftware::cropImage() {
	QRect cropRect = imgWidget->getCropRect();
	cv::Rect cvCropRect(cropRect.topLeft().x(), cropRect.topLeft().y(), cropRect.width(), cropRect.height());
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, (*imgWidget->imgMat)(cvCropRect)));
	imgWidget->removeLastItem();
}

void DIPSoftware::rotateImage(float theta) {
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, Utils::rotateImageMat(*(imgWidget->imgMat), theta)));
}

void DIPSoftware::rotateImageAnyAngle() {
	bool ok;
	float theta = QInputDialog::getDouble(this, QStringLiteral("旋转图像"), QStringLiteral("旋转角度（顺时针）"), 0.0, -180.0, 180.0, 2, &ok);
	if (ok) {
		rotateImage(theta * PI / 180);
	}
}

void DIPSoftware::horizontalFlipImage() {
	Mat image = Mat(imgWidget->imgMat->rows, imgWidget->imgMat->cols, CV_32FC3);
	rep(j, image.cols) {
		imgWidget->imgMat->col(image.cols - j - 1).copyTo(image.col(j));
	}

	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::verticalFlipImage() {
	Mat image = Mat(imgWidget->imgMat->rows, imgWidget->imgMat->cols, CV_32FC3);
	rep(i, image.rows) {
		imgWidget->imgMat->row(image.rows - i - 1).copyTo(image.row(i));
	}

	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::changeImage(function<Mat(vector<float>)> lambdaFunc, function<vector<float>(function<Mat(vector<float>)>, bool&)> changeFunc) {
	setOriginMat();

	bool ok;
	vector<float> deltas = changeFunc(lambdaFunc, ok);
	if (!ok) {
		imgWidget->setImageMat(*originMat);
	} else if (deltas != vector<float>{}) {
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *originMat, lambdaFunc(deltas)));
	} else {
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *originMat, *imgWidget->imgMat));
	}
}

void DIPSoftware::uiChangeImage(Utils::changeFuncType changeFunc, const QString &title, const vector<ParameterInfo>& infos) {
	auto lambdaFunc = [=](vector<float> d){ return Utils::changeImageMat(*originMat, d, changeFunc); };
	changeImage(lambdaFunc, [=](function<Mat(vector<float>)> lambdaFunc, bool& ok){ return InputPreviewDialog::changeFloat(this, imgWidget, lambdaFunc, title, infos, &ok); });
}

void DIPSoftware::histEquImage() {
	Mat image = Utils::histogramEqualization(*imgWidget->imgMat);
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::setActionsEnabled(bool enabled) {
	for (const auto& action : *actionObservers) {
		action->setEnabled(enabled);
	}
}