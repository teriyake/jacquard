#include "mainwindow.h"
#include <QMessageBox>

#include "loomsettingsdialog.h"
#include <ui_mainwindow.h>

#include <QAction>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), m_presetMenu(nullptr)
{
	ui->setupUi(this);
	ui->mygl->setFocus();
	ui->treadlingGrid->setRotated(true);
	ui->scrollAreaTreadling->setHorizontalScrollBarPolicy(
		Qt::ScrollBarAlwaysOff);
	ui->scrollAreaTreadling->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollAreaTreadling->setMinimumHeight(100);

	m_patternScale = QVector2D(0.2f, 0.2f);
	m_warpColor = QColor(204, 204, 204);
	m_weftColor = QColor(77, 77, 77);
	m_warpMaterialId = LoomMaterial::MAT_COTTON;
	m_weftMaterialId = LoomMaterial::MAT_COTTON;

	setupFileMenu();
	setupPresets();

	connect(ui->mygl, &MyGL::glInitialized, this,
			&MainWindow::onMyGLInitialized);

	connect(ui->envMapButton, SIGNAL(clicked()), ui->mygl,
			SLOT(slot_loadEnvMap()));

	connect(ui->spinWarpEnds, SIGNAL(valueChanged(int)), this,
			SLOT(onDimensionsChanged()));
	connect(ui->spinShafts, SIGNAL(valueChanged(int)), this,
			SLOT(onDimensionsChanged()));
	connect(ui->spinTreadles, SIGNAL(valueChanged(int)), this,
			SLOT(onDimensionsChanged()));
	connect(ui->spinWeftPicks, SIGNAL(valueChanged(int)), this,
			SLOT(onDimensionsChanged()));

	loadDefaultPattern();
	onDimensionsChanged();

	statusBar()->showMessage("Ready.", 2000);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setupFileMenu()
{
	connect(ui->actionSave_Draft, &QAction::triggered, this,
			&MainWindow::onSaveDraft);
	connect(ui->actionLoad_Draft, &QAction::triggered, this,
			&MainWindow::onLoadDraft);

	m_presetMenu = new QMenu("Load Preset", this);
	QList<QAction*> fileActions = ui->menuFile->actions();
	QAction* quitAction = ui->actionQuit;
	QAction* insertBeforeAction = quitAction;
	for (int i = 0; i < fileActions.size(); ++i)
	{
		if (fileActions[i]->isSeparator() && i + 1 < fileActions.size() &&
			fileActions[i + 1] == quitAction)
		{
			insertBeforeAction = fileActions[i];
			break;
		}
	}
	ui->menuFile->insertMenu(insertBeforeAction, m_presetMenu);
	ui->menuFile->insertSeparator(insertBeforeAction);
}

void MainWindow::setupPresets()
{
	DraftData defaultPreset;
	defaultPreset.name = "Point Twill (Default)";
	defaultPreset.warpEnds = 8;
	defaultPreset.shafts = 4;
	defaultPreset.treadles = 6;
	defaultPreset.weftPicks = 8;
	defaultPreset.threading.resize(defaultPreset.shafts);
	for (auto& row : defaultPreset.threading)
		row.resize(defaultPreset.warpEnds, false);
	const int defaultThreadingRaw[] = {0, 1, 2, 3, 0, 1, 2, 3};
	for (int warpIdx = 0; warpIdx < defaultPreset.warpEnds; ++warpIdx)
	{
		int shaftId = defaultThreadingRaw[warpIdx];
		if (shaftId >= 0 && shaftId < defaultPreset.shafts)
		{
			defaultPreset.threading[shaftId][warpIdx] = true;
		}
	}
	defaultPreset.tieup.resize(defaultPreset.shafts);
	for (auto& row : defaultPreset.tieup)
		row.resize(defaultPreset.treadles, false);
	const int defaultTieupRaw[] = {1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1,
								   0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1};
	for (int shaftIdx = 0; shaftIdx < defaultPreset.shafts; ++shaftIdx)
	{
		for (int treadleIdx = 0; treadleIdx < defaultPreset.treadles;
			 ++treadleIdx)
		{
			int flatIndex = shaftIdx * defaultPreset.treadles + treadleIdx;
			if (defaultTieupRaw[flatIndex] == 1)
			{
				defaultPreset.tieup[shaftIdx][treadleIdx] = true;
			}
		}
	}
	defaultPreset.treadling.resize(defaultPreset.treadles);
	for (auto& row : defaultPreset.treadling)
		row.resize(defaultPreset.weftPicks, false);
	const int defaultTreadlingRaw[] = {0, 1, 2, 3, 0, 1, 2, 3};
	for (int pickIdx = 0; pickIdx < defaultPreset.weftPicks; ++pickIdx)
	{
		int treadleId = defaultTreadlingRaw[pickIdx];
		if (treadleId >= 0 && treadleId < defaultPreset.treadles)
		{
			defaultPreset.treadling[treadleId][pickIdx] = true;
		}
	}
	m_presets.append(defaultPreset);


	DraftData plainWeavePreset;
	plainWeavePreset.name = "Plain Weave";
	plainWeavePreset.warpEnds = 8;
	plainWeavePreset.shafts = 2;
	plainWeavePreset.treadles = 2;
	plainWeavePreset.weftPicks = 8;
	plainWeavePreset.threading.resize(plainWeavePreset.shafts);
	for (auto& row : plainWeavePreset.threading)
		row.resize(plainWeavePreset.warpEnds, false);
	for (int i = 0; i < plainWeavePreset.warpEnds; ++i)
		plainWeavePreset.threading[i % 2][i] = true;
	plainWeavePreset.tieup.resize(plainWeavePreset.shafts);
	for (auto& row : plainWeavePreset.tieup)
		row.resize(plainWeavePreset.treadles, false);
	plainWeavePreset.tieup[0][0] = true;
	plainWeavePreset.tieup[1][0] = false;
	plainWeavePreset.tieup[0][1] = false;
	plainWeavePreset.tieup[1][1] = true;
	plainWeavePreset.treadling.resize(plainWeavePreset.treadles);
	for (auto& row : plainWeavePreset.treadling)
		row.resize(plainWeavePreset.weftPicks, false);
	for (int i = 0; i < plainWeavePreset.weftPicks; ++i)
		plainWeavePreset.treadling[i % 2][i] = true;
	m_presets.append(plainWeavePreset);

	DraftData twillPreset;
	twillPreset.name = "2/2 Twill";
	twillPreset.warpEnds = 8;
	twillPreset.shafts = 4;
	twillPreset.treadles = 4;
	twillPreset.weftPicks = 8;
	twillPreset.threading.resize(twillPreset.shafts);
	for (auto& row : twillPreset.threading)
		row.resize(twillPreset.warpEnds, false);
	for (int i = 0; i < twillPreset.warpEnds; ++i)
		twillPreset.threading[i % 4][i] = true;
	twillPreset.tieup.resize(twillPreset.shafts);
	for (auto& row : twillPreset.tieup)
		row.resize(twillPreset.treadles, false);
	twillPreset.tieup[0][0] = true;
	twillPreset.tieup[1][0] = true;
	twillPreset.tieup[2][0] = false;
	twillPreset.tieup[3][0] = false;
	twillPreset.tieup[0][1] = false;
	twillPreset.tieup[1][1] = true;
	twillPreset.tieup[2][1] = true;
	twillPreset.tieup[3][1] = false;
	twillPreset.tieup[0][2] = false;
	twillPreset.tieup[1][2] = false;
	twillPreset.tieup[2][2] = true;
	twillPreset.tieup[3][2] = true;
	twillPreset.tieup[0][3] = true;
	twillPreset.tieup[1][3] = false;
	twillPreset.tieup[2][3] = false;
	twillPreset.tieup[3][3] = true;
	twillPreset.treadling.resize(twillPreset.treadles);
	for (auto& row : twillPreset.treadling)
		row.resize(twillPreset.weftPicks, false);
	for (int i = 0; i < twillPreset.weftPicks; ++i)
		twillPreset.treadling[i % 4][i] = true;
	m_presets.append(twillPreset);

	m_presetMenu->clear();
	for (int i = 0; i < m_presets.size(); ++i)
	{
		QAction* presetAction = new QAction(m_presets[i].name, this);
		presetAction->setData(i);
		connect(presetAction, &QAction::triggered, this,
				&MainWindow::onLoadPresetTriggered);
		m_presetMenu->addAction(presetAction);
	}
}

void MainWindow::onSaveDraft()
{
	QString fileName = QFileDialog::getSaveFileName(
		this, "Save Weaving Draft", "",
		"Jacquard Draft Files (*.jqd);;All Files (*)");

	if (fileName.isEmpty())
	{
		return;
	}

	if (!fileName.endsWith(".jqd"))
	{
		fileName += ".jqd";
	}

	DraftData currentData = getCurrentDraftData();
	QJsonObject rootObj;

	rootObj["version"] = currentData.version;

	QJsonObject dimObj;
	dimObj["warpEnds"] = currentData.warpEnds;
	dimObj["shafts"] = currentData.shafts;
	dimObj["treadles"] = currentData.treadles;
	dimObj["weftPicks"] = currentData.weftPicks;
	rootObj["dimensions"] = dimObj;

	rootObj["threading"] = gridToJson(currentData.threading);
	rootObj["tieup"] = gridToJson(currentData.tieup);
	rootObj["treadling"] = gridToJson(currentData.treadling);

	QJsonDocument saveDoc(rootObj);
	QFile saveFile(fileName);
	if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Save Error",
							 "Could not open file for writing:\n" +
								 saveFile.errorString());
		return;
	}

	saveFile.write(saveDoc.toJson());
	saveFile.close();

	qDebug() << "Draft saved successfully to" << fileName;
	statusBar()->showMessage(
		QString("Draft saved to %1").arg(QFileInfo(fileName).fileName()), 3000);
}

