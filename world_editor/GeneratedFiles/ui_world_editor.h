/********************************************************************************
** Form generated from reading UI file 'world_editor.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WORLD_EDITOR_H
#define UI_WORLD_EDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WorldEditorClass
{
public:
    QAction *action_save_selected_item;
    QAction *actionSave_Selected_Item_As;
    QAction *action_save_all;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *action_remove;
    QAction *action_view_Game_Explorer;
    QAction *actionUpdate_Changes;
    QAction *actionGame;
    QAction *actionGame_2;
    QAction *action_new_pipeline;
    QAction *actionPipeline_2;
    QAction *actionRenderer;
    QAction *actionScene;
    QAction *actionRenderer_2;
    QAction *actionScene_2;
    QAction *action_open_in_file_explorer;
    QAction *action_new_stage;
    QAction *action_compile;
    QAction *actionOutput;
    QAction *actionBuild;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuDebug;
    QMenu *menuShader;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *WorldEditorClass)
    {
        if (WorldEditorClass->objectName().isEmpty())
            WorldEditorClass->setObjectName(QStringLiteral("WorldEditorClass"));
        WorldEditorClass->resize(1032, 800);
        action_save_selected_item = new QAction(WorldEditorClass);
        action_save_selected_item->setObjectName(QStringLiteral("action_save_selected_item"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/WorldEditor/misc/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_save_selected_item->setIcon(icon);
        actionSave_Selected_Item_As = new QAction(WorldEditorClass);
        actionSave_Selected_Item_As->setObjectName(QStringLiteral("actionSave_Selected_Item_As"));
        action_save_all = new QAction(WorldEditorClass);
        action_save_all->setObjectName(QStringLiteral("action_save_all"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/WorldEditor/misc/save_all.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_save_all->setIcon(icon1);
        actionUndo = new QAction(WorldEditorClass);
        actionUndo->setObjectName(QStringLiteral("actionUndo"));
        actionRedo = new QAction(WorldEditorClass);
        actionRedo->setObjectName(QStringLiteral("actionRedo"));
        actionCut = new QAction(WorldEditorClass);
        actionCut->setObjectName(QStringLiteral("actionCut"));
        actionCopy = new QAction(WorldEditorClass);
        actionCopy->setObjectName(QStringLiteral("actionCopy"));
        actionPaste = new QAction(WorldEditorClass);
        actionPaste->setObjectName(QStringLiteral("actionPaste"));
        action_remove = new QAction(WorldEditorClass);
        action_remove->setObjectName(QStringLiteral("action_remove"));
        action_view_Game_Explorer = new QAction(WorldEditorClass);
        action_view_Game_Explorer->setObjectName(QStringLiteral("action_view_Game_Explorer"));
        actionUpdate_Changes = new QAction(WorldEditorClass);
        actionUpdate_Changes->setObjectName(QStringLiteral("actionUpdate_Changes"));
        actionGame = new QAction(WorldEditorClass);
        actionGame->setObjectName(QStringLiteral("actionGame"));
        actionGame_2 = new QAction(WorldEditorClass);
        actionGame_2->setObjectName(QStringLiteral("actionGame_2"));
        action_new_pipeline = new QAction(WorldEditorClass);
        action_new_pipeline->setObjectName(QStringLiteral("action_new_pipeline"));
        actionPipeline_2 = new QAction(WorldEditorClass);
        actionPipeline_2->setObjectName(QStringLiteral("actionPipeline_2"));
        actionRenderer = new QAction(WorldEditorClass);
        actionRenderer->setObjectName(QStringLiteral("actionRenderer"));
        actionScene = new QAction(WorldEditorClass);
        actionScene->setObjectName(QStringLiteral("actionScene"));
        actionRenderer_2 = new QAction(WorldEditorClass);
        actionRenderer_2->setObjectName(QStringLiteral("actionRenderer_2"));
        actionScene_2 = new QAction(WorldEditorClass);
        actionScene_2->setObjectName(QStringLiteral("actionScene_2"));
        action_open_in_file_explorer = new QAction(WorldEditorClass);
        action_open_in_file_explorer->setObjectName(QStringLiteral("action_open_in_file_explorer"));
        action_new_stage = new QAction(WorldEditorClass);
        action_new_stage->setObjectName(QStringLiteral("action_new_stage"));
        action_compile = new QAction(WorldEditorClass);
        action_compile->setObjectName(QStringLiteral("action_compile"));
        actionOutput = new QAction(WorldEditorClass);
        actionOutput->setObjectName(QStringLiteral("actionOutput"));
        actionBuild = new QAction(WorldEditorClass);
        actionBuild->setObjectName(QStringLiteral("actionBuild"));
        centralWidget = new QWidget(WorldEditorClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        WorldEditorClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(WorldEditorClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1032, 26));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuNew = new QMenu(menuFile);
        menuNew->setObjectName(QStringLiteral("menuNew"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QStringLiteral("menuView"));
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName(QStringLiteral("menuDebug"));
        menuShader = new QMenu(menuBar);
        menuShader->setObjectName(QStringLiteral("menuShader"));
        WorldEditorClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(WorldEditorClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        WorldEditorClass->setStatusBar(statusBar);
        toolBar = new QToolBar(WorldEditorClass);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        toolBar->setIconSize(QSize(21, 21));
        WorldEditorClass->addToolBar(Qt::TopToolBarArea, toolBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuShader->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuFile->addAction(menuNew->menuAction());
        menuFile->addAction(action_save_selected_item);
        menuFile->addAction(actionSave_Selected_Item_As);
        menuFile->addAction(action_save_all);
        menuFile->addAction(action_open_in_file_explorer);
        menuNew->addAction(actionGame);
        menuNew->addAction(action_new_stage);
        menuNew->addAction(action_new_pipeline);
        menuNew->addAction(actionRenderer);
        menuNew->addAction(actionScene);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addAction(action_remove);
        menuView->addAction(action_view_Game_Explorer);
        menuView->addAction(actionOutput);
        menuDebug->addAction(actionUpdate_Changes);
        menuShader->addAction(action_compile);
        menuShader->addAction(actionBuild);
        toolBar->addAction(action_save_selected_item);
        toolBar->addAction(action_save_all);

        retranslateUi(WorldEditorClass);

        QMetaObject::connectSlotsByName(WorldEditorClass);
    } // setupUi

    void retranslateUi(QMainWindow *WorldEditorClass)
    {
        WorldEditorClass->setWindowTitle(QApplication::translate("WorldEditorClass", "WorldEditor", Q_NULLPTR));
        action_save_selected_item->setText(QApplication::translate("WorldEditorClass", "Save Selected Item", Q_NULLPTR));
        actionSave_Selected_Item_As->setText(QApplication::translate("WorldEditorClass", "Save Selected Item As", Q_NULLPTR));
        action_save_all->setText(QApplication::translate("WorldEditorClass", "Save All", Q_NULLPTR));
        actionUndo->setText(QApplication::translate("WorldEditorClass", "Undo", Q_NULLPTR));
        actionRedo->setText(QApplication::translate("WorldEditorClass", "Redo", Q_NULLPTR));
        actionCut->setText(QApplication::translate("WorldEditorClass", "Cut", Q_NULLPTR));
        actionCopy->setText(QApplication::translate("WorldEditorClass", "Copy", Q_NULLPTR));
        actionPaste->setText(QApplication::translate("WorldEditorClass", "Paste", Q_NULLPTR));
        action_remove->setText(QApplication::translate("WorldEditorClass", "Remove", Q_NULLPTR));
        action_view_Game_Explorer->setText(QApplication::translate("WorldEditorClass", "Game Explorer", Q_NULLPTR));
        actionUpdate_Changes->setText(QApplication::translate("WorldEditorClass", "Update Changes", Q_NULLPTR));
        actionGame->setText(QApplication::translate("WorldEditorClass", "Game", Q_NULLPTR));
        actionGame_2->setText(QApplication::translate("WorldEditorClass", "Game", Q_NULLPTR));
        action_new_pipeline->setText(QApplication::translate("WorldEditorClass", "Pipeline", Q_NULLPTR));
        actionPipeline_2->setText(QApplication::translate("WorldEditorClass", "Pipeline", Q_NULLPTR));
        actionRenderer->setText(QApplication::translate("WorldEditorClass", "Renderer", Q_NULLPTR));
        actionScene->setText(QApplication::translate("WorldEditorClass", "Scene", Q_NULLPTR));
        actionRenderer_2->setText(QApplication::translate("WorldEditorClass", "Renderer", Q_NULLPTR));
        actionScene_2->setText(QApplication::translate("WorldEditorClass", "Scene", Q_NULLPTR));
        action_open_in_file_explorer->setText(QApplication::translate("WorldEditorClass", "Open In Explorer", Q_NULLPTR));
        action_new_stage->setText(QApplication::translate("WorldEditorClass", "Stage", Q_NULLPTR));
        action_compile->setText(QApplication::translate("WorldEditorClass", "Compile", Q_NULLPTR));
        actionOutput->setText(QApplication::translate("WorldEditorClass", "Output", Q_NULLPTR));
        actionBuild->setText(QApplication::translate("WorldEditorClass", "Build", Q_NULLPTR));
        menuFile->setTitle(QApplication::translate("WorldEditorClass", "File", Q_NULLPTR));
        menuNew->setTitle(QApplication::translate("WorldEditorClass", "New", Q_NULLPTR));
        menuEdit->setTitle(QApplication::translate("WorldEditorClass", "Edit", Q_NULLPTR));
        menuView->setTitle(QApplication::translate("WorldEditorClass", "View", Q_NULLPTR));
        menuDebug->setTitle(QApplication::translate("WorldEditorClass", "Debug", Q_NULLPTR));
        menuShader->setTitle(QApplication::translate("WorldEditorClass", "Build", Q_NULLPTR));
        toolBar->setWindowTitle(QApplication::translate("WorldEditorClass", "toolBar", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class WorldEditorClass: public Ui_WorldEditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WORLD_EDITOR_H
