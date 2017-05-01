/********************************************************************************
** Form generated from reading UI file 'QtGuiApplication.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTGUIAPPLICATION_H
#define UI_QTGUIAPPLICATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtGuiApplicationClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QToolButton *addButton;
    QToolButton *saveButton;
    QToolButton *removeButton;
    QListWidget *listWidget;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_2;
    QTreeWidget *treeWidget;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiApplicationClass)
    {
        if (QtGuiApplicationClass->objectName().isEmpty())
            QtGuiApplicationClass->setObjectName(QStringLiteral("QtGuiApplicationClass"));
        QtGuiApplicationClass->resize(1074, 816);
        centralWidget = new QWidget(QtGuiApplicationClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        groupBox_3 = new QGroupBox(splitter);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(groupBox_3);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(groupBox_3);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setMinimumSize(QSize(0, 21));
        addButton = new QToolButton(groupBox);
        addButton->setObjectName(QStringLiteral("addButton"));
        addButton->setGeometry(QRect(0, 0, 21, 21));
        addButton->setMaximumSize(QSize(21, 21));
        QIcon icon;
        icon.addFile(QStringLiteral(":/png/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        addButton->setIcon(icon);
        saveButton = new QToolButton(groupBox);
        saveButton->setObjectName(QStringLiteral("saveButton"));
        saveButton->setGeometry(QRect(60, 0, 21, 21));
        saveButton->setMaximumSize(QSize(21, 21));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/png/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveButton->setIcon(icon1);
        removeButton = new QToolButton(groupBox);
        removeButton->setObjectName(QStringLiteral("removeButton"));
        removeButton->setGeometry(QRect(30, 0, 21, 21));
        removeButton->setMaximumSize(QSize(21, 21));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/png/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        removeButton->setIcon(icon2);

        verticalLayout->addWidget(groupBox);

        listWidget = new QListWidget(groupBox_3);
        listWidget->setObjectName(QStringLiteral("listWidget"));

        verticalLayout->addWidget(listWidget);

        splitter->addWidget(groupBox_3);
        groupBox_4 = new QGroupBox(splitter);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(4);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy1);
        verticalLayout_2 = new QVBoxLayout(groupBox_4);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        treeWidget = new QTreeWidget(groupBox_4);
        treeWidget->setObjectName(QStringLiteral("treeWidget"));
        treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

        verticalLayout_2->addWidget(treeWidget);

        splitter->addWidget(groupBox_4);

        horizontalLayout->addWidget(splitter);

        QtGuiApplicationClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtGuiApplicationClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1074, 26));
        QtGuiApplicationClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(QtGuiApplicationClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtGuiApplicationClass->setStatusBar(statusBar);

        retranslateUi(QtGuiApplicationClass);

        QMetaObject::connectSlotsByName(QtGuiApplicationClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiApplicationClass)
    {
        QtGuiApplicationClass->setWindowTitle(QApplication::translate("QtGuiApplicationClass", "Render Editor", Q_NULLPTR));
        groupBox_3->setTitle(QString());
        groupBox->setTitle(QString());
        addButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        saveButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        removeButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        groupBox_4->setTitle(QString());
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(1, QApplication::translate("QtGuiApplicationClass", "Value", Q_NULLPTR));
        ___qtreewidgetitem->setText(0, QApplication::translate("QtGuiApplicationClass", "Name", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QtGuiApplicationClass: public Ui_QtGuiApplicationClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUIAPPLICATION_H