void MainWindow::onLoadDraft()
{
	QString fileName = QFileDialog::getOpenFileName(
		this, "Load Weaving Draft", "",
		"Jacquard Draft Files (*.jqd);;All Files (*)");

	if (fileName.isEmpty())
	{
		return;
	}

	QFile loadFile(fileName);
	if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Load Error",
							 "Could not open file for reading:\n" +
								 loadFile.errorString());
		return;
	}

	QByteArray saveData = loadFile.readAll();
	loadFile.close();

	QJsonParseError parseError;
	QJsonDocument loadDoc = QJsonDocument::fromJson(saveData, &parseError);

	if (parseError.error != QJsonParseError::NoError)
	{
		QMessageBox::warning(this, "Load Error",
							 "Failed to parse JSON file:\n" +
								 parseError.errorString());
		return;
	}

	if (!loadDoc.isObject())
	{
		QMessageBox::warning(
			this, "Load Error",
			"Invalid draft format: JSON root is not an object.");
		return;
	}

	QJsonObject rootObj = loadDoc.object();
	DraftData loadedData;

	if (rootObj.contains("version") && rootObj["version"].isDouble())
	{
		loadedData.version = rootObj["version"].toInt();
	}

	if (!rootObj.contains("dimensions") || !rootObj["dimensions"].isObject())
	{
		QMessageBox::warning(
			this, "Load Error",
			"Invalid draft format: Missing or invalid 'dimensions' object.");
		return;
	}
	QJsonObject dimObj = rootObj["dimensions"].toObject();
	QStringList missingDims;
	if (!dimObj.contains("warpEnds") || !dimObj["warpEnds"].isDouble())
		missingDims << "warpEnds";
	if (!dimObj.contains("shafts") || !dimObj["shafts"].isDouble())
		missingDims << "shafts";
	if (!dimObj.contains("treadles") || !dimObj["treadles"].isDouble())
		missingDims << "treadles";
	if (!dimObj.contains("weftPicks") || !dimObj["weftPicks"].isDouble())
		missingDims << "weftPicks";

	if (!missingDims.isEmpty())
	{
		QMessageBox::warning(this, "Load Error",
							 "Invalid draft format: Missing dimensions: " +
								 missingDims.join(", "));
		return;
	}
	loadedData.warpEnds = dimObj["warpEnds"].toInt();
	loadedData.shafts = dimObj["shafts"].toInt();
	loadedData.treadles = dimObj["treadles"].toInt();
	loadedData.weftPicks = dimObj["weftPicks"].toInt();

	if (!rootObj.contains("threading") || !rootObj["threading"].isArray())
	{
		QMessageBox::warning(
			this, "Load Error",
			"Invalid draft format: Missing or invalid 'threading' array.");
		return;
	}
	loadedData.threading = jsonToGrid(rootObj["threading"].toArray(),
									  loadedData.shafts, loadedData.warpEnds);

	if (!rootObj.contains("tieup") || !rootObj["tieup"].isArray())
	{
		QMessageBox::warning(
			this, "Load Error",
			"Invalid draft format: Missing or invalid 'tieup' array.");
		return;
	}
	loadedData.tieup = jsonToGrid(rootObj["tieup"].toArray(), loadedData.shafts,
								  loadedData.treadles);


	if (!rootObj.contains("treadling") || !rootObj["treadling"].isArray())
	{
		QMessageBox::warning(
			this, "Load Error",
			"Invalid draft format: Missing or invalid 'treadling' array.");
		return;
	}
	loadedData.treadling =
		jsonToGrid(rootObj["treadling"].toArray(), loadedData.treadles,
				   loadedData.weftPicks);

	applyDraftData(loadedData);

	qDebug() << "Draft loaded successfully from" << fileName;
	statusBar()->showMessage(
		QString("Draft loaded: %1").arg(QFileInfo(fileName).fileName()), 3000);
}


