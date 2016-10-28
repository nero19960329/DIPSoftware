#include "DiagramPreviewDialog.h"
#include "Utils.h"

using namespace std;

DiagramPreviewDialog::DiagramPreviewDialog(ImgWidget *_imgWidget, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags),
	imgWidget(_imgWidget), title(nullptr), previewCheckBox(nullptr), buttonBox(nullptr), mainLayout(nullptr), diagramWidget(nullptr), previewFlag(false) {}

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
		//QObject::connect();
	}
}

void DiagramPreviewDialog::setPreviewMode(bool mode) {

}

list<pair<float, float>> DiagramPreviewDialog::changeDiagram(QWidget *parent, ImgWidget *_imgWidget, const QString &title, bool *ok, Qt::WindowFlags flags) {
	DiagramPreviewDialog dialog(_imgWidget, parent, flags);
	dialog.setWindowTitle(title);
	dialog.ensureDiagram();
	dialog.ensureCheckBox();
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