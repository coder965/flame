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
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QToolButton *addPipelineToolButton;
    QToolButton *removePipelineToolButton;
    QToolButton *explorerPipelineFileToolButton;
    QGroupBox *groupBox_5;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *pipelineNameLineEdit;
    QToolButton *savePipelineToolButton;
    QTreeWidget *pipelineTree;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_4;
    QToolButton *addStageToolButton;
    QToolButton *removeStageToolButton;
    QToolButton *saveStageToolButton;
    QToolButton *toSpvToolButton;
    QToolButton *explorerStageFileToolButton;
    QSplitter *splitter_2;
    QTabWidget *stageTabWidget;
    QTabWidget *bottomTabWidget;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiApplicationClass)
    {
        if (QtGuiApplicationClass->objectName().isEmpty())
            QtGuiApplicationClass->setObjectName(QStringLiteral("QtGuiApplicationClass"));
        QtGuiApplicationClass->resize(947, 783);
        centralWidget = new QWidget(QtGuiApplicationClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
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
        addPipelineToolButton = new QToolButton(groupBox);
        addPipelineToolButton->setObjectName(QStringLiteral("addPipelineToolButton"));
        addPipelineToolButton->setGeometry(QRect(0, 0, 21, 21));
        QIcon icon;
        icon.addFile(QStringLiteral(":/png/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        addPipelineToolButton->setIcon(icon);
        removePipelineToolButton = new QToolButton(groupBox);
        removePipelineToolButton->setObjectName(QStringLiteral("removePipelineToolButton"));
        removePipelineToolButton->setGeometry(QRect(30, 0, 21, 21));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/png/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        removePipelineToolButton->setIcon(icon1);
        explorerPipelineFileToolButton = new QToolButton(groupBox);
        explorerPipelineFileToolButton->setObjectName(QStringLiteral("explorerPipelineFileToolButton"));
        explorerPipelineFileToolButton->setGeometry(QRect(60, 0, 21, 21));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/png/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        explorerPipelineFileToolButton->setIcon(icon2);

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
        pipelineNameLineEdit = new QLineEdit(groupBox_5);
        pipelineNameLineEdit->setObjectName(QStringLiteral("pipelineNameLineEdit"));

        horizontalLayout_2->addWidget(pipelineNameLineEdit);

        savePipelineToolButton = new QToolButton(groupBox_5);
        savePipelineToolButton->setObjectName(QStringLiteral("savePipelineToolButton"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/png/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        savePipelineToolButton->setIcon(icon3);

        horizontalLayout_2->addWidget(savePipelineToolButton);


        verticalLayout_2->addWidget(groupBox_5);

        pipelineTree = new QTreeWidget(groupBox_2);
        pipelineTree->headerItem()->setText(0, QString());
        pipelineTree->setObjectName(QStringLiteral("pipelineTree"));

        verticalLayout_2->addWidget(pipelineTree);

        splitter->addWidget(groupBox_2);
        groupBox_3 = new QGroupBox(splitter);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(4);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy2);
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
        addStageToolButton = new QToolButton(groupBox_4);
        addStageToolButton->setObjectName(QStringLiteral("addStageToolButton"));
        addStageToolButton->setGeometry(QRect(0, 0, 21, 21));
        addStageToolButton->setIcon(icon);
        removeStageToolButton = new QToolButton(groupBox_4);
        removeStageToolButton->setObjectName(QStringLiteral("removeStageToolButton"));
        removeStageToolButton->setGeometry(QRect(30, 0, 21, 21));
        removeStageToolButton->setIcon(icon1);
        saveStageToolButton = new QToolButton(groupBox_4);
        saveStageToolButton->setObjectName(QStringLiteral("saveStageToolButton"));
        saveStageToolButton->setGeometry(QRect(60, 0, 21, 21));
        saveStageToolButton->setIcon(icon3);
        toSpvToolButton = new QToolButton(groupBox_4);
        toSpvToolButton->setObjectName(QStringLiteral("toSpvToolButton"));
        toSpvToolButton->setGeometry(QRect(90, 0, 21, 21));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/png/spv.png"), QSize(), QIcon::Normal, QIcon::Off);
        toSpvToolButton->setIcon(icon4);
        explorerStageFileToolButton = new QToolButton(groupBox_4);
        explorerStageFileToolButton->setObjectName(QStringLiteral("explorerStageFileToolButton"));
        explorerStageFileToolButton->setGeometry(QRect(120, 0, 21, 21));
        explorerStageFileToolButton->setIcon(icon2);

        verticalLayout->addWidget(groupBox_4);

        splitter_2 = new QSplitter(groupBox_3);
        splitter_2->setObjectName(QStringLiteral("splitter_2"));
        splitter_2->setOrientation(Qt::Vertical);
        stageTabWidget = new QTabWidget(splitter_2);
        stageTabWidget->setObjectName(QStringLiteral("stageTabWidget"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(3);
        sizePolicy3.setHeightForWidth(stageTabWidget->sizePolicy().hasHeightForWidth());
        stageTabWidget->setSizePolicy(sizePolicy3);
        splitter_2->addWidget(stageTabWidget);
        bottomTabWidget = new QTabWidget(splitter_2);
        bottomTabWidget->setObjectName(QStringLiteral("bottomTabWidget"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(1);
        sizePolicy4.setHeightForWidth(bottomTabWidget->sizePolicy().hasHeightForWidth());
        bottomTabWidget->setSizePolicy(sizePolicy4);
        splitter_2->addWidget(bottomTabWidget);

        verticalLayout->addWidget(splitter_2);

        splitter->addWidget(groupBox_3);

        horizontalLayout->addWidget(splitter);

        QtGuiApplicationClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtGuiApplicationClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 947, 26));
        QtGuiApplicationClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(QtGuiApplicationClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtGuiApplicationClass->setStatusBar(statusBar);

        retranslateUi(QtGuiApplicationClass);

        QMetaObject::connectSlotsByName(QtGuiApplicationClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiApplicationClass)
    {
        QtGuiApplicationClass->setWindowTitle(QApplication::translate("QtGuiApplicationClass", "QtGuiApplication", Q_NULLPTR));
        groupBox_2->setTitle(QString());
        groupBox->setTitle(QString());
        addPipelineToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        removePipelineToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        explorerPipelineFileToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        groupBox_5->setTitle(QString());
        savePipelineToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        groupBox_3->setTitle(QString());
        groupBox_4->setTitle(QString());
        addStageToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        removeStageToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        saveStageToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        toSpvToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
        explorerStageFileToolButton->setText(QApplication::translate("QtGuiApplicationClass", "...", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QtGuiApplicationClass: public Ui_QtGuiApplicationClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUIAPPLICATION_H
