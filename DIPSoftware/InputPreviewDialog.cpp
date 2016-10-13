#include "InputPreviewDialog.h"
#include "Utils.h"

#include <QDebug>

using namespace cv;
using namespace std;

InputPreviewDialog::InputPreviewDialog(ImgWidget *widget, function<Mat(int)> lambdaFunc, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	imgWidget = widget;
	title = 0;
	label = 0;
	slider = 0;
	spinBox = 0;
	previewCheckBox = 0;
	buttonBox = 0;
	mainLayout = 0;
	matIntFunc = lambdaFunc;
	previewFlag = true;
}

InputPreviewDialog::~InputPreviewDialog() {

}

void InputPreviewDialog::ensureLayout() {
	if (mainLayout) {
		return;
	}

	if (!slider) {
		ensureSlider();
	}

	if (!spinBox) {
		ensureSpinBox();
	}

	if (!label) {
		label = new QLabel(InputPreviewDialog::tr("Enter a value:"), this);
	}
#ifndef QT_NO_SHORTCUT
	label->setBuddy(spinBox);
#endif
	//label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &InputPreviewDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &InputPreviewDialog::reject);

	mainLayout = new QGridLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	mainLayout->addWidget(label, 0, 0, 1, 4);
	mainLayout->addWidget(slider, 1, 0, 1, 3);
	mainLayout->addWidget(spinBox, 1, 3, 1, 1);
	mainLayout->addWidget(previewCheckBox, 2, 3, 1, 1);
	mainLayout->addWidget(buttonBox, 3, 0, 1, 4);
}

void InputPreviewDialog::ensureSlider() {
	if (!slider) {
		slider = new QSlider(this);
		slider->setOrientation(Qt::Orientation::Horizontal);
		setPreviewMode(true);
	}
}

void InputPreviewDialog::ensureSpinBox() {
	if (!spinBox) {
		spinBox = new QSpinBox(this);
		QObject::connect(slider, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), spinBox, [this](int d){ spinBox->setValue(d); });
		QObject::connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), slider, [this](int d){ slider->setValue(d); });
	}
}

void InputPreviewDialog::ensureCheckBox() {
	if (!previewCheckBox) {
		previewCheckBox = new QCheckBox(QStringLiteral("¿ªÆôÔ¤ÀÀ"), this);
		previewCheckBox->setChecked(true);
		QObject::connect(previewCheckBox, &QCheckBox::clicked, this, &InputPreviewDialog::setPreviewMode);
	}
}

void InputPreviewDialog::setLabelText(const QString& text) {
	if (!label) {
		label = new QLabel(text, this);
	} else {
		label->setText(text);
	}
}

void InputPreviewDialog::setRange(int min, int max) {
	slider->setRange(min, max);
	spinBox->setRange(min, max);
}

void InputPreviewDialog::setValue(int value) {
	slider->setValue(value);
	spinBox->setValue(value);
}

void InputPreviewDialog::setPreviewMode(bool mode) {
	previewFlag = mode;

	if (previewFlag) {
		imgWidget->setImageMat(matIntFunc(slider->value()));
		QObject::connect(slider, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), this, [this](int d){ imgWidget->setImageMat(matIntFunc(d)); });
	} else {
		QObject::disconnect(slider, 0, this, 0);
	}
}

int InputPreviewDialog::changeInt(QWidget *parent, ImgWidget *widget, function<Mat(int)> lambdaFunc, const QString &title, const QString &label, int value, int minValue, int maxValue, bool *ok, Qt::WindowFlags flags) {
	InputPreviewDialog dialog(widget, lambdaFunc, parent, flags);
	dialog.setWindowTitle(title);
	dialog.setLabelText(label);
	dialog.ensureSlider();
	dialog.ensureSpinBox();
	dialog.setRange(minValue, maxValue);
	dialog.setValue(value);
	dialog.ensureCheckBox();
	dialog.ensureLayout();

	int ret = dialog.exec();
	if (ok) {
		*ok = !!ret;
	}

	if (!dialog.previewFlag && ret) {
		return dialog.slider->value();
	} else {
		return -2147483647;
	}
}