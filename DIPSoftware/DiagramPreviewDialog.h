#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

#include <list>

#include "DiagramWidget.h"
#include "ImgWidget.h"

class DiagramPreviewDialog : public QDialog {
	Q_OBJECT

private:
	QLabel *title;
	QCheckBox *previewCheckBox;
	QDialogButtonBox *buttonBox;

	QVBoxLayout *mainLayout;

	ImgWidget *imgWidget;

	DiagramWidget *diagramWidget;

	bool previewFlag;

private:
	DiagramPreviewDialog(ImgWidget *_imgWidget, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~DiagramPreviewDialog();

	void ensureLayout();
	void ensureDiagram();
	void ensureCheckBox();

public slots:
	void setPreviewMode(bool mode);

public:
	static std::list<std::pair<float, float>> changeDiagram(QWidget *parent, ImgWidget *_imgWidget, const QString &title, bool *ok = 0, Qt::WindowFlags flags = 0);
};