void MainWindow::onLoadPresetTriggered()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action)
		return;

	bool ok;
	int presetIndex = action->data().toInt(&ok);

	if (ok && presetIndex >= 0 && presetIndex < m_presets.size())
	{
		applyDraftData(m_presets[presetIndex]);
		qDebug() << "Loaded preset:" << m_presets[presetIndex].name;
		statusBar()->showMessage(
			QString("Loaded preset: %1").arg(m_presets[presetIndex].name),
			3000);
	}
	else
	{
		qWarning() << "Failed to load preset - invalid index or action data.";
		statusBar()->showMessage("Error: Failed to load preset.", 3000);
	}
}

void MainWindow::on_actionQuit_triggered()
{
	QApplication::exit();
}

void MainWindow::onDimensionsChanged()
{
	int warpEnds = ui->spinWarpEnds->value();
	int shafts = ui->spinShafts->value();
	int treadles = ui->spinTreadles->value();
	int weftPicks = ui->spinWeftPicks->value();

	ui->threadingGrid->setGridSize(shafts, warpEnds);
	ui->tieupGrid->setGridSize(shafts, treadles);
	ui->treadlingGrid->setGridSize(treadles, weftPicks);

	qDebug() << "Dimensions Changed - Threading:" << shafts << "x" << warpEnds
			 << "Tieup:" << shafts << "x" << treadles
			 << "Treadling:" << treadles << "x" << weftPicks;
	statusBar()->showMessage(
		"Loom dimensions changed. Click 'Update Preview' to apply.", 4000);
}

