#ifndef __QFINDWIDGET__
#define __QFINDWIDGET__

struct QFindWidget : QObject
{
	QTextEdit *pEdit;

	QLineEdit *line;
	QLabel *label;
	QGroupBox *group;

	int strSize;
	std::vector<int> datas;
	int current;

	void on_find()
	{
		auto s = currentTabStage();
		if (!s) return;

		QList<QTextEdit::ExtraSelection> extraSelections;

		datas.clear();
		current = -1;

		auto findStr = line->text();
		Find::strSize = findStr.size();

		auto str = pEdit->toPlainText();

		if (Find::strSize > 0)
		{
			auto offset = 0;
			while (true)
			{
				offset = str.indexOf(findStr, offset + Find::strSize);
				if (offset == -1) break;

				Find::datas.push_back(offset);

				QTextEdit::ExtraSelection selection;

				QColor lineColor = QColor(Qt::yellow).lighter(160);

				selection.format.setBackground(lineColor);
				selection.cursor = pEdit->textCursor();
				selection.cursor.setPosition(offset);
				selection.cursor.setPosition(offset + Find::strSize, QTextCursor::KeepAnchor);
				extraSelections.append(selection);
			}
		}

		pEdit->setExtraSelections(extraSelections);

		update();
	}

	void on_find_enter()
	{
		gotoIndex(0);
	}

	void on_find_previous()
	{
		gotoIndex(Find::current - 1);
	}

	void on_find_next()
	{
		gotoIndex(Find::current + 1);
	}

	QFindWidget()
	{
		line = new QLineEdit;
		connect(line, &QLineEdit::textChanged, this, &QtGuiApplication::on_find);
		connect(line, &QLineEdit::returnPressed, this, &QtGuiApplication::on_find_enter);

		label = new QLabel;
		label->setText("0/0");
		auto btnUp = new QToolButton();
		btnUp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		btnUp->setGeometry(0, 0, 21, 21);
		btnUp->setMaximumSize(21, 21);
		btnUp->setIcon(QIcon(":/image/misc/up.png"));
		connect(btnUp, &QToolButton::clicked, this, &QtGuiApplication::on_find_previous);
		auto btnDown = new QToolButton();
		btnDown->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		btnDown->setGeometry(0, 0, 21, 21);
		btnDown->setMaximumSize(21, 21);
		btnDown->setIcon(QIcon(":/image/misc/down.png"));
		connect(btnDown, &QToolButton::clicked, this, &QtGuiApplication::on_find_next);

		auto layout = new QHBoxLayout;
		layout->addWidget(line);
		layout->addWidget(label);
		layout->addWidget(btnUp);
		layout->addWidget(btnDown);

		group = new QGroupBox();
		group->setLayout(layout);
		group->setStyleSheet("QGroupBox{border:0px;padding-top:-10px;padding-bottom:-10px;}");
	}

	~QFindWidget()
	{
		delete group;
	}

	void update()
	{
		Find::label->setText(QString("%1/%2").arg(current + 1).arg(datas.size()));
	}

	void gotoIndex(int index)
	{
		if (datas.size() == 0 || index < 0 || index >= datas.size() || current == index) return;

		auto stage = currentTabStage();
		if (!stage) return;

		pEdit->setFocus();

		auto cursor = pEdit->textCursor();
		cursor.setPosition(datas[index]);
		cursor.setPosition(datas[index] + strSize, QTextCursor::KeepAnchor);
		pEdit->setTextCursor(cursor);

		current = index;
		update();
	}
};

#endif