#include "world_editor.h"
#include <QTimer>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	tke::init("../", 1280, 720);

	QApplication a(argc, argv);
	WorldEditor w;
	w.show();

	return a.exec();
}
