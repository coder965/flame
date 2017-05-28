#include "world_editor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	auto resCx = 1600, resCy = 900;
	tke::enginePath = "../";
	tke::init("TK Engine World Editor", resCx, resCy);

	QApplication a(argc, argv);
	WorldEditor w;
	w.show();

	return a.exec();
}
