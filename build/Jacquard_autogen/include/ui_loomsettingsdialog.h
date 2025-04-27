/********************************************************************************
** Form generated from reading UI file 'loomsettingsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOOMSETTINGSDIALOG_H
#define UI_LOOMSETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_LoomSettingsDialog
{
public:
    QVBoxLayout *verticalLayout_Main;
    QGroupBox *groupBoxPattern;
    QFormLayout *formLayout_Pattern;
    QLabel *labelScaleX;
    QDoubleSpinBox *spinScaleX;
    QLabel *labelScaleY;
    QDoubleSpinBox *spinScaleY;
    QGroupBox *groupBoxWarp;
    QFormLayout *formLayout_Warp;
    QLabel *labelWarpColor;
    QHBoxLayout *horizontalLayout_WarpColor;
    QFrame *frameWarpColorPreview;
    QPushButton *buttonWarpColor;
    QSpacerItem *horizontalSpacer_Warp;
    QLabel *labelWarpMaterial;
    QComboBox *comboWarpMaterial;
    QGroupBox *groupBoxWeft;
    QFormLayout *formLayout_Weft;
    QLabel *labelWeftColor;
    QHBoxLayout *horizontalLayout_WeftColor;
    QFrame *frameWeftColorPreview;
    QPushButton *buttonWeftColor;
    QSpacerItem *horizontalSpacer_Weft;
    QLabel *labelWeftMaterial;
    QComboBox *comboWeftMaterial;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *LoomSettingsDialog)
    {
        if (LoomSettingsDialog->objectName().isEmpty())
            LoomSettingsDialog->setObjectName("LoomSettingsDialog");
        LoomSettingsDialog->resize(400, 350);
        verticalLayout_Main = new QVBoxLayout(LoomSettingsDialog);
        verticalLayout_Main->setObjectName("verticalLayout_Main");
        groupBoxPattern = new QGroupBox(LoomSettingsDialog);
        groupBoxPattern->setObjectName("groupBoxPattern");
        formLayout_Pattern = new QFormLayout(groupBoxPattern);
        formLayout_Pattern->setObjectName("formLayout_Pattern");
        labelScaleX = new QLabel(groupBoxPattern);
        labelScaleX->setObjectName("labelScaleX");

        formLayout_Pattern->setWidget(0, QFormLayout::LabelRole, labelScaleX);

        spinScaleX = new QDoubleSpinBox(groupBoxPattern);
        spinScaleX->setObjectName("spinScaleX");
        spinScaleX->setDecimals(3);
        spinScaleX->setMinimum(0.010000000000000);
        spinScaleX->setSingleStep(0.010000000000000);
        spinScaleX->setValue(0.200000000000000);

        formLayout_Pattern->setWidget(0, QFormLayout::FieldRole, spinScaleX);

        labelScaleY = new QLabel(groupBoxPattern);
        labelScaleY->setObjectName("labelScaleY");

        formLayout_Pattern->setWidget(1, QFormLayout::LabelRole, labelScaleY);

        spinScaleY = new QDoubleSpinBox(groupBoxPattern);
        spinScaleY->setObjectName("spinScaleY");
        spinScaleY->setDecimals(3);
        spinScaleY->setMinimum(0.010000000000000);
        spinScaleY->setSingleStep(0.010000000000000);
        spinScaleY->setValue(0.200000000000000);

        formLayout_Pattern->setWidget(1, QFormLayout::FieldRole, spinScaleY);


        verticalLayout_Main->addWidget(groupBoxPattern);

        groupBoxWarp = new QGroupBox(LoomSettingsDialog);
        groupBoxWarp->setObjectName("groupBoxWarp");
        formLayout_Warp = new QFormLayout(groupBoxWarp);
        formLayout_Warp->setObjectName("formLayout_Warp");
        labelWarpColor = new QLabel(groupBoxWarp);
        labelWarpColor->setObjectName("labelWarpColor");

        formLayout_Warp->setWidget(0, QFormLayout::LabelRole, labelWarpColor);

        horizontalLayout_WarpColor = new QHBoxLayout();
        horizontalLayout_WarpColor->setObjectName("horizontalLayout_WarpColor");
        frameWarpColorPreview = new QFrame(groupBoxWarp);
        frameWarpColorPreview->setObjectName("frameWarpColorPreview");
        frameWarpColorPreview->setMinimumSize(QSize(20, 20));
        frameWarpColorPreview->setMaximumSize(QSize(20, 20));
        frameWarpColorPreview->setAutoFillBackground(true);
        frameWarpColorPreview->setFrameShape(QFrame::StyledPanel);
        frameWarpColorPreview->setFrameShadow(QFrame::Raised);

        horizontalLayout_WarpColor->addWidget(frameWarpColorPreview);

        buttonWarpColor = new QPushButton(groupBoxWarp);
        buttonWarpColor->setObjectName("buttonWarpColor");

        horizontalLayout_WarpColor->addWidget(buttonWarpColor);

        horizontalSpacer_Warp = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_WarpColor->addItem(horizontalSpacer_Warp);


        formLayout_Warp->setLayout(0, QFormLayout::FieldRole, horizontalLayout_WarpColor);

        labelWarpMaterial = new QLabel(groupBoxWarp);
        labelWarpMaterial->setObjectName("labelWarpMaterial");

        formLayout_Warp->setWidget(1, QFormLayout::LabelRole, labelWarpMaterial);

        comboWarpMaterial = new QComboBox(groupBoxWarp);
        comboWarpMaterial->setObjectName("comboWarpMaterial");

        formLayout_Warp->setWidget(1, QFormLayout::FieldRole, comboWarpMaterial);


        verticalLayout_Main->addWidget(groupBoxWarp);

        groupBoxWeft = new QGroupBox(LoomSettingsDialog);
        groupBoxWeft->setObjectName("groupBoxWeft");
        formLayout_Weft = new QFormLayout(groupBoxWeft);
        formLayout_Weft->setObjectName("formLayout_Weft");
        labelWeftColor = new QLabel(groupBoxWeft);
        labelWeftColor->setObjectName("labelWeftColor");

        formLayout_Weft->setWidget(0, QFormLayout::LabelRole, labelWeftColor);

        horizontalLayout_WeftColor = new QHBoxLayout();
        horizontalLayout_WeftColor->setObjectName("horizontalLayout_WeftColor");
        frameWeftColorPreview = new QFrame(groupBoxWeft);
        frameWeftColorPreview->setObjectName("frameWeftColorPreview");
        frameWeftColorPreview->setMinimumSize(QSize(20, 20));
        frameWeftColorPreview->setMaximumSize(QSize(20, 20));
        frameWeftColorPreview->setAutoFillBackground(true);
        frameWeftColorPreview->setFrameShape(QFrame::StyledPanel);
        frameWeftColorPreview->setFrameShadow(QFrame::Raised);

        horizontalLayout_WeftColor->addWidget(frameWeftColorPreview);

        buttonWeftColor = new QPushButton(groupBoxWeft);
        buttonWeftColor->setObjectName("buttonWeftColor");

        horizontalLayout_WeftColor->addWidget(buttonWeftColor);

        horizontalSpacer_Weft = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_WeftColor->addItem(horizontalSpacer_Weft);


        formLayout_Weft->setLayout(0, QFormLayout::FieldRole, horizontalLayout_WeftColor);

        labelWeftMaterial = new QLabel(groupBoxWeft);
        labelWeftMaterial->setObjectName("labelWeftMaterial");

        formLayout_Weft->setWidget(1, QFormLayout::LabelRole, labelWeftMaterial);

        comboWeftMaterial = new QComboBox(groupBoxWeft);
        comboWeftMaterial->setObjectName("comboWeftMaterial");

        formLayout_Weft->setWidget(1, QFormLayout::FieldRole, comboWeftMaterial);


        verticalLayout_Main->addWidget(groupBoxWeft);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_Main->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(LoomSettingsDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_Main->addWidget(buttonBox);


        retranslateUi(LoomSettingsDialog);

        QMetaObject::connectSlotsByName(LoomSettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *LoomSettingsDialog)
    {
        LoomSettingsDialog->setWindowTitle(QCoreApplication::translate("LoomSettingsDialog", "Loom Settings", nullptr));
        groupBoxPattern->setTitle(QCoreApplication::translate("LoomSettingsDialog", "Pattern", nullptr));
        labelScaleX->setText(QCoreApplication::translate("LoomSettingsDialog", "Scale X:", nullptr));
        labelScaleY->setText(QCoreApplication::translate("LoomSettingsDialog", "Scale Y:", nullptr));
        groupBoxWarp->setTitle(QCoreApplication::translate("LoomSettingsDialog", "Warp Threads", nullptr));
        labelWarpColor->setText(QCoreApplication::translate("LoomSettingsDialog", "Color:", nullptr));
        buttonWarpColor->setText(QCoreApplication::translate("LoomSettingsDialog", "Choose...", nullptr));
        labelWarpMaterial->setText(QCoreApplication::translate("LoomSettingsDialog", "Material:", nullptr));
        groupBoxWeft->setTitle(QCoreApplication::translate("LoomSettingsDialog", "Weft Threads", nullptr));
        labelWeftColor->setText(QCoreApplication::translate("LoomSettingsDialog", "Color:", nullptr));
        buttonWeftColor->setText(QCoreApplication::translate("LoomSettingsDialog", "Choose...", nullptr));
        labelWeftMaterial->setText(QCoreApplication::translate("LoomSettingsDialog", "Material:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoomSettingsDialog: public Ui_LoomSettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOOMSETTINGSDIALOG_H
