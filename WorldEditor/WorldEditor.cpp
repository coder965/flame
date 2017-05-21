#include "WorldEditor.h"
#include <qwindow.h>
#include <qscrollarea.h>

#include "../src/window.h"

WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QWindow *window = QWindow::fromWinId((unsigned int)tke::currentWindow->hWnd);
	window->setFlags(Qt::FramelessWindowHint);

	QWidget *widget = QWidget::createWindowContainer(window);
	//QScrollArea *scrollArea = new QScrollArea;
	//scrollArea->setWidget(widget);

	ui.tabWidget->addTab(widget, "1");
}
