/********************************************************************************
** Form generated from reading UI file 'WorldEditor.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WORLDEDITOR_H
#define UI_WORLDEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WorldEditorClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *WorldEditorClass)
    {
        if (WorldEditorClass->objectName().isEmpty())
            WorldEditorClass->setObjectName(QStringLiteral("WorldEditorClass"));
        WorldEditorClass->resize(909, 675);
        centralWidget = new QWidget(WorldEditorClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));

        horizontalLayout->addWidget(tabWidget);

        WorldEditorClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(WorldEditorClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 909, 26));
        WorldEditorClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(WorldEditorClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        WorldEditorClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(WorldEditorClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        WorldEditorClass->setStatusBar(statusBar);

        retranslateUi(WorldEditorClass);

        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(WorldEditorClass);
    } // setupUi

    void retranslateUi(QMainWindow *WorldEditorClass)
    {
        WorldEditorClass->setWindowTitle(QApplication::translate("WorldEditorClass", "WorldEditor", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class WorldEditorClass: public Ui_WorldEditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WORLDEDITOR_H
