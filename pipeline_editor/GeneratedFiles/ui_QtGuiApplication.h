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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtGuiApplicationClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QToolButton *addPipeline;
    QToolButton *removePipeline;
    QToolButton *explorerPipeline;
    QGroupBox *groupBox_5;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *pipelineNameEdit;
    QToolButton *savePipeline;
    QListWidget *pipelineList;
    QTabWidget *tabWidget;
    QWidget *tab1;
    QHBoxLayout *horizontalLayout_4;
    QTreeWidget *attributeTree;
    QWidget *tab2;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_4;
    QToolButton *addStage;
    QToolButton *removeStage;
    QToolButton *saveStage;
    QToolButton *toSpv;
    QToolButton *explorerStage;
    QSplitter *splitter_2;
    QTabWidget *stageTab;
    QTabWidget *bottomTab;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiApplicationClass)
    {
        if (QtGuiApplicationClass->objectName().isEmpty())
            QtGuiApplicationClass->setObjectName(QStringLiteral("QtGuiApplicationClass"));
        QtGuiApplicationClass->resize(947, 783);
        centralWidget = new QWidget(QtGuiApplicationClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout_3 = new QHBoxLayout(centralWidget);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        groupBox_2 = new QGroupBox(splitter);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        groupBox = new QGroupBox(groupBox_2);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        groupBox->setMinimumSize(QSize(0, 21));
        groupBox->setMaximumSize(QSize(16777215, 21));
        addPipeline = new QToolButton(groupBox);
        addPipeline->setObjectName(QStringLiteral("addPipeline"));
        addPipeline->setGeometry(QRect(0, 0, 21, 21));
        QIcon icon;
        icon.addFile(QStringLiteral(":/image/misc/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        addPipeline->setIcon(icon);
        removePipeline = new QToolButton(groupBox);
        removePipeline->setObjectName(QStringLiteral("removePipeline"));
        removePipeline->setGeometry(QRect(30, 0, 21, 21));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/image/misc/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        removePipeline->setIcon(icon1);
        explorerPipeline = new QToolButton(groupBox);
        explorerPipeline->setObjectName(QStringLiteral("explorerPipeline"));
        explorerPipeline->setGeometry(QRect(60, 0, 21, 21));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/image/misc/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        explorerPipeline->setIcon(icon2);

        verticalLayout_2->addWidget(groupBox);

        groupBox_5 = new QGroupBox(groupBox_2);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        sizePolicy1.setHeightForWidth(groupBox_5->sizePolicy().hasHeightForWidth());
        groupBox_5->setSizePolicy(sizePolicy1);
        groupBox_5->setMinimumSize(QSize(0, 0));
        groupBox_5->setMaximumSize(QSize(16777215, 30));
        groupBox_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        horizontalLayout_2 = new QHBoxLayout(groupBox_5);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        pipelineNameEdit = new QLineEdit(groupBox_5);
        pipelineNameEdit->setObjectName(QStringLiteral("pipelineNameEdit"));

        horizontalLayout_2->addWidget(pipelineNameEdit);

        savePipeline = new QToolButton(groupBox_5);
        savePipeline->setObjectName(QStringLiteral("savePipeline"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/image/misc/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        savePipeline->setIcon(icon3);

        horizontalLayout_2->addWidget(savePipeline);


        verticalLayout_2->addWidget(groupBox_5);

        pipelineList = new QListWidget(groupBox_2);
        pipelineList->setObjectName(QStringLiteral("pipelineList"));

        verticalLayout_2->addWidget(pipelineList);

        splitter->addWidget(groupBox_2);
        tabWidget = new QTabWidget(splitter);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(4);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy2);
        tabWidget->setTabPosition(QTabWidget::East);
        tab1 = new QWidget();
        tab1->setObjectName(QStringLiteral("tab1"));
        horizontalLayout_4 = new QHBoxLayout(tab1);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        attributeTree = new QTreeWidget(tab1);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        attributeTree->setHeaderItem(__qtreewidgetitem);
        attributeTree->setObjectName(QStringLiteral("attributeTree"));

        horizontalLayout_4->addWidget(attributeTree);

        tabWidget->addTab(tab1, QString());
        tab2 = new QWidget();
        tab2->setObjectName(QStringLiteral("tab2"));
        horizontalLayout = new QHBoxLayout(tab2);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox_3 = new QGroupBox(tab2);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(4);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy3);
        verticalLayout = new QVBoxLayout(groupBox_3);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox_4 = new QGroupBox(groupBox_3);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        sizePolicy1.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy1);
        groupBox_4->setMinimumSize(QSize(0, 21));
        groupBox_4->setMaximumSize(QSize(16777215, 21));
        addStage = new QToolButton(groupBox_4);
        addStage->setObjectName(QStringLiteral("addStage"));
        addStage->setGeometry(QRect(0, 0, 21, 21));
        addStage->setIcon(icon);
        removeStage = new QToolButton(groupBox_4);
        removeStage->setObjectName(QStringLiteral("removeStage"));
        removeStage->setGeometry(QRect(30, 0, 21, 21));
        removeStage->setIcon(icon1);
        saveStage = new QToolButton(groupBox_4);
        saveStage->setObjectName(QStringLiteral("saveStage"));
        saveStage->setGeometry(QRect(60, 0, 21, 21));
        saveStage->setIcon(icon3);
        toSpv = new QToolButton(groupBox_4);
        toSpv->setObjectName(QStringLiteral("toSpv"));
        toSpv->setGeometry(QRect(90, 0, 21, 21));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/image/misc/spv.png"), QSize(), QIcon::Normal, QIcon::Off);
        toSpv->setIcon(icon4);
        explorerStage = new QToolButton(groupBox_4);
        explorerStage->setObjectName(QStringLiteral("explorerStage"));
        explorerStage->setGeometry(QRect(120, 0, 21, 21));
        explorerStage->setIcon(icon2);

        verticalLayout->addWidget(groupBox_4);

        splitter_2 = new QSplitter(groupBox_3);
        splitter_2->setObjectName(QStringLiteral("splitter_2"));
        splitter_2->setOrientation(Qt::Vertical);
        stageTab = new QTabWidget(splitter_2);
        stageTab->setObjectName(QStringLiteral("stageTab"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(3);
        sizePolicy4.setHeightForWidth(stageTab->sizePolicy().hasHeightForWidth());
        stageTab->setSizePolicy(sizePolicy4);
        splitter_2->addWidget(stageTab);
        bottomTab = new QTabWidget(splitter_2);
        bottomTab->setObjectName(QStringLiteral("bottomTab"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(1);
        sizePolicy5.setHeightForWidth(bottomTab->sizePolicy().hasHeightForWidth());
        bottomTab->setSizePolicy(sizePolicy5);
        splitter_2->addWidget(bottomTab);

        verticalLayout->addWidget(splitter_2);


        horizontalLayout->addWidget(groupBox_3);

        tabWidget->addTab(tab2, QString());
        splitter->addWidget(tabWidget);

        horizontalLayout_3->addWidget(splitter);

        QtGuiApplicationClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtGuiApplicationClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 947, 26));
        QtGuiApplicationClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(QtGuiApplicationClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtGuiApplicationClass->setStatusBar(statusBar);

        retranslateUi(QtGuiApplicationClass);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(QtGuiApplicationClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiApplicationClass)
    {
        QtGuiApplicationClass->setWindowTitle(QApplication::translate("QtGuiApplicationClass", "QtGuiApplication", Q_NULLPTR));
        groupBox_2->setTitle(QString());
        groupBox->setTitle(QString());
        addPipeline->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        removePipeline->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        explorerPipeline->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        groupBox_5->setTitle(QString());
        savePipeline->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab1), QApplication::translate("QtGuiApplicationClass", "Attribute", Q_NULLPTR));
        groupBox_3->setTitle(QString());
        groupBox_4->setTitle(QString());
        addStage->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        removeStage->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        saveStage->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        toSpv->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        explorerStage->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab2), QApplication::translate("QtGuiApplicationClass", "Shader", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QtGuiApplicationClass: public Ui_QtGuiApplicationClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUIAPPLICATION_H
