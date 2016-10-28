#include "ImgWidget.h"
#include "Utils.h"

#include <QCursor>
#include <QDebug>
#include <QPainter>
#include <QPixmap>

using namespace cv;
using namespace std;

EditPixmapItem::EditPixmapItem(const QPixmap& pixmap) : QGraphicsPixmapItem(pixmap), isDrag(false) {
	setCursor(Qt::CrossCursor);
}

QRect EditPixmapItem::getCropRect() {
	return cropRect;
}

void EditPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		QPointF p = event->pos();
		isDrag = true;
		originX = p.x();
		originY = p.y();
	}
}

void EditPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if (isDrag) {
		QPointF p = event->pos();
		int leftUpX, leftUpY, w, h;
		if (originX < p.x()) {
			leftUpX = originX;
			w = p.x() - originX;
		} else {
			leftUpX = p.x();
			w = originX - leftUpX;
		}
		if (originY < p.y()) {
			leftUpY = originY;
			h = p.y() - leftUpY;
		} else {
			leftUpY = p.y();
			h = originY - leftUpY;
		}

		cropRect = QRect(leftUpX, leftUpY, w, h);
		emit(updateCropRectSignal());
	}
}

void EditPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		isDrag = false;
	}
}

ImgWidget::ImgWidget(QWidget *parent) : QWidget(parent) {
	setMouseTracking(true);

	scene = new QGraphicsScene(this);
	view = new QGraphicsView(scene);
	pixmap = nullptr;
	pixmapItem = nullptr;
	rectItem = nullptr;
	imgMat = nullptr;
	mainLayout = new QHBoxLayout;

	popMenu = new QMenu;
	cancelCropRectAction = new QAction(QSL("取消选中"), this);

	connect(cancelCropRectAction, &QAction::triggered, this, &ImgWidget::removeLastItem);

	view->setBackgroundBrush(QBrush(QColor(40, 40, 40), Qt::SolidPattern));
	setContextMenuPolicy(Qt::DefaultContextMenu);
	mainLayout->addWidget(view);
	setLayout(mainLayout);
}

ImgWidget::~ImgWidget() {

}

void ImgWidget::setImageMat(const Mat& mat) {
	imgMat = make_shared<Mat>(mat);

	if (pixmap) {
		delete pixmap;
	}
	pixmap = new QPixmap;
	pixmap->convertFromImage(Utils::mat2QImage(*imgMat));

	if (pixmapItem) {
		delete pixmapItem;
	}
	pixmapItem = new EditPixmapItem(*pixmap);
	QObject::connect(pixmapItem, &EditPixmapItem::updateCropRectSignal, this, &ImgWidget::updateRectItem);
	scene->addItem(pixmapItem);
	view->setSceneRect(0, 0, pixmap->width(), pixmap->height());
}

QRect ImgWidget::getCropRect() {
	return pixmapItem->getCropRect();
}

void ImgWidget::removeLastItem() {
	if (rectItem) {
		scene->removeItem(rectItem);
		delete rectItem;
		rectItem = nullptr;
		view->viewport()->update();
		emit(setCropActionEnabled(false));
	}
}

void ImgWidget::contextMenuEvent(QContextMenuEvent *event) {
	popMenu->clear();
	popMenu->addAction(cancelCropRectAction);

	popMenu->exec(QCursor::pos());
	event->accept();
}

void ImgWidget::updateRectItem() {
	if (rectItem) {
		scene->removeItem(rectItem);
		delete rectItem;
	}
	rectItem = new QGraphicsRectItem(pixmapItem->getCropRect());
	rectItem->setPen(QPen(Qt::DashLine));
	scene->addItem(rectItem);
	view->viewport()->update();
	emit(setCropActionEnabled(true));
}