void MainWindow::onMyGLInitialized()
{
	qDebug() << "MainWindow received MyGL initialization signal.";
	statusBar()->showMessage("OpenGL Initialized. Updating preview...", 2000);
	updateShaderData();
}

bool MainWindow::convertDraftGridToMapping(
	const QVector<QVector<bool>>& gridData, DraftType type,
	QVector<int>& mappingArray, int& errorCount)
{
	errorCount = 0;
	if (gridData.isEmpty() || gridData[0].isEmpty())
	{
		mappingArray.clear();
		return true;
	}

	int rows = gridData.size();
	int cols = gridData[0].size();
	mappingArray.resize(cols);
	mappingArray.fill(-1);

	for (int c = 0; c < cols; ++c)
	{
		int selectedRow = -1;
		int selectionCount = 0;
		for (int r = 0; r < rows; ++r)
		{
			if (gridData[r][c])
			{
				selectedRow = r;
				selectionCount++;
			}
		}

		if (selectionCount == 1)
		{
			mappingArray[c] = selectedRow;
		}
		else
		{
			mappingArray[c] = 0;
			errorCount++;
		}
	}
	return errorCount == 0;
}


void MainWindow::updateShaderData()
{
	QVector<QVector<bool>> threadingGridData = ui->threadingGrid->getData();
	QVector<QVector<bool>> tieupGridData = ui->tieupGrid->getData();
	QVector<QVector<bool>> treadlingGridData = ui->treadlingGrid->getData();

	int warpEnds = ui->spinWarpEnds->value();
	int shafts = ui->spinShafts->value();
	int treadles = ui->spinTreadles->value();
	int weftPicks = ui->spinWeftPicks->value();

	QVector<int> flatThreading;
	int threadingErrors = 0;
	bool threadingValid =
		convertDraftGridToMapping(threadingGridData, DraftType::Threading,
								  flatThreading, threadingErrors);
	if (threadingErrors > 0)
	{
		qDebug() << "Warning: Found" << threadingErrors
				 << "warp ends with invalid shaft selections (defaulting to "
					"shaft 0).";
	}

	QVector<int> flatTieup;
	if (!tieupGridData.isEmpty())
	{
		flatTieup.reserve(shafts * treadles);
		for (int r = 0; r < shafts; ++r)
		{
			for (int c = 0; c < treadles; ++c)
			{
				flatTieup.append((r < tieupGridData.size() &&
								  c < tieupGridData[r].size() &&
								  tieupGridData[r][c])
									 ? 1
									 : 0);
			}
		}
	}

	QVector<int> flatTreadling;
	int treadlingErrors = 0;
	bool treadlingValid =
		convertDraftGridToMapping(treadlingGridData, DraftType::Treadling,
								  flatTreadling, treadlingErrors);
	if (treadlingErrors > 0)
	{
		qDebug() << "Warning: Found" << treadlingErrors
				 << "weft picks with invalid treadle selections (defaulting to "
					"treadle 0).";
	}

	glm::vec2 patternScale(m_patternScale.x(), m_patternScale.y());
	glm::vec3 warpColor(m_warpColor.redF(), m_warpColor.greenF(),
						m_warpColor.blueF());
	glm::vec3 weftColor(m_weftColor.redF(), m_weftColor.greenF(),
						m_weftColor.blueF());
	int warpMaterial = m_warpMaterialId;
	int weftMaterial = m_weftMaterialId;

	qDebug() << "Updating shader data...";
	qDebug() << "Dimensions (W,S,T,P):" << warpEnds << shafts << treadles
			 << weftPicks;
	qDebug() << "Scale:" << patternScale.x << patternScale.y;
	qDebug() << "Threading Size:" << flatThreading.size()
			 << "(Errors:" << threadingErrors << ")";
	qDebug() << "Tie-up Size:" << flatTieup.size();
	qDebug() << "Treadling Size:" << flatTreadling.size()
			 << "(Errors:" << treadlingErrors << ")";
	qDebug() << "Warp Mat/Color:" << warpMaterial << "/" << warpColor.r
			 << warpColor.g << warpColor.b;
	qDebug() << "Weft Mat/Color:" << weftMaterial << "/" << weftColor.r
			 << weftColor.g << weftColor.b;

	// qDebug() << "Flat Threading:" << flatThreading;
	// qDebug() << "Flat Tieup:" << flatTieup;
	// qDebug() << "Flat Treadling:" << flatTreadling;


	ui->mygl->slot_updateLoomState(warpEnds, shafts, treadles, weftPicks,
								   flatThreading, flatTieup, flatTreadling,
								   patternScale, warpMaterial, warpColor,
								   weftMaterial, weftColor);
}

