#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

#include <functional>
#include <list>

#include "DiagramWidget.h"
#include "ImgWidget.h"

class DiagramPreviewDialog : public QDialog {
	Q_OBJECT

private:
	using matListType = std::function<cv::Mat(std::list<std::pair<float, float>>)>;

	QLabel *title;
	QCheckBox *previewCheckBox;
	QDialogButtonBox *buttonBox;

	QVBoxLayout *mainLayout;

	ImgWidget *imgWidget;
	matListType matListFunc;

	DiagramWidget *diagramWidget;

	bool previewFlag;

private:
	DiagramPreviewDialog(ImgWidget *_imgWidget, matListType _matListFunc, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~DiagramPreviewDialog();

	void ensureLayout();
	void ensureDiagram();
	void ensureCheckBox();

public slots:
	void setPreviewMode(bool mode);

public:
	static std::list<std::pair<float, float>> changeDiagram(QWidget *parent, ImgWidget *_imgWidget, matListType _matListFunc, const QString &title, bool *ok = 0, Qt::WindowFlags flags = 0);
};