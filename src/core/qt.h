#ifndef __TK_QT__
#define __TK_QT__

#include <qlineedit.h>
#include <qcombobox.h>
#include <qtreewidget.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qgroupbox.h>

#include "utils.h"

namespace tke
{
	struct QIntEdit : QLineEdit
	{
		int *p;

		QIntEdit(int *_p, int min = 0, int max = 0);
		void set(const QString &str);
	};

	struct QFloatEdit : QLineEdit
	{
		float *p;

		QFloatEdit(float *_p);
		void set(const QString &str);
	};

	struct QIntDropCombo : QComboBox
	{
		int *p;

		QIntDropCombo(int *_p);
		void set(int index);
	};

	struct QStringCombo : QComboBox
	{
		std::string *p;

		QStringCombo(std::string *_p);
		void set(const QString &str);
	};

	struct QBoolCheck : QCheckBox 
	{
		bool *p;

		QBoolCheck(bool *_p);
		void set(int index);
	};

	template<class T>
	struct QTreeItemPair
	{
		QTreeWidgetItem *item = nullptr;
		T *partner = nullptr;

		~QTreeItemPair();
		void setup(QTreeWidget *tree, QTreeWidgetItem *parent, const char *name, std::string *str = nullptr);
		void setup(QTreeWidgetItem *parent, const char *name0, const char *name1);
	};

	void qAddTreeItem(QTreeWidget *tree, QTreeWidgetItem *parent, void *p, Variable *r);
}

#endif