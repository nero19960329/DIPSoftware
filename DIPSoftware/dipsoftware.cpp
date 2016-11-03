#include "EditImageCommand.h"
#include "DiagramPreviewDialog.h"
#include "dipsoftware.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QDebug>

#include <cmath>

#include <list>

#define PI 3.1415926

using namespace cv;
using namespace std;

DIPSoftware::DIPSoftware(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	showMaximized();

	imgWidget = new ImgWidget(this);
	histogramWidget = new HistogramWidget(this);
	diagramWidget = new DiagramWidget(this);
	originMat = nullptr;
	mainLayout = new QHBoxLayout;
	centerWidget = new QWidget(this);
	undoStack = new QUndoStack(this);

	openFileAction = new QAction(QSL("&��..."), this);
	openFileAction->setShortcut(QKeySequence::Open);
	saveFileAction = new QAction(QSL("&����..."), this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveAsFileAction = new QAction(QSL("&���Ϊ..."), this);
	saveAsFileAction->setShortcut(QKeySequence::SaveAs);

	undoAction = undoStack->createUndoAction(this, QSL("&����"));
	undoAction->setShortcut(QKeySequence::Undo);
	redoAction = undoStack->createRedoAction(this, QSL("&����"));
	redoAction->setShortcut(QKeySequence::Redo);

	cropAction = new QAction(QSL("&�ü�..."), this);
	cropAction->setEnabled(false);
	rotate90Action = new QAction(QSL("&˳ʱ����ת90��"), this);
	rotate180Action = new QAction(QSL("&��ת180��"), this);
	rotate270Action = new QAction(QSL("&��ʱ����ת90��"), this);
	rotateAction = new QAction(QSL("&����Ƕ�..."), this);
	horizontalFlipAction = new QAction(QSL("&ˮƽ��ת"), this);
	verticalFlipAction = new QAction(QSL("&��ֱ��ת"), this);
	changeLightnessAction = new QAction(QSL("&����..."), this);
	changeSaturationAction = new QAction(QSL("&���Ͷ�..."), this);
	changeHueAction = new QAction(QSL("&ɫ��..."), this);
	linearConvertAction = new QAction(QSL("&�ֶ����Ա任"), this);
	changeGammaAction = new QAction(QSL("&GammaУ��"), this);
	changeLogAction = new QAction(QSL("&�����任"), this);
	changePowAction = new QAction(QSL("&ָ���任"), this);
	histEquAction = new QAction(QSL("&ֱ��ͼ���⻯"), this);
	histSpecSMLAction = new QAction(QSL("&��ӳ�����"), this);
	histSpecGMLAction = new QAction(QSL("&��ӳ�����"), this);
	medianFilterAction = new QAction(QSL("&��ֵ�˲�"), this);
	gaussianFilterAction = new QAction(QSL("&��˹�˲�"), this);
	sharpenRobertFilterAction = new QAction(QSL("&Robert��������"), this);
	sharpenPrewittFilterAction = new QAction(QSL("&Prewitt����"), this);
	sharpenSobelFilterAction = new QAction(QSL("&Sobel����"), this);
	sharpenLaplaceFilterAction = new QAction(QSL("&Laplace����"), this);

	actionObservers = make_shared<vector<QAction*>>(initializer_list<QAction*>{
		saveFileAction, saveAsFileAction, rotate90Action,
		rotate180Action, rotate270Action, rotateAction,
		horizontalFlipAction, verticalFlipAction, changeLightnessAction,
		changeSaturationAction, changeHueAction, linearConvertAction,
		changeGammaAction, changeLogAction, changePowAction,
		histEquAction, histSpecSMLAction, histSpecGMLAction,
		medianFilterAction, gaussianFilterAction, sharpenRobertFilterAction,
		sharpenPrewittFilterAction, sharpenSobelFilterAction, sharpenLaplaceFilterAction
	});

	QMenu *fileMenu = menuBar()->addMenu(QSL("&�ļ�"));
	fileMenu->addAction(openFileAction);
	fileMenu->addAction(saveFileAction);
	fileMenu->addAction(saveAsFileAction);
	QMenu *editMenu = menuBar()->addMenu(QSL("&����"));
	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);
	QMenu *imageMenu = menuBar()->addMenu(QSL("&ͼ��"));
	imageMenu->addAction(cropAction);
	imageMenu->addSeparator();
	QMenu *rotateMenu = imageMenu->addMenu(QSL("&ͼ����ת"));
	rotateMenu->addAction(rotate90Action);
	rotateMenu->addAction(rotate180Action);
	rotateMenu->addAction(rotate270Action);
	rotateMenu->addAction(rotateAction);
	rotateMenu->addSeparator();
	rotateMenu->addAction(horizontalFlipAction);
	rotateMenu->addAction(verticalFlipAction);
	QMenu *changeMenu = imageMenu->addMenu(QSL("&����..."));
	changeMenu->addAction(changeLightnessAction);
	changeMenu->addAction(changeSaturationAction);
	changeMenu->addAction(changeHueAction);
	imageMenu->addSeparator();
	imageMenu->addAction(linearConvertAction);
	QMenu *nonLinearMenu = imageMenu->addMenu(QSL("&�����Ա任"));
	nonLinearMenu->addAction(changeGammaAction);
	nonLinearMenu->addAction(changeLogAction);
	nonLinearMenu->addAction(changePowAction);
	imageMenu->addAction(histEquAction);
	QMenu *histSpecMenu = imageMenu->addMenu(QSL("&ֱ��ͼ�涨��"));
	histSpecMenu->addAction(histSpecSMLAction);
	histSpecMenu->addAction(histSpecGMLAction);
	imageMenu->addSeparator();
	QMenu *spaceFilterMenu = imageMenu->addMenu(QSL("&�����˲���"));
	spaceFilterMenu->addAction(medianFilterAction);
	spaceFilterMenu->addAction(gaussianFilterAction);
	QMenu *sharpenMenu = spaceFilterMenu->addMenu(QSL("&��"));
	sharpenMenu->addAction(sharpenRobertFilterAction);
	sharpenMenu->addAction(sharpenPrewittFilterAction);
	sharpenMenu->addAction(sharpenSobelFilterAction);
	sharpenMenu->addAction(sharpenLaplaceFilterAction);

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
	connect(changeLightnessAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatLightness, QSL("����"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return exp(-d / 100); }, [](float d){ return -100 * log(d); }, QSL("���ȣ�"), 0, -150, 150)
	}));
	connect(changeSaturationAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatSaturation, QSL("���Ͷ�"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return 100 * d; }, QSL("���Ͷȣ�"), 0, -100, 100)
	}));
	connect(changeHueAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatHue, QSL("ɫ��"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d; }, [](float d){ return d; }, QSL("ɫ����"), 0, -180, 180)
	}));
	connect(changeGammaAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatGamma, QSL("GammaУ��"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return pow(10, d / 1000); }, [](float d){ return 1000 * log10(d); }, QSL("�ã�"), 0, -1400, 1400),
		ParameterInfo([](float d){ return pow(10, d / 100); }, [](float d){ return 100 * log10(d); }, QSL("c��"), 0, -100, 100)
	}));
	connect(changeLogAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatLog, QSL("�����任"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return d * 100; }, QSL("a��"), 0, -100, 100),
		ParameterInfo([](float d){ return pow(10, d / 100); }, [](float d){ return 100 * log10(d); }, QSL("b��"), 0, -100, 100),
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return 100 * d; }, QSL("c��"), 200, 105, 1000)
	}));
	connect(changePowAction, &QAction::triggered, this, bind(&DIPSoftware::uiChangeImage, this, &Utils::changePartialImageMatPow, QSL("ָ���任"), vector<ParameterInfo>{
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return d * 100; }, QSL("a��"), 0, -100, 100),
		ParameterInfo([](float d){ return d / 100; }, [](float d){ return 100 * d; }, QSL("b��"), 230, 101, 1000),
		ParameterInfo([](float d){ return pow(10, d / 100); }, [](float d){ return 100 * log10(d); }, QSL("c��"), 0, -100, 100)
	}));
	connect(linearConvertAction, &QAction::triggered, this, &DIPSoftware::linearConvertImage);
	connect(histEquAction, &QAction::triggered, this, &DIPSoftware::histEquImage);
	connect(histSpecSMLAction, &QAction::triggered, this, &DIPSoftware::histSpecSMLImage);
	connect(histSpecGMLAction, &QAction::triggered, this, &DIPSoftware::histSpecGMLImage);
	connect(medianFilterAction, &QAction::triggered, this, &DIPSoftware::medianFilterImage);
	connect(gaussianFilterAction, &QAction::triggered, this, &DIPSoftware::gaussianFilterImage);
	connect(sharpenRobertFilterAction, &QAction::triggered, this, bind(&DIPSoftware::sharpenImage, this, 0));
	connect(sharpenPrewittFilterAction, &QAction::triggered, this, bind(&DIPSoftware::sharpenImage, this, 1));
	connect(sharpenSobelFilterAction, &QAction::triggered, this, bind(&DIPSoftware::sharpenImage, this, 2));
	connect(sharpenLaplaceFilterAction, &QAction::triggered, this, bind(&DIPSoftware::sharpenImage, this, 3));

	//diagramWidget->setFixedSize(400, 300);
	histogramWidget->setFixedSize(400, 300);
	mainLayout->addWidget(imgWidget);
	//mainLayout->addWidget(diagramWidget, 0, Qt::AlignTop);
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
	QString inputFileName = QFileDialog::getOpenFileName(this, QSL("���ļ�"), " ", QSL("ͼ���ļ�(*.bmp;*.png;*.jpg;*.jpeg)"));
	if (!inputFileName.size()) {
		return;
	}
	currentFileName = String((const char *) inputFileName.toLocal8Bit());
	Mat imageMat = imread(currentFileName);
	imgWidget->setImageMat(imageMat);
	histogramWidget->setImageMat(imageMat);
	setActionsEnabled(true);
}

