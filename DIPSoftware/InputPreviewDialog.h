#ifndef INPUTPREIVEWDIALOG_H
#define INPUTPREVIEWDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QWidget>

#include <functional>
#include <vector>

#include <opencv2/opencv.hpp>

#include "ImgWidget.h"

struct ParameterInfo {
	std::function<float(float)> deltaFunc;
	std::function<float(float)> invDeltaFunc;
	QString label;
	float value, minValue, maxValue;

	ParameterInfo() {}
	ParameterInfo(std::function<float(float)> _deltaFunc, std::function<float(float)> _invDeltaFunc, const QString& _label, float _value, float _minValue, float _maxValue) : deltaFunc(_deltaFunc), invDeltaFunc(_invDeltaFunc), label(_label), value(_value), minValue(_minValue), maxValue(_maxValue) {}
};

class InputPreviewDialog : public QDialog {
	Q_OBJECT

private:
	QLabel *title;
	std::vector<QLabel*> labels;
	std::vector<QSlider*> sliders;
	std::vector<QDoubleSpinBox*> spinBoxes;
	QCheckBox *previewCheckBox;
	QDialogButtonBox *buttonBox;

	QGridLayout *mainLayout;

	ImgWidget *imgWidget;
	std::function<cv::Mat(std::vector<float>)> matFloatFunc;

	int parameterLen;
	bool previewFlag;

private:
	InputPreviewDialog(ImgWidget *widget, std::function<cv::Mat(std::vector<float>)> lambdaFunc, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~InputPreviewDialog();

	void setParameterLen(int _parameterLen);
	void ensureLayout();
	void ensureSliders();
	void ensureSpinBoxes(std::vector<std::function<float(float)>> deltaFuncs, std::vector<std::function<float(float)>> invDeltaFuncs);
	void ensureCheckBox(std::vector<std::function<float(float)>> deltaFuncs);
	void setLabelText(const std::vector<QString>& texts);
	void setRange(std::vector<std::function<float(float)>> deltaFuncs, const std::vector<float>& mins, const std::vector<float>& maxs);
	void setValue(std::vector<std::function<float(float)>> deltaFuncs, const std::vector<float>& values);

public slots:
	void setPreviewMode(std::vector<std::function<float(float)>> deltaFuncs, bool mode);

public:
	static std::vector<float> changeFloat(QWidget *parent, ImgWidget *widget, const std::function<cv::Mat(std::vector<float>)>& lambdaFunc, const QString &title, const std::vector<ParameterInfo>& infos, bool *ok = 0, Qt::WindowFlags flags = 0);
	//static std::vector<float> changeFloat(QWidget *parent, ImgWidget *widget, std::function<cv::Mat(std::vector<float>)> lambdaFunc, const QString &title, const QString &label, float value = 0, float minValue = -2147483647, float maxValue = 2147483647, bool *ok = 0, Qt::WindowFlags flags = 0);
};

#endif