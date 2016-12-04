#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QWidget>

#include <vector>

#include <opencv2/opencv.hpp>

class MultiInputDialog : public QDialog {
	Q_OBJECT
public:
	struct ParameterInfo {
		QString label;
		float value, minValue, maxValue;

		ParameterInfo() {}
		ParameterInfo(const QString& _label, float _value, float _minValue, float _maxValue) : 
			label(_label), value(_value), minValue(_minValue), maxValue(_maxValue) {}
	};

private:
	QLabel *title;
	std::vector<QLabel*> labels;
	std::vector<QDoubleSpinBox*> spinBoxes;
	QDialogButtonBox *buttonBox;

	QGridLayout *mainLayout;

	int parameterLen;

private:
	MultiInputDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~MultiInputDialog();

	void setParameterLen(int _parameterLen);
	void ensureLayout();
	void ensureSpinBoxes();
	void setLabelText(const std::vector<QString>& texts);
	void setRange(const std::vector<float>& mins, const std::vector<float>& maxs);
	void setValue(const std::vector<float>& values);

public:
	static std::vector<float> getFloats(QWidget *parent, const QString &title, const std::vector<ParameterInfo>& infos, bool *ok = 0, Qt::WindowFlags flags = 0);
};