void DIPSoftware::saveFile() {
	imwrite(currentFileName, *(imgWidget->imgMat));
}

void DIPSoftware::saveAsFile() {
	QString inputFileName = QFileDialog::getSaveFileName(this, QSL("���Ϊ"), "", QSL("λͼ�ļ�(*.bmp);;PNG�ļ�(*.png);;JPEG�ļ�(*.jpg;*.jpeg)"));
	currentFileName = String((const char *) inputFileName.toLocal8Bit());
	imwrite(currentFileName, *(imgWidget->imgMat));
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
	float theta = QInputDialog::getDouble(this, QSL("��תͼ��"), QSL("��ת�Ƕȣ�˳ʱ�룩"), 0.0, -180.0, 180.0, 2, &ok);
	if (ok) {
		rotateImage(theta * PI / 180);
	}
}

void DIPSoftware::horizontalFlipImage() {
	Mat image = Mat(imgWidget->imgMat->rows, imgWidget->imgMat->cols, CV_8UC3);
	rep(j, image.cols) {
		imgWidget->imgMat->col(image.cols - j - 1).copyTo(image.col(j));
	}

	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::verticalFlipImage() {
	Mat image = Mat(imgWidget->imgMat->rows, imgWidget->imgMat->cols, CV_8UC3);
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
	} else if (deltas.size()) {
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *originMat, lambdaFunc(deltas)));
	} else {
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *originMat, *imgWidget->imgMat));
	}
}

