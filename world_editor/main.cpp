#include "world_editor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	tke::enginePath = "../";
	tke::init("TK Engine World Editor", 1280, 720);

	QApplication a(argc, argv);
	WorldEditor w;
	w.show();

	return a.exec();
}
