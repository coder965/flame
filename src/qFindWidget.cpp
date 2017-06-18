#include "qFindWidget.h"

//void QFindWidget::on_find()
//{
//	if (!pEdit) return;
//
//	QList<QTextEdit::ExtraSelection> extraSelections;
//
//	datas.clear();
//	current = -1;
//
//	auto findStr = line->text();
//	strSize = findStr.size();
//
//	auto str = pEdit->toPlainText();
//
//	if (strSize > 0)
//	{
//		auto offset = 0;
//		while (true)
//		{
//			offset = str.indexOf(findStr, offset + strSize);
//			if (offset == -1) break;
//
//			datas.push_back(offset);
//
//			QTextEdit::ExtraSelection selection;
//
//			QColor lineColor = QColor(Qt::yellow).lighter(160);
//
//			selection.format.setBackground(lineColor);
//			selection.cursor = pEdit->textCursor();
//			selection.cursor.setPosition(offset);
//			selection.cursor.setPosition(offset + strSize, QTextCursor::KeepAnchor);
//			extraSelections.append(selection);
//		}
//	}
//
//	pEdit->setExtraSelections(extraSelections);
//
//	update();
//}
//
//void QFindWidget::on_find_enter()
//{
//	gotoIndex(0);
//}
//
//void QFindWidget::on_find_previous()
//{
//	gotoIndex(current - 1);
//}
//
//void QFindWidget::on_find_next()
//{
//	gotoIndex(current + 1);
//}
//
//QFindWidget::QFindWidget()
//{
//	line = new QLineEdit;
//	connect(line, &QLineEdit::textChanged, this, &QFindWidget::on_find);
//	connect(line, &QLineEdit::returnPressed, this, &QFindWidget::on_find_enter);
//
//	label = new QLabel;
//	label->setText("0/0");
//	auto btnUp = new QToolButton();
//	btnUp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//	btnUp->setGeometry(0, 0, 21, 21);
//	btnUp->setMaximumSize(21, 21);
//	btnUp->setIcon(QIcon(":/WorldEditor/misc/up.png"));
//	connect(btnUp, &QToolButton::clicked, this, &QFindWidget::on_find_previous);
//	auto btnDown = new QToolButton();
//	btnDown->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//	btnDown->setGeometry(0, 0, 21, 21);
//	btnDown->setMaximumSize(21, 21);
//	btnDown->setIcon(QIcon(":/WorldEditor/misc/down.png"));
//	connect(btnDown, &QToolButton::clicked, this, &QFindWidget::on_find_next);
//
//	auto layout = new QHBoxLayout;
//	layout->addWidget(line);
//	layout->addWidget(label);
//	layout->addWidget(btnUp);
//	layout->addWidget(btnDown);
//
//	group = new QGroupBox();
//	group->setLayout(layout);
//	group->setStyleSheet("QGroupBox{border:0px;padding-top:-10px;padding-bottom:-10px;}");
//}
//
//QFindWidget::~QFindWidget()
//{
//	delete group;
//}
//
//void QFindWidget::update()
//{
//	label->setText(QString("%1/%2").arg(current + 1).arg(datas.size()));
//}
//
//void QFindWidget::gotoIndex(int index)
//{
//	if (!pEdit || datas.size() == 0 || index < 0 || index >= datas.size() || current == index) return;
//
//	pEdit->setFocus();
//
//	auto cursor = pEdit->textCursor();
//	cursor.setPosition(datas[index]);
//	cursor.setPosition(datas[index] + strSize, QTextCursor::KeepAnchor);
//	pEdit->setTextCursor(cursor);
//
//	current = index;
//	update();
//}
