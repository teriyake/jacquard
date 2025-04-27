#pragma once

#include <QColor>
#include <QDialog>
#include <QFrame>
#include <QVector2D>

namespace LoomMaterial
{
const int MAT_COTTON = 0;
const int MAT_SILK = 1;
const int MAT_WOOL_YARN = 2;
const int MAT_LINEN = 3;
const int MAT_METALLIC_GOLD = 4;
const int MAT_POLYESTER = 5;
const QStringList Names = {"Cotton", "Silk",		  "Wool Yarn",
						   "Linen",	 "Metallic Gold", "Polyester"};
} // namespace LoomMaterial

namespace Ui
{
class LoomSettingsDialog;
}

class LoomSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit LoomSettingsDialog(QWidget* parent = nullptr);
	~LoomSettingsDialog();

	void setPatternScale(const QVector2D& scale);
	void setWarpColor(const QColor& color);
	void setWeftColor(const QColor& color);
	void setWarpMaterial(int materialId);
	void setWeftMaterial(int materialId);

	QVector2D getPatternScale() const;
	QColor getWarpColor() const;
	QColor getWeftColor() const;
	int getWarpMaterial() const;
	int getWeftMaterial() const;


private slots:
	void on_buttonWarpColor_clicked();
	void on_buttonWeftColor_clicked();

private:
	Ui::LoomSettingsDialog* ui;
	QColor m_warpColor;
	QColor m_weftColor;

	void updateColorPreview(QFrame* frame, const QColor& color);
	void populateMaterialCombos();
};