void MainWindow::loadDefaultPattern()
{
	const int defaultWarpEnds = 8;
	const int defaultShafts = 4;
	const int defaultTreadles = 6;
	const int defaultWeftPicks = 8;

	ui->spinWarpEnds->setValue(defaultWarpEnds);
	ui->spinShafts->setValue(defaultShafts);
	ui->spinTreadles->setValue(defaultTreadles);
	ui->spinWeftPicks->setValue(defaultWeftPicks);

	const int defaultThreadingRaw[defaultWarpEnds] = {0, 1, 2, 3, 0, 1, 2, 3};
	const int defaultTieupRaw[defaultShafts * defaultTreadles] = {
		//
		1, 0, 0, 0, 1, 0, //
		0, 1, 0, 0, 0, 1, //
		0, 0, 1, 0, 1, 0, //
		0, 0, 0, 1, 0, 1  //
	};
	const int defaultTreadlingRaw[defaultWeftPicks] = {0, 1, 2, 3, 0, 1, 2, 3};

	QVector<QVector<bool>> defaultThreadingData(
		defaultShafts, QVector<bool>(defaultWarpEnds, false));
	for (int warpIdx = 0; warpIdx < defaultWarpEnds; ++warpIdx)
	{
		int shaftId = defaultThreadingRaw[warpIdx];
		if (shaftId >= 0 && shaftId < defaultShafts)
		{
			defaultThreadingData[shaftId][warpIdx] = true;
		}
	}

	QVector<QVector<bool>> defaultTieupData(
		defaultShafts, QVector<bool>(defaultTreadles, false));
	for (int shaftIdx = 0; shaftIdx < defaultShafts; ++shaftIdx)
	{
		for (int treadleIdx = 0; treadleIdx < defaultTreadles; ++treadleIdx)
		{
			int flatIndex = shaftIdx * defaultTreadles + treadleIdx;
			if (defaultTieupRaw[flatIndex] == 1)
			{
				defaultTieupData[shaftIdx][treadleIdx] = true;
			}
		}
	}

	QVector<QVector<bool>> defaultTreadlingData(
		defaultTreadles, QVector<bool>(defaultWeftPicks, false));
	for (int pickIdx = 0; pickIdx < defaultWeftPicks; ++pickIdx)
	{
		int treadleId = defaultTreadlingRaw[pickIdx];
		if (treadleId >= 0 && treadleId < defaultTreadles)
		{
			defaultTreadlingData[treadleId][pickIdx] = true;
		}
	}

	ui->threadingGrid->setData(defaultThreadingData);
	ui->tieupGrid->setData(defaultTieupData);
	ui->treadlingGrid->setData(defaultTreadlingData);
}

