#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QString>
#include <QVector2D>
#include <QVector>

class QMenu;
class QAction;

enum class DraftType
{
	Threading,
	Treadling
};

struct DraftData
{
	int version = 1;
	int warpEnds = 0;
	int shafts = 0;
	int treadles = 0;
	int weftPicks = 0;
	QVector<QVector<bool>> threading;
	QVector<QVector<bool>> tieup;
	QVector<QVector<bool>> treadling;
	QString name;
};

namespace Ui
{
class MainWindow;
}


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = 0);
	~MainWindow();

private slots:
	void on_actionQuit_triggered();
	void onDimensionsChanged();
	void onMyGLInitialized();
	void on_settingsButton_clicked();
	void on_updateShaderButton_clicked();
	void onSaveDraft();
	void onLoadDraft();
	void onLoadPresetTriggered();

private:
	Ui::MainWindow* ui;
	QVector2D m_patternScale;
	QColor m_warpColor;
	QColor m_weftColor;
	int m_warpMaterialId;
	int m_weftMaterialId;

	QList<DraftData> m_presets;
	QMenu* m_presetMenu;

	void updateShaderData();
	bool convertDraftGridToMapping(const QVector<QVector<bool>>& gridData,
								   DraftType type, QVector<int>& mappingArray,
								   int& errorCount);
	void loadDefaultPattern();
	DraftData getCurrentDraftData() const;
	void applyDraftData(const DraftData& data);
	void setupFileMenu();
	void setupPresets();
	QJsonArray gridToJson(const QVector<QVector<bool>>& gridData);
	QVector<QVector<bool>> jsonToGrid(const QJsonArray& jsonArray,
									  int expectedRows, int expectedCols);
};


#endif // MAINWINDOW_H
