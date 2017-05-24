/********************************************************************************
** Form generated from reading UI file 'world_editor.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WORLD_EDITOR_H
#define UI_WORLD_EDITOR_H

#include <../src/qLineNumberEdit.h>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WorldEditorClass
{
public:
    QAction *actionLoad;
    QAction *actionSave;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QWidget *tab_2;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_3;
    QToolButton *addPipeline;
    QToolButton *removePipeline;
    QToolButton *savePipeline;
    QToolButton *explorerPipeline;
    QListWidget *pipelineList;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QTabWidget *tabWidget_2;
    QWidget *tab_3;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_4;
    QToolButton *addStage;
    QToolButton *removeStage;
    QToolButton *saveStage;
    QToolButton *toSpv;
    QToolButton *explorerStage;
    QTabWidget *stageTab;
    QWidget *tab_8;
    QHBoxLayout *horizontalLayout_7;
    QLineNumberEdit *vertEdit;
    QWidget *tab_9;
    QHBoxLayout *horizontalLayout_8;
    QLineNumberEdit *tescEdit;
    QWidget *tab_10;
    QHBoxLayout *horizontalLayout_9;
    QLineNumberEdit *teseEdit;
    QWidget *tab_11;
    QHBoxLayout *horizontalLayout_10;
    QLineNumberEdit *geomEdit;
    QWidget *tab_12;
    QHBoxLayout *horizontalLayout_11;
    QLineNumberEdit *fragEdit;
    QTabWidget *bottomTab;
    QWidget *tab_6;
    QHBoxLayout *horizontalLayout_6;
    QLineNumberEdit *outputText;
    QWidget *tab_7;
    QHBoxLayout *horizontalLayout_5;
    QTextBrowser *compileText;
    QWidget *tab_5;
    QMenuBar *menuBar;
    QMenu *menuScene;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *WorldEditorClass)
    {
        if (WorldEditorClass->objectName().isEmpty())
            WorldEditorClass->setObjectName(QStringLiteral("WorldEditorClass"));
        WorldEditorClass->resize(1032, 800);
        actionLoad = new QAction(WorldEditorClass);
        actionLoad->setObjectName(QStringLiteral("actionLoad"));
        actionSave = new QAction(WorldEditorClass);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        centralWidget = new QWidget(WorldEditorClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout_2 = new QHBoxLayout(centralWidget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        horizontalLayout = new QHBoxLayout(tab);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        scrollArea = new QScrollArea(tab);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 980, 675));
        scrollArea->setWidget(scrollAreaWidgetContents);

        horizontalLayout->addWidget(scrollArea);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        horizontalLayout_3 = new QHBoxLayout(tab_2);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        splitter = new QSplitter(tab_2);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        groupBox = new QGroupBox(splitter);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox_3 = new QGroupBox(groupBox);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy);
        groupBox_3->setMinimumSize(QSize(0, 21));
        groupBox_3->setMaximumSize(QSize(16777215, 21));
        addPipeline = new QToolButton(groupBox_3);
        addPipeline->setObjectName(QStringLiteral("addPipeline"));
        addPipeline->setGeometry(QRect(0, 0, 21, 21));
        QIcon icon;
        icon.addFile(QStringLiteral(":/WorldEditor/misc/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        addPipeline->setIcon(icon);
        removePipeline = new QToolButton(groupBox_3);
        removePipeline->setObjectName(QStringLiteral("removePipeline"));
        removePipeline->setGeometry(QRect(30, 0, 21, 21));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/WorldEditor/misc/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        removePipeline->setIcon(icon1);
        savePipeline = new QToolButton(groupBox_3);
        savePipeline->setObjectName(QStringLiteral("savePipeline"));
        savePipeline->setGeometry(QRect(60, 0, 21, 21));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/WorldEditor/misc/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        savePipeline->setIcon(icon2);
        explorerPipeline = new QToolButton(groupBox_3);
        explorerPipeline->setObjectName(QStringLiteral("explorerPipeline"));
        explorerPipeline->setGeometry(QRect(90, 0, 21, 21));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/WorldEditor/misc/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        explorerPipeline->setIcon(icon3);

        verticalLayout->addWidget(groupBox_3);

        pipelineList = new QListWidget(groupBox);
        pipelineList->setObjectName(QStringLiteral("pipelineList"));

        verticalLayout->addWidget(pipelineList);

        splitter->addWidget(groupBox);
        groupBox_2 = new QGroupBox(splitter);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        tabWidget_2 = new QTabWidget(groupBox_2);
        tabWidget_2->setObjectName(QStringLiteral("tabWidget_2"));
        tabWidget_2->setTabPosition(QTabWidget::East);
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        tabWidget_2->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        verticalLayout_2 = new QVBoxLayout(tab_4);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        groupBox_4 = new QGroupBox(tab_4);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        sizePolicy.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy);
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
        saveStage->setIcon(icon2);
        toSpv = new QToolButton(groupBox_4);
        toSpv->setObjectName(QStringLiteral("toSpv"));
        toSpv->setGeometry(QRect(90, 0, 21, 21));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/WorldEditor/misc/spv.png"), QSize(), QIcon::Normal, QIcon::Off);
        toSpv->setIcon(icon4);
        explorerStage = new QToolButton(groupBox_4);
        explorerStage->setObjectName(QStringLiteral("explorerStage"));
        explorerStage->setGeometry(QRect(120, 0, 21, 21));
        explorerStage->setIcon(icon3);

        verticalLayout_2->addWidget(groupBox_4);

        stageTab = new QTabWidget(tab_4);
        stageTab->setObjectName(QStringLiteral("stageTab"));
        tab_8 = new QWidget();
        tab_8->setObjectName(QStringLiteral("tab_8"));
        horizontalLayout_7 = new QHBoxLayout(tab_8);
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        vertEdit = new QLineNumberEdit(tab_8);
        vertEdit->setObjectName(QStringLiteral("vertEdit"));
        vertEdit->setEnabled(false);
        vertEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout_7->addWidget(vertEdit);

        stageTab->addTab(tab_8, QString());
        tab_9 = new QWidget();
        tab_9->setObjectName(QStringLiteral("tab_9"));
        horizontalLayout_8 = new QHBoxLayout(tab_9);
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        tescEdit = new QLineNumberEdit(tab_9);
        tescEdit->setObjectName(QStringLiteral("tescEdit"));
        tescEdit->setEnabled(false);
        tescEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout_8->addWidget(tescEdit);

        stageTab->addTab(tab_9, QString());
        tab_10 = new QWidget();
        tab_10->setObjectName(QStringLiteral("tab_10"));
        horizontalLayout_9 = new QHBoxLayout(tab_10);
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        teseEdit = new QLineNumberEdit(tab_10);
        teseEdit->setObjectName(QStringLiteral("teseEdit"));
        teseEdit->setEnabled(false);
        teseEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout_9->addWidget(teseEdit);

        stageTab->addTab(tab_10, QString());
        tab_11 = new QWidget();
        tab_11->setObjectName(QStringLiteral("tab_11"));
        horizontalLayout_10 = new QHBoxLayout(tab_11);
        horizontalLayout_10->setSpacing(6);
        horizontalLayout_10->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        geomEdit = new QLineNumberEdit(tab_11);
        geomEdit->setObjectName(QStringLiteral("geomEdit"));
        geomEdit->setEnabled(false);
        geomEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout_10->addWidget(geomEdit);

        stageTab->addTab(tab_11, QString());
        tab_12 = new QWidget();
        tab_12->setObjectName(QStringLiteral("tab_12"));
        horizontalLayout_11 = new QHBoxLayout(tab_12);
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        fragEdit = new QLineNumberEdit(tab_12);
        fragEdit->setObjectName(QStringLiteral("fragEdit"));
        fragEdit->setEnabled(false);
        fragEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout_11->addWidget(fragEdit);

        stageTab->addTab(tab_12, QString());

        verticalLayout_2->addWidget(stageTab);

        bottomTab = new QTabWidget(tab_4);
        bottomTab->setObjectName(QStringLiteral("bottomTab"));
        tab_6 = new QWidget();
        tab_6->setObjectName(QStringLiteral("tab_6"));
        horizontalLayout_6 = new QHBoxLayout(tab_6);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        outputText = new QLineNumberEdit(tab_6);
        outputText->setObjectName(QStringLiteral("outputText"));
        outputText->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout_6->addWidget(outputText);

        bottomTab->addTab(tab_6, QString());
        tab_7 = new QWidget();
        tab_7->setObjectName(QStringLiteral("tab_7"));
        horizontalLayout_5 = new QHBoxLayout(tab_7);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        compileText = new QTextBrowser(tab_7);
        compileText->setObjectName(QStringLiteral("compileText"));
        compileText->setLineWrapMode(QTextEdit::NoWrap);

        horizontalLayout_5->addWidget(compileText);

        bottomTab->addTab(tab_7, QString());

        verticalLayout_2->addWidget(bottomTab);

        tabWidget_2->addTab(tab_4, QString());

        horizontalLayout_4->addWidget(tabWidget_2);

        splitter->addWidget(groupBox_2);

        horizontalLayout_3->addWidget(splitter);

        tabWidget->addTab(tab_2, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QStringLiteral("tab_5"));
        tabWidget->addTab(tab_5, QString());

        horizontalLayout_2->addWidget(tabWidget);

        WorldEditorClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(WorldEditorClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1032, 26));
        menuScene = new QMenu(menuBar);
        menuScene->setObjectName(QStringLiteral("menuScene"));
        WorldEditorClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(WorldEditorClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        WorldEditorClass->setStatusBar(statusBar);

        menuBar->addAction(menuScene->menuAction());
        menuScene->addAction(actionLoad);
        menuScene->addAction(actionSave);

        retranslateUi(WorldEditorClass);

        tabWidget->setCurrentIndex(1);
        tabWidget_2->setCurrentIndex(1);
        stageTab->setCurrentIndex(0);
        bottomTab->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(WorldEditorClass);
    } // setupUi

    void retranslateUi(QMainWindow *WorldEditorClass)
    {
        WorldEditorClass->setWindowTitle(QApplication::translate("WorldEditorClass", "WorldEditor", Q_NULLPTR));
        actionLoad->setText(QApplication::translate("WorldEditorClass", "Load", Q_NULLPTR));
        actionSave->setText(QApplication::translate("WorldEditorClass", "Save", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("WorldEditorClass", "Scene", Q_NULLPTR));
        groupBox->setTitle(QString());
        groupBox_3->setTitle(QString());
        addPipeline->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        removePipeline->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        savePipeline->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        explorerPipeline->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        groupBox_2->setTitle(QString());
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_3), QApplication::translate("WorldEditorClass", "Attribute", Q_NULLPTR));
        groupBox_4->setTitle(QString());
        addStage->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        removeStage->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        saveStage->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        toSpv->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        explorerStage->setText(QApplication::translate("WorldEditorClass", "...", Q_NULLPTR));
        stageTab->setTabText(stageTab->indexOf(tab_8), QApplication::translate("WorldEditorClass", "vert", Q_NULLPTR));
        stageTab->setTabText(stageTab->indexOf(tab_9), QApplication::translate("WorldEditorClass", "tesc", Q_NULLPTR));
        stageTab->setTabText(stageTab->indexOf(tab_10), QApplication::translate("WorldEditorClass", "tese", Q_NULLPTR));
        stageTab->setTabText(stageTab->indexOf(tab_11), QApplication::translate("WorldEditorClass", "geom", Q_NULLPTR));
        stageTab->setTabText(stageTab->indexOf(tab_12), QApplication::translate("WorldEditorClass", "frag", Q_NULLPTR));
        bottomTab->setTabText(bottomTab->indexOf(tab_6), QApplication::translate("WorldEditorClass", "Output", Q_NULLPTR));
        bottomTab->setTabText(bottomTab->indexOf(tab_7), QApplication::translate("WorldEditorClass", "Compile", Q_NULLPTR));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_4), QApplication::translate("WorldEditorClass", "Shader", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("WorldEditorClass", "Pipeline Editor", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QApplication::translate("WorldEditorClass", "Renderer Editor", Q_NULLPTR));
        menuScene->setTitle(QApplication::translate("WorldEditorClass", "Scene", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class WorldEditorClass: public Ui_WorldEditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WORLD_EDITOR_H