void DIPSoftware::uiChangeImage(Utils::changeFuncType changeFunc, const QString &title, const vector<ParameterInfo>& infos) {
	auto lambdaFunc = [=](vector<float> d){ return Utils::changeImageMat(*originMat, d, changeFunc); };
	changeImage(lambdaFunc, [=](function<Mat(vector<float>)> lambdaFunc, bool& ok){ return InputPreviewDialog::changeFloat(this, imgWidget, lambdaFunc, title, infos, &ok); });
}

void DIPSoftware::linearConvertImage() {
	setOriginMat();

	bool ok;
	list<pair<float, float>> vertices = DiagramPreviewDialog::changeDiagram(this, imgWidget, bind(&Utils::linearConvert, *originMat, placeholders::_1), QSL("�ֶ����Ա任"), &ok);
	if (!ok) {
		imgWidget->setImageMat(*originMat);
	} else if (vertices.size()) {
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *originMat, Utils::linearConvert(*originMat, vertices)));
	} else {
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *originMat, *imgWidget->imgMat));
	}
}

void DIPSoftware::histEquImage() {
	Mat image = Utils::histogramEqualization(*imgWidget->imgMat);
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::histSpecSMLImage() {
	QString inputFileName = QFileDialog::getOpenFileName(this, QSL("���ļ�"), " ", QSL("ͼ���ļ�(*.bmp;*.png;*.jpg;*.jpeg)"));
	if (!inputFileName.size()) {
		return;
	}
	Mat patternMat = imread(String((const char *) inputFileName.toLocal8Bit()));
	Mat image = Utils::histogramSpecificationSML(*imgWidget->imgMat, patternMat);
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::histSpecGMLImage() {
	QString inputFileName = QFileDialog::getOpenFileName(this, QSL("���ļ�"), " ", QSL("ͼ���ļ�(*.bmp;*.png;*.jpg;*.jpeg)"));
	if (!inputFileName.size()) {
		return;
	}
	Mat patternMat = imread(String((const char *) inputFileName.toLocal8Bit()));
	Mat image = Utils::histogramSpecificationGML(*imgWidget->imgMat, patternMat);
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::medianFilterImage() {
	bool ok;
	int maxKernel = min(imgWidget->imgMat->rows, imgWidget->imgMat->cols);
	maxKernel = (maxKernel / 2) * 2 - 1;
	int size = QInputDialog::getInt(this, QSL("��ֵ�˲�"), QSL("����˴�С"), 3, 3, maxKernel, 2, &ok);
	if (ok) {
		Mat image = Utils::medianFilterImageMat(*imgWidget->imgMat, size);
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
	}
}

void DIPSoftware::gaussianFilterImage() {
	bool ok;
	int maxKernel = min(imgWidget->imgMat->rows, imgWidget->imgMat->cols);
	maxKernel = (maxKernel / 2) * 2 - 1;
	int size(QInputDialog::getInt(this, QSL("��˹�˲�"), QSL("����˴�С"), 3, 3, maxKernel, 2, &ok));
	if (ok) {
		Mat image = Utils::gaussianFilterImageMat(*imgWidget->imgMat, size, 1.0);
		undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
	}
}

void DIPSoftware::sharpenImage(int type) {
	Mat image = Utils::sharpenImageMat(*imgWidget->imgMat, type);
	undoStack->push(new EditImageCommand(imgWidget, histogramWidget, *imgWidget->imgMat, image));
}

void DIPSoftware::setActionsEnabled(bool enabled) {
	for (const auto& action : *actionObservers) {
		action->setEnabled(enabled);
	}
}