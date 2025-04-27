/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "draftgridwidget.h"
#include "mygl.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionQuit;
    QAction *actionSave_Draft;
    QAction *actionLoad_Draft;
    QAction *actionAbout;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *mainSplitter;
    MyGL *mygl;
    QWidget *rightControlWidget;
    QHBoxLayout *horizontalLayout_rightControl;
    QSplitter *mainVerticalSplitter;
    QSplitter *topHorizontalSplitter;
    QWidget *threadingContainerWidget;
    QVBoxLayout *verticalLayout_threading;
    QLabel *threadingLabel;
    QScrollArea *scrollAreaThreading;
    DraftGridWidget *threadingGrid;
    QWidget *tieupContainerWidget;
    QVBoxLayout *verticalLayout_tieup;
    QLabel *tieupLabel;
    QScrollArea *scrollAreaTieup;
    DraftGridWidget *tieupGrid;
    QSplitter *bottomHorizontalSplitter;
    QGroupBox *dimensionsGroup;
    QVBoxLayout *verticalLayout_groupBox;
    QFormLayout *formLayout;
    QLabel *labelWarpEnds;
    QSpinBox *spinWarpEnds;
    QLabel *labelShafts;
    QSpinBox *spinShafts;
    QLabel *labelTreadles;
    QSpinBox *spinTreadles;
    QLabel *labelWeftPicks;
    QSpinBox *spinWeftPicks;
    QVBoxLayout *verticalLayout_Buttons;
    QPushButton *updateShaderButton;
    QPushButton *settingsButton;
    QPushButton *envMapButton;
    QWidget *treadlingContainerWidget;
    QVBoxLayout *verticalLayout_treadling;
    QLabel *treadlingLabel;
    QScrollArea *scrollAreaTreadling;
    DraftGridWidget *treadlingGrid;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1024, 768);
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName("actionQuit");
        actionSave_Draft = new QAction(MainWindow);
        actionSave_Draft->setObjectName("actionSave_Draft");
        actionSave_Draft->setMenuRole(QAction::MenuRole::NoRole);
        actionLoad_Draft = new QAction(MainWindow);
        actionLoad_Draft->setObjectName("actionLoad_Draft");
        actionLoad_Draft->setMenuRole(QAction::MenuRole::NoRole);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName("actionAbout");
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName("horizontalLayout");
        mainSplitter = new QSplitter(centralWidget);
        mainSplitter->setObjectName("mainSplitter");
        mainSplitter->setOrientation(Qt::Orientation::Horizontal);
        mygl = new MyGL(mainSplitter);
        mygl->setObjectName("mygl");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(mygl->sizePolicy().hasHeightForWidth());
        mygl->setSizePolicy(sizePolicy);
        mygl->setMinimumSize(QSize(300, 300));
        mygl->setBaseSize(QSize(512, 512));
        mainSplitter->addWidget(mygl);
        rightControlWidget = new QWidget(mainSplitter);
        rightControlWidget->setObjectName("rightControlWidget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(rightControlWidget->sizePolicy().hasHeightForWidth());
        rightControlWidget->setSizePolicy(sizePolicy1);
        rightControlWidget->setMinimumSize(QSize(450, 0));
        horizontalLayout_rightControl = new QHBoxLayout(rightControlWidget);
        horizontalLayout_rightControl->setSpacing(6);
        horizontalLayout_rightControl->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_rightControl->setObjectName("horizontalLayout_rightControl");
        horizontalLayout_rightControl->setContentsMargins(0, 6, 0, 0);
        mainVerticalSplitter = new QSplitter(rightControlWidget);
        mainVerticalSplitter->setObjectName("mainVerticalSplitter");
        mainVerticalSplitter->setOrientation(Qt::Orientation::Vertical);
        mainVerticalSplitter->setChildrenCollapsible(false);
        topHorizontalSplitter = new QSplitter(mainVerticalSplitter);
        topHorizontalSplitter->setObjectName("topHorizontalSplitter");
        topHorizontalSplitter->setOrientation(Qt::Orientation::Horizontal);
        topHorizontalSplitter->setChildrenCollapsible(false);
        threadingContainerWidget = new QWidget(topHorizontalSplitter);
        threadingContainerWidget->setObjectName("threadingContainerWidget");
        verticalLayout_threading = new QVBoxLayout(threadingContainerWidget);
        verticalLayout_threading->setSpacing(6);
        verticalLayout_threading->setContentsMargins(11, 11, 11, 11);
        verticalLayout_threading->setObjectName("verticalLayout_threading");
        threadingLabel = new QLabel(threadingContainerWidget);
        threadingLabel->setObjectName("threadingLabel");

        verticalLayout_threading->addWidget(threadingLabel);

        scrollAreaThreading = new QScrollArea(threadingContainerWidget);
        scrollAreaThreading->setObjectName("scrollAreaThreading");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(scrollAreaThreading->sizePolicy().hasHeightForWidth());
        scrollAreaThreading->setSizePolicy(sizePolicy2);
        scrollAreaThreading->setMinimumSize(QSize(50, 50));
        scrollAreaThreading->setWidgetResizable(true);
        threadingGrid = new DraftGridWidget();
        threadingGrid->setObjectName("threadingGrid");
        threadingGrid->setGeometry(QRect(0, 0, 100, 100));
        scrollAreaThreading->setWidget(threadingGrid);

        verticalLayout_threading->addWidget(scrollAreaThreading);

        topHorizontalSplitter->addWidget(threadingContainerWidget);
        tieupContainerWidget = new QWidget(topHorizontalSplitter);
        tieupContainerWidget->setObjectName("tieupContainerWidget");
        verticalLayout_tieup = new QVBoxLayout(tieupContainerWidget);
        verticalLayout_tieup->setSpacing(6);
        verticalLayout_tieup->setContentsMargins(11, 11, 11, 11);
        verticalLayout_tieup->setObjectName("verticalLayout_tieup");
        tieupLabel = new QLabel(tieupContainerWidget);
        tieupLabel->setObjectName("tieupLabel");

        verticalLayout_tieup->addWidget(tieupLabel);

        scrollAreaTieup = new QScrollArea(tieupContainerWidget);
        scrollAreaTieup->setObjectName("scrollAreaTieup");
        sizePolicy2.setHeightForWidth(scrollAreaTieup->sizePolicy().hasHeightForWidth());
        scrollAreaTieup->setSizePolicy(sizePolicy2);
        scrollAreaTieup->setMinimumSize(QSize(50, 50));
        scrollAreaTieup->setWidgetResizable(true);
        tieupGrid = new DraftGridWidget();
        tieupGrid->setObjectName("tieupGrid");
        tieupGrid->setGeometry(QRect(0, 0, 100, 100));
        scrollAreaTieup->setWidget(tieupGrid);

        verticalLayout_tieup->addWidget(scrollAreaTieup);

        topHorizontalSplitter->addWidget(tieupContainerWidget);
        mainVerticalSplitter->addWidget(topHorizontalSplitter);
        bottomHorizontalSplitter = new QSplitter(mainVerticalSplitter);
        bottomHorizontalSplitter->setObjectName("bottomHorizontalSplitter");
        bottomHorizontalSplitter->setOrientation(Qt::Orientation::Horizontal);
        bottomHorizontalSplitter->setChildrenCollapsible(false);
        dimensionsGroup = new QGroupBox(bottomHorizontalSplitter);
        dimensionsGroup->setObjectName("dimensionsGroup");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(dimensionsGroup->sizePolicy().hasHeightForWidth());
        dimensionsGroup->setSizePolicy(sizePolicy3);
        verticalLayout_groupBox = new QVBoxLayout(dimensionsGroup);
        verticalLayout_groupBox->setSpacing(6);
        verticalLayout_groupBox->setContentsMargins(11, 11, 11, 11);
        verticalLayout_groupBox->setObjectName("verticalLayout_groupBox");
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName("formLayout");
        formLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
        labelWarpEnds = new QLabel(dimensionsGroup);
        labelWarpEnds->setObjectName("labelWarpEnds");

        formLayout->setWidget(0, QFormLayout::LabelRole, labelWarpEnds);

        spinWarpEnds = new QSpinBox(dimensionsGroup);
        spinWarpEnds->setObjectName("spinWarpEnds");
        spinWarpEnds->setMinimum(1);
        spinWarpEnds->setMaximum(256);
        spinWarpEnds->setValue(8);

        formLayout->setWidget(0, QFormLayout::FieldRole, spinWarpEnds);

        labelShafts = new QLabel(dimensionsGroup);
        labelShafts->setObjectName("labelShafts");

        formLayout->setWidget(1, QFormLayout::LabelRole, labelShafts);

        spinShafts = new QSpinBox(dimensionsGroup);
        spinShafts->setObjectName("spinShafts");
        spinShafts->setMinimum(1);
        spinShafts->setMaximum(64);
        spinShafts->setValue(4);

        formLayout->setWidget(1, QFormLayout::FieldRole, spinShafts);

        labelTreadles = new QLabel(dimensionsGroup);
        labelTreadles->setObjectName("labelTreadles");

        formLayout->setWidget(2, QFormLayout::LabelRole, labelTreadles);

        spinTreadles = new QSpinBox(dimensionsGroup);
        spinTreadles->setObjectName("spinTreadles");
        spinTreadles->setMinimum(1);
        spinTreadles->setMaximum(64);
        spinTreadles->setValue(6);

        formLayout->setWidget(2, QFormLayout::FieldRole, spinTreadles);

        labelWeftPicks = new QLabel(dimensionsGroup);
        labelWeftPicks->setObjectName("labelWeftPicks");

        formLayout->setWidget(3, QFormLayout::LabelRole, labelWeftPicks);

        spinWeftPicks = new QSpinBox(dimensionsGroup);
        spinWeftPicks->setObjectName("spinWeftPicks");
        spinWeftPicks->setMinimum(1);
        spinWeftPicks->setMaximum(256);
        spinWeftPicks->setValue(8);

        formLayout->setWidget(3, QFormLayout::FieldRole, spinWeftPicks);


        verticalLayout_groupBox->addLayout(formLayout);

        verticalLayout_Buttons = new QVBoxLayout();
        verticalLayout_Buttons->setSpacing(6);
        verticalLayout_Buttons->setObjectName("verticalLayout_Buttons");
        updateShaderButton = new QPushButton(dimensionsGroup);
        updateShaderButton->setObjectName("updateShaderButton");

        verticalLayout_Buttons->addWidget(updateShaderButton);

        settingsButton = new QPushButton(dimensionsGroup);
        settingsButton->setObjectName("settingsButton");

        verticalLayout_Buttons->addWidget(settingsButton);

        envMapButton = new QPushButton(dimensionsGroup);
        envMapButton->setObjectName("envMapButton");

        verticalLayout_Buttons->addWidget(envMapButton);


        verticalLayout_groupBox->addLayout(verticalLayout_Buttons);

        bottomHorizontalSplitter->addWidget(dimensionsGroup);
        treadlingContainerWidget = new QWidget(bottomHorizontalSplitter);
        treadlingContainerWidget->setObjectName("treadlingContainerWidget");
        verticalLayout_treadling = new QVBoxLayout(treadlingContainerWidget);
        verticalLayout_treadling->setSpacing(6);
        verticalLayout_treadling->setContentsMargins(11, 11, 11, 11);
        verticalLayout_treadling->setObjectName("verticalLayout_treadling");
        treadlingLabel = new QLabel(treadlingContainerWidget);
        treadlingLabel->setObjectName("treadlingLabel");

        verticalLayout_treadling->addWidget(treadlingLabel);

        scrollAreaTreadling = new QScrollArea(treadlingContainerWidget);
        scrollAreaTreadling->setObjectName("scrollAreaTreadling");
        sizePolicy2.setHeightForWidth(scrollAreaTreadling->sizePolicy().hasHeightForWidth());
        scrollAreaTreadling->setSizePolicy(sizePolicy2);
        scrollAreaTreadling->setMinimumSize(QSize(50, 50));
        scrollAreaTreadling->setWidgetResizable(true);
        treadlingGrid = new DraftGridWidget();
        treadlingGrid->setObjectName("treadlingGrid");
        treadlingGrid->setGeometry(QRect(0, 0, 100, 100));
        scrollAreaTreadling->setWidget(treadlingGrid);

        verticalLayout_treadling->addWidget(scrollAreaTreadling);

        bottomHorizontalSplitter->addWidget(treadlingContainerWidget);
        mainVerticalSplitter->addWidget(bottomHorizontalSplitter);

        horizontalLayout_rightControl->addWidget(mainVerticalSplitter);

        mainSplitter->addWidget(rightControlWidget);

        horizontalLayout->addWidget(mainSplitter);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1024, 24));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName("menuHelp");
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionSave_Draft);
        menuFile->addAction(actionLoad_Draft);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Jacquard", nullptr));
        actionQuit->setText(QCoreApplication::translate("MainWindow", "Quit", nullptr));
