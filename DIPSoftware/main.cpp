#include "dipsoftware.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	DIPSoftware w;
	w.show();
	return a.exec();
}