void MainWindow::on_settingsButton_clicked()
{
	LoomSettingsDialog dialog(this);

	dialog.setPatternScale(m_patternScale);
	dialog.setWarpColor(m_warpColor);
	dialog.setWeftColor(m_weftColor);
	dialog.setWarpMaterial(m_warpMaterialId);
	dialog.setWeftMaterial(m_weftMaterialId);

	if (dialog.exec() == QDialog::Accepted)
	{
		m_patternScale = dialog.getPatternScale();
		m_warpColor = dialog.getWarpColor();
		m_weftColor = dialog.getWeftColor();
		m_warpMaterialId = dialog.getWarpMaterial();
		m_weftMaterialId = dialog.getWeftMaterial();
		qDebug() << "Settings updated via dialog.";
	}
	else
	{
		qDebug() << "Settings dialog cancelled.";
		statusBar()->showMessage("Thread settings cancelled.", 2000);
	}
}

void MainWindow::on_updateShaderButton_clicked()
{
	qDebug() << "Update Shader button clicked.";
	updateShaderData();
}

DraftData MainWindow::getCurrentDraftData() const
{
	DraftData data;
	data.warpEnds = ui->spinWarpEnds->value();
	data.shafts = ui->spinShafts->value();
	data.treadles = ui->spinTreadles->value();
	data.weftPicks = ui->spinWeftPicks->value();
	data.threading = ui->threadingGrid->getData();
	data.tieup = ui->tieupGrid->getData();
	data.treadling = ui->treadlingGrid->getData();
	return data;
}

void MainWindow::applyDraftData(const DraftData& data)
{
	bool blockedW = ui->spinWarpEnds->blockSignals(true);
	bool blockedS = ui->spinShafts->blockSignals(true);
	bool blockedT = ui->spinTreadles->blockSignals(true);
	bool blockedP = ui->spinWeftPicks->blockSignals(true);

	ui->spinWarpEnds->setValue(data.warpEnds);
	ui->spinShafts->setValue(data.shafts);
	ui->spinTreadles->setValue(data.treadles);
	ui->spinWeftPicks->setValue(data.weftPicks);

	ui->spinWarpEnds->blockSignals(blockedW);
	ui->spinShafts->blockSignals(blockedS);
	ui->spinTreadles->blockSignals(blockedT);
	ui->spinWeftPicks->blockSignals(blockedP);

	onDimensionsChanged();

	ui->threadingGrid->setData(data.threading);
	ui->tieupGrid->setData(data.tieup);
	ui->treadlingGrid->setData(data.treadling);

	updateShaderData();
}


QJsonArray MainWindow::gridToJson(const QVector<QVector<bool>>& gridData)
{
	QJsonArray rowsArray;
	for (const auto& row : gridData)
	{
		QJsonArray colsArray;
		for (bool cell : row)
		{
			colsArray.append(cell);
		}
		rowsArray.append(colsArray);
	}
	return rowsArray;
}

QVector<QVector<bool>> MainWindow::jsonToGrid(const QJsonArray& jsonArray,
											  int expectedRows,
											  int expectedCols)
{
	QVector<QVector<bool>> gridData(expectedRows,
									QVector<bool>(expectedCols, false));

	int rowsToRead = qMin(expectedRows, jsonArray.size());
	for (int r = 0; r < rowsToRead; ++r)
	{
		if (!jsonArray[r].isArray())
		{
			qWarning() << "JSON grid load warning: Row" << r
					   << "is not an array.";
			continue;
		}
		QJsonArray colsArray = jsonArray[r].toArray();
		int colsToRead = qMin(expectedCols, colsArray.size());
		for (int c = 0; c < colsToRead; ++c)
		{
			if (!colsArray[c].isBool())
			{
				qWarning() << "JSON grid load warning: Cell (" << r << "," << c
						   << ") is not a boolean.";
				gridData[r][c] = false;
			}
			else
			{
				gridData[r][c] = colsArray[c].toBool();
			}
		}
	}
	return gridData;
}