#if QT_CONFIG(shortcut)
        actionQuit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_Draft->setText(QCoreApplication::translate("MainWindow", "Save Draft", nullptr));
#if QT_CONFIG(shortcut)
        actionSave_Draft->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLoad_Draft->setText(QCoreApplication::translate("MainWindow", "Load Draft", nullptr));
#if QT_CONFIG(shortcut)
        actionLoad_Draft->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
        threadingLabel->setText(QCoreApplication::translate("MainWindow", "Threading (Shafts x Warp Ends)", nullptr));
        tieupLabel->setText(QCoreApplication::translate("MainWindow", "Tie-up (Shafts x Treadles)", nullptr));
        dimensionsGroup->setTitle(QCoreApplication::translate("MainWindow", "Loom Setup", nullptr));
        labelWarpEnds->setText(QCoreApplication::translate("MainWindow", "Warp Ends:", nullptr));
        labelShafts->setText(QCoreApplication::translate("MainWindow", "Shafts:", nullptr));
        labelTreadles->setText(QCoreApplication::translate("MainWindow", "Treadles:", nullptr));
        labelWeftPicks->setText(QCoreApplication::translate("MainWindow", "Weft Picks:", nullptr));
#if QT_CONFIG(tooltip)
        updateShaderButton->setToolTip(QCoreApplication::translate("MainWindow", "Apply current draft and settings to the preview", nullptr));
#endif // QT_CONFIG(tooltip)
        updateShaderButton->setText(QCoreApplication::translate("MainWindow", "Update Preview", nullptr));
#if QT_CONFIG(tooltip)
        settingsButton->setToolTip(QCoreApplication::translate("MainWindow", "Adjust thread colors, materials, and pattern scale", nullptr));
#endif // QT_CONFIG(tooltip)
        settingsButton->setText(QCoreApplication::translate("MainWindow", "Thread Settings", nullptr));
#if QT_CONFIG(tooltip)
        envMapButton->setToolTip(QCoreApplication::translate("MainWindow", "Load a high dynamic range image for environment lighting", nullptr));
#endif // QT_CONFIG(tooltip)
        envMapButton->setText(QCoreApplication::translate("MainWindow", "Load Env Map", nullptr));
        treadlingLabel->setText(QCoreApplication::translate("MainWindow", "Treadling (Treadles x Weft Picks)", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
