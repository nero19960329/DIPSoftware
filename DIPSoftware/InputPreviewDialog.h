#ifndef INPUTPREIVEWDIALOG_H
#define INPUTPREVIEWDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QSpinBox>
#include <QWidget>

#include <functional>

#include <opencv2/opencv.hpp>

#include "ImgWidget.h"

class InputPreviewDialog : public QDialog {
	Q_OBJECT

private:
	enum InputMode {
		SingleSliderInput
	};

	QLabel *title;
	QLabel *label;
	QSlider *slider;
	QSpinBox *spinBox;
	QCheckBox *previewCheckBox;
	QDialogButtonBox *buttonBox;

	QGridLayout *mainLayout;

	ImgWidget *imgWidget;
	std::function<cv::Mat(int)> matIntFunc;

	bool previewFlag;

private:
	InputPreviewDialog(ImgWidget *widget, std::function<cv::Mat(int)> lambdaFunc, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~InputPreviewDialog();

	void ensureLayout();
	void ensureSlider();
	void ensureSpinBox();
	void ensureCheckBox();
	void setLabelText(const QString& text);
	void setRange(int min, int max);
	void setValue(int value);

public slots:
	void setPreviewMode(bool mode);

public:
	static int changeInt(QWidget *parent, ImgWidget *widget, std::function<cv::Mat(int)> lambdaFunc, const QString &title, const QString &label, int value = 0, int minValue = -2147483647, int maxValue = 2147483647, bool *ok = 0, Qt::WindowFlags flags = 0);
};

#endif