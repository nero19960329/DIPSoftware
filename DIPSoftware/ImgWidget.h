#pragma once

#include <QAction>
#include <QContextMenuEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMenu>
#include <QRect>
#include <QWidget>

#include <opencv2/opencv.hpp>

#include <memory>

class EditPixmapItem : public QObject, public QGraphicsPixmapItem {
	Q_OBJECT

public:
	EditPixmapItem(const QPixmap& pixmap);
	~EditPixmapItem() {}

	QRect getCropRect();

private:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
	void updateCropRectSignal();

private:
	bool isDrag;
	int originX, originY;
	QRect cropRect;
};

class ImgWidget : public QWidget {
	Q_OBJECT

public:
	ImgWidget(QWidget *parent);
	~ImgWidget();

	void setImageMat(const cv::Mat &mat);
	QRect getCropRect();
	void removeLastItem();

signals:
	void setCropActionEnabled(bool);

private:
	void contextMenuEvent(QContextMenuEvent *event);

public slots:
	void updateRectItem();

private:
	QPixmap *pixmap;
	EditPixmapItem *pixmapItem;
	QGraphicsRectItem *rectItem;
	QGraphicsScene *scene;
	QGraphicsView *view;

	QHBoxLayout *mainLayout;

	QMenu *popMenu;
	QAction *cancelCropRectAction;

public:
	std::shared_ptr<cv::Mat> imgMat;
};