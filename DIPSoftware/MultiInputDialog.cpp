#include "MultiInputDialog.h"
#include "Utils.h"

using namespace cv;
using namespace std;

MultiInputDialog::MultiInputDialog(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags),
	title(nullptr), buttonBox(nullptr), mainLayout(nullptr), parameterLen(0) {}

MultiInputDialog::~MultiInputDialog() {

}

void MultiInputDialog::setParameterLen(int _parameterLen) {
	parameterLen = _parameterLen;
}

void MultiInputDialog::ensureLayout() {
	if (mainLayout) {
		return;
	}

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &MultiInputDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &MultiInputDialog::reject);

	mainLayout = new QGridLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

	rep(i, parameterLen) {
		mainLayout->addWidget(labels[i], i * 2, 0, 1, 1);
		mainLayout->addWidget(spinBoxes[i], i * 2 + 1, 0, 1, 1);
	}
	mainLayout->addWidget(buttonBox, parameterLen * 2 + 1, 0, 1, 1);
}

void MultiInputDialog::ensureSpinBoxes() {
	if (!spinBoxes.size()) {
		spinBoxes = vector<QDoubleSpinBox*>(parameterLen);
		rep(i, parameterLen) spinBoxes[i] = new QDoubleSpinBox(this);
	}
}

void MultiInputDialog::setLabelText(const vector<QString>& texts) {
	if (!labels.size()) {
		labels = vector<QLabel*>(parameterLen);
		rep(i, parameterLen) labels[i] = new QLabel(texts[i], this);
	} else {
		rep(i, parameterLen) labels[i]->setText(texts[i]);
	}
}

void MultiInputDialog::setRange(const vector<float>& mins, const vector<float>& maxs) {
	rep(i, parameterLen) {
		spinBoxes[i]->setRange(mins[i], maxs[i]);
	}
}

void MultiInputDialog::setValue(const vector<float>& values) {
	rep(i, parameterLen) {
		spinBoxes[i]->setValue(values[i]);
	}
}

vector<float> MultiInputDialog::getFloats(QWidget *parent, const QString &title, const vector<ParameterInfo> &infos, bool *ok, Qt::WindowFlags flags) {
	vector<QString> texts;
	vector<float> values, minValues, maxValues;
	vector<function<float(float)>> deltaFuncs, invDeltaFuncs;
	for (const auto &info : infos) {
		texts.push_back(info.label);
		values.push_back(info.value);
		minValues.push_back(info.minValue);
		maxValues.push_back(info.maxValue);
	}

	MultiInputDialog dialog(parent, flags);
	dialog.setWindowTitle(title);
	dialog.setParameterLen(infos.size());
	dialog.setLabelText(texts);
	dialog.ensureSpinBoxes();
	dialog.setRange(minValues, maxValues);
	dialog.setValue(values);
	dialog.ensureLayout();

	int ret = dialog.exec();
	if (ok) *ok = !!ret;

	if (ret) {
		vector<float> res;
		for (const auto &slider : dialog.spinBoxes) res.push_back(slider->value());
		return res;
	} else return{};
}