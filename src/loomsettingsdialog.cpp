#include "loomsettingsdialog.h"
#include "ui_loomsettingsdialog.h"
#include <QColorDialog>
#include <QDebug>

LoomSettingsDialog::LoomSettingsDialog(QWidget* parent)
	: QDialog(parent), ui(new Ui::LoomSettingsDialog), m_warpColor(Qt::white),
	  m_weftColor(Qt::black)
{
	ui->setupUi(this);
	populateMaterialCombos();

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
			&LoomSettingsDialog::accept);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this,
			&LoomSettingsDialog::reject);
}

LoomSettingsDialog::~LoomSettingsDialog()
{
	delete ui;
}

void LoomSettingsDialog::populateMaterialCombos()
{
	ui->comboWarpMaterial->clear();
	ui->comboWeftMaterial->clear();

	for (int i = 0; i < LoomMaterial::Names.size(); ++i)
	{
		ui->comboWarpMaterial->addItem(LoomMaterial::Names[i], i);
		ui->comboWeftMaterial->addItem(LoomMaterial::Names[i], i);
	}
}


void LoomSettingsDialog::updateColorPreview(QFrame* frame, const QColor& color)
{
	if (frame)
	{
		QPalette pal = frame->palette();
		pal.setColor(QPalette::Window, color);
		frame->setPalette(pal);
		frame->update();
	}
}

void LoomSettingsDialog::setPatternScale(const QVector2D& scale)
{
	ui->spinScaleX->setValue(scale.x());
	ui->spinScaleY->setValue(scale.y());
}

void LoomSettingsDialog::setWarpColor(const QColor& color)
{
	m_warpColor = color;
	updateColorPreview(ui->frameWarpColorPreview, m_warpColor);
}

void LoomSettingsDialog::setWeftColor(const QColor& color)
{
	m_weftColor = color;
	updateColorPreview(ui->frameWeftColorPreview, m_weftColor);
}

void LoomSettingsDialog::setWarpMaterial(int materialId)
{
	int index = ui->comboWarpMaterial->findData(materialId);
	if (index != -1)
	{
		ui->comboWarpMaterial->setCurrentIndex(index);
	}
	else
	{
		ui->comboWarpMaterial->setCurrentIndex(0);
	}
}

void LoomSettingsDialog::setWeftMaterial(int materialId)
{
	int index = ui->comboWeftMaterial->findData(materialId);
	if (index != -1)
	{
		ui->comboWeftMaterial->setCurrentIndex(index);
	}
	else
	{
		ui->comboWeftMaterial->setCurrentIndex(0);
	}
}

QVector2D LoomSettingsDialog::getPatternScale() const
{
	return QVector2D(ui->spinScaleX->value(), ui->spinScaleY->value());
}

QColor LoomSettingsDialog::getWarpColor() const
{
	return m_warpColor;
}

QColor LoomSettingsDialog::getWeftColor() const
{
	return m_weftColor;
}

int LoomSettingsDialog::getWarpMaterial() const
{
	return ui->comboWarpMaterial->currentData().toInt();
}

int LoomSettingsDialog::getWeftMaterial() const
{
	return ui->comboWeftMaterial->currentData().toInt();
}

void LoomSettingsDialog::on_buttonWarpColor_clicked()
{
	QColor newColor =
		QColorDialog::getColor(m_warpColor, this, "Select Warp Color");
	if (newColor.isValid())
	{
		m_warpColor = newColor;
		updateColorPreview(ui->frameWarpColorPreview, m_warpColor);
	}
}

void LoomSettingsDialog::on_buttonWeftColor_clicked()
{
	QColor newColor =
		QColorDialog::getColor(m_weftColor, this, "Select Weft Color");
	if (newColor.isValid())
	{
		m_weftColor = newColor;
		updateColorPreview(ui->frameWeftColorPreview, m_weftColor);
	}
}