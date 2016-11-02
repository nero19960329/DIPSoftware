 #include "InputPreviewDialog.h"
#include "Utils.h"

#include <QDebug>

using namespace cv;
using namespace std;

InputPreviewDialog::InputPreviewDialog(ImgWidget *widget, function<Mat(vector<float>)> lambdaFunc, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), 
	imgWidget(widget), title(nullptr), previewCheckBox(nullptr), buttonBox(nullptr), mainLayout(nullptr), matFloatFunc(lambdaFunc), previewFlag(true), parameterLen(0) {}

InputPreviewDialog::~InputPreviewDialog() {

}

void InputPreviewDialog::setParameterLen(int _parameterLen) {
	parameterLen = _parameterLen;
}

void InputPreviewDialog::ensureLayout() {
	if (mainLayout) {
		return;
	}

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &InputPreviewDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &InputPreviewDialog::reject);

	mainLayout = new QGridLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	
	rep(i, parameterLen) {
		mainLayout->addWidget(labels[i], i * 2, 0, 1, 4);
		mainLayout->addWidget(sliders[i], i * 2 + 1, 0, 1, 3);
		mainLayout->addWidget(spinBoxes[i], i * 2 + 1, 3, 1, 1);
	}
	mainLayout->addWidget(previewCheckBox, parameterLen * 2, 3, 1, 1);
	mainLayout->addWidget(buttonBox, parameterLen * 2 + 1, 0, 1, 4);
}

void InputPreviewDialog::ensureSliders() {
	if (!sliders.size()) {
		sliders = vector<QSlider*>(parameterLen);
		for (auto& slider : sliders) {
			slider = new QSlider(this);
			slider->setOrientation(Qt::Orientation::Horizontal);
		}
	}
}

void InputPreviewDialog::ensureSpinBoxes(vector<function<float(float)>> deltaFuncs, vector<function<float(float)>> invDeltaFuncs) {
	if (!spinBoxes.size()) {
		spinBoxes = vector<QDoubleSpinBox*>(parameterLen);
		rep(i, parameterLen) {
			spinBoxes[i] = new QDoubleSpinBox(this);
			QObject::connect(sliders[i], static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), spinBoxes[i], bind(&QDoubleSpinBox::setValue, spinBoxes[i], bind(deltaFuncs[i], placeholders::_1)));
			QObject::connect(spinBoxes[i], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), sliders[i], bind(&QSlider::setValue, sliders[i], bind(invDeltaFuncs[i], placeholders::_1)));
		}
	}
}

void InputPreviewDialog::ensureCheckBox(vector<function<float(float)>> deltaFuncs) {
	if (!previewCheckBox) {
		previewCheckBox = new QCheckBox(QSL("¿ªÆôÔ¤ÀÀ"), this);
		previewCheckBox->setChecked(true);
		QObject::connect(previewCheckBox, &QCheckBox::clicked, this, bind(&InputPreviewDialog::setPreviewMode, this, deltaFuncs, placeholders::_1));
	}
}

void InputPreviewDialog::setLabelText(const vector<QString>& texts) {
	if (!labels.size()) {
		labels = vector<QLabel*>(parameterLen);
		rep(i, parameterLen) {
			labels[i] = new QLabel(texts[i], this);
		}
	} else {
		rep(i, parameterLen) {
			labels[i]->setText(texts[i]);
		}
	}
}

void InputPreviewDialog::setRange(std::vector<std::function<float(float)>> deltaFuncs, const vector<float>& mins, const vector<float>& maxs) {
	rep(i, parameterLen) {
		sliders[i]->setRange(mins[i], maxs[i]);
		float bound[2] = { deltaFuncs[i](mins[i]), deltaFuncs[i](maxs[i]) };
		if (bound[0] < bound[1]) {
			spinBoxes[i]->setRange(bound[0], bound[1]);
		} else {
			spinBoxes[i]->setRange(bound[1], bound[0]);
		}
	}
}

void InputPreviewDialog::setValue(vector<function<float(float)>> deltaFuncs, const vector<float>& values) {
	rep(i, parameterLen) {
		sliders[i]->setValue(values[i]);
		spinBoxes[i]->setValue(deltaFuncs[i](values[i]));
	}
}

void InputPreviewDialog::setPreviewMode(vector<function<float(float)>> deltaFuncs, bool mode) {
	previewFlag = mode;

	if (previewFlag) {
		vector<float> values;
		rep(i, parameterLen) {
			values.push_back(deltaFuncs[i](sliders[i]->value()));
		}
		imgWidget->setImageMat(matFloatFunc(std::move(values)));
		for (const auto &slider : sliders) {
			QObject::connect(slider, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), this, [=](int d) {
				vector<float> values;
				rep(i, parameterLen) {
					values.push_back(deltaFuncs[i](sliders[i]->value()));
				}
				imgWidget->setImageMat(matFloatFunc(std::move(values)));
			});
			//QObject::connect(slider, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), this, bind(&ImgWidget::setImageMat, imgWidget, bind(matFloatFunc, placeholders::_1)));
		}
	} else {
		for (const auto &slider : sliders) {
			QObject::disconnect(slider, 0, this, 0);
		}
	}
}

vector<float> InputPreviewDialog::changeFloat(QWidget *parent, ImgWidget *widget, const function<Mat(vector<float>)>& lambdaFunc, const QString &title, const vector<ParameterInfo> &infos, bool *ok, Qt::WindowFlags flags) {
	vector<QString> texts;
	vector<float> values, minValues, maxValues;
	vector<function<float(float)>> deltaFuncs, invDeltaFuncs;
	for (const auto &info : infos) {
		deltaFuncs.push_back(info.deltaFunc);
		invDeltaFuncs.push_back(info.invDeltaFunc);
		texts.push_back(info.label);
		values.push_back(info.value);
		minValues.push_back(info.minValue);
		maxValues.push_back(info.maxValue);
	}
	
	InputPreviewDialog dialog(widget, lambdaFunc, parent, flags);
	dialog.setWindowTitle(title);
	dialog.setParameterLen(infos.size());
	dialog.setLabelText(texts);
	dialog.ensureSliders();
	dialog.setPreviewMode(deltaFuncs, true);
	dialog.ensureSpinBoxes(deltaFuncs, invDeltaFuncs);
	dialog.setRange(deltaFuncs, minValues, maxValues);
	dialog.setValue(deltaFuncs, values);
	dialog.ensureCheckBox(deltaFuncs);
	dialog.ensureLayout();

	int ret = dialog.exec();
	if (ok) {
		*ok = !!ret;
	}

	if (!dialog.previewFlag && ret) {
		vector<float> res;
		for (const auto &slider : dialog.sliders) {
			res.push_back(slider->value());
		}
		return res;
	} else {
		return {};
	}
}