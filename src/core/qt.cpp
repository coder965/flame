#include "qt.h"
#include <qvalidator.h>

namespace tk
{
	QIntEdit::QIntEdit(int *_p, int min, int max)
	{
		p = _p;
		setValidator(new QIntValidator(min, max));
		setText(QString("%1").arg(*p));
		connect(this, &QIntEdit::textChanged, this, &QIntEdit::set);
	}

	void QIntEdit::set(const QString &str)
	{
		*p = str.toInt();
	}

	QFloatEdit::QFloatEdit(float *_p)
	{
		p = _p;
		setValidator(new QDoubleValidator());
		setText(QString("%1").arg(*p));
		connect(this, &QFloatEdit::textChanged, this, &QFloatEdit::set);
	}

	void QFloatEdit::set(const QString &str)
	{
		*p = str.toFloat();
	}

	QIntDropCombo::QIntDropCombo(int *_p)
	{
		p = _p;
		connect(this, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &QIntDropCombo::set);
	}

	void QIntDropCombo::set(int index)
	{
		*p = index;
	}

	QStringCombo::QStringCombo(std::string *_p)
	{
		p = _p;
		setEditable(true);
		setCurrentText(p->c_str());
		connect(this, (void(QComboBox::*)(const QString &str))&QComboBox::currentIndexChanged, this, &QStringCombo::set);
		connect(this, (void(QComboBox::*)(const QString &str))&QComboBox::currentTextChanged, this, &QStringCombo::set);
	}

	void QStringCombo::set(const QString &str)
	{
		*p = str.toUtf8().data();
	}

	QBoolCheck::QBoolCheck(bool *_p)
	{
		p = _p;
		setCheckState(*p ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	}

	void QBoolCheck::set(int index)
	{
		*p = (bool)index;
	}

	template<class T>
	void QTreeItemPair<T>::setup(QTreeWidget *tree, QTreeWidgetItem *parent, const char *name, std::string *str)
	{
		item = new QTreeWidgetItem;
		item->setText(0, name);
		if (str)
		{
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			item->setData(0, Qt::UserRole, (unsigned int)str);
		}
		if (tree == reinterpret_cast<QTreeWidget*>(parent))
			tree->addTopLevelItem(item);
		else
			parent->addChild(item);
		tree->setItemWidget(item, 1, partner);
	}

	template<class T>
	void QTreeItemPair<T>::setup(QTreeWidgetItem *parent, const char *name0, const char *name1)
	{
		item = new QTreeWidgetItem;
		item->setText(0, name0);
		item->setText(1, name1);
		parent->addChild(item);
	}

	template struct QTreeItemPair<QToolButton>;
	template struct QTreeItemPair<QIntEdit>;
	template struct QTreeItemPair<QComboBox>;
	template struct QTreeItemPair<QFloatEdit>;
	template struct QTreeItemPair<QCheckBox>;
	template struct QTreeItemPair<QLineEdit>;
	template struct QTreeItemPair<QIntDropCombo>;
	template struct QTreeItemPair<QStringCombo>;
	template struct QTreeItemPair<QBoolCheck>;
	template struct QTreeItemPair<QGroupBox>;
}