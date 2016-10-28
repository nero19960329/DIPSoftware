#include "DiagramPreviewDialog.h"
#include "Utils.h"

using namespace cv;
using namespace std;

DiagramPreviewDialog::DiagramPreviewDialog(ImgWidget *_imgWidget, matListType _matListFunc, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags),
	imgWidget(_imgWidget), matListFunc(_matListFunc), title(nullptr), previewCheckBox(nullptr), buttonBox(nullptr), mainLayout(nullptr), diagramWidget(nullptr), previewFlag(true) {}

DiagramPreviewDialog::~DiagramPreviewDialog() {}

void DiagramPreviewDialog::ensureLayout() {
	if (mainLayout) {
		return;
	}

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &DiagramPreviewDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &DiagramPreviewDialog::reject);

	mainLayout = new QVBoxLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

	mainLayout->addWidget(diagramWidget);
	mainLayout->addWidget(previewCheckBox);
	mainLayout->addWidget(buttonBox);
}

void DiagramPreviewDialog::ensureDiagram() {
	if (!diagramWidget) {
		diagramWidget = new DiagramWidget(this);
		diagramWidget->setFixedSize(300, 200);
	}
}

void DiagramPreviewDialog::ensureCheckBox() {
	if (!previewCheckBox) {
		previewCheckBox = new QCheckBox(QSL("¿ªÆôÔ¤ÀÀ"), this);
		previewCheckBox->setChecked(true);
		QObject::connect(previewCheckBox, &QCheckBox::clicked, this, &DiagramPreviewDialog::setPreviewMode);
	}
}

void DiagramPreviewDialog::setPreviewMode(bool mode) {
	previewFlag = mode;

	if (previewFlag) {
		imgWidget->setImageMat(matListFunc(diagramWidget->vertices));
		QObject::connect(diagramWidget, &DiagramWidget::valueChanged, this, [=](const list<pair<float, float>>& vertices) {
			imgWidget->setImageMat(matListFunc(vertices));
		});
	} else {
		QObject::disconnect(diagramWidget, 0, this, 0);
	}
}

list<pair<float, float>> DiagramPreviewDialog::changeDiagram(QWidget *parent, ImgWidget *_imgWidget, matListType _matListFunc, const QString &title, bool *ok, Qt::WindowFlags flags) {
	DiagramPreviewDialog dialog(_imgWidget, _matListFunc, parent, flags);
	dialog.setWindowTitle(title);
	dialog.ensureDiagram();
	dialog.ensureCheckBox();
	dialog.setPreviewMode(true);
	dialog.ensureLayout();

	int ret = dialog.exec();
	if (ok) {
		*ok = !!ret;
	}

	if (!dialog.previewFlag && ret) {
		return dialog.diagramWidget->vertices;
	} else {
		return {};
	}
}