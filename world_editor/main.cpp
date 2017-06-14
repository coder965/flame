#include "world_editor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	tke::init("../", 1280, 720);
	tke::initGeneralModels();

	QApplication a(argc, argv);
	WorldEditor w;
	w.show();

	return a.exec();
}
