#include <mainwindow.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSurfaceFormat>
#include <QTextStream>

void debugFormatVersion()
{
	QSurfaceFormat form = QSurfaceFormat::defaultFormat();
	QSurfaceFormat::OpenGLContextProfile prof = form.profile();

	const char* profile = prof == QSurfaceFormat::CoreProfile ? "Core"
						  : prof == QSurfaceFormat::CompatibilityProfile
							  ? "Compatibility"
							  : "None";

	printf("Requested format:\n");
	printf("  Version: %d.%d\n", form.majorVersion(), form.minorVersion());
	printf("  Profile: %s\n", profile);
}

int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);

	QSurfaceFormat format;
	format.setVersion(3, 2);
	format.setOption(QSurfaceFormat::DeprecatedFunctions, false);
	format.setProfile(QSurfaceFormat::CoreProfile);
	// format.setSamples(4);

	if (qgetenv("CIS277_AUTOTESTING") != nullptr)
		format.setSamples(0);

	QSurfaceFormat::setDefaultFormat(format);
	debugFormatVersion();

	QString appPath = QCoreApplication::applicationDirPath();
	QDir currentDir(appPath);
	QString path;

	if (currentDir.dirName() == "MacOS")
	{
		QDir bundleDir = currentDir;
		bundleDir.cdUp();
		path = bundleDir.absoluteFilePath("Resources");
		QFileInfo bundleCheck(path);
		if (!bundleCheck.exists() || !bundleCheck.isDir())
		{
			path.clear();
		}
	}
	if (path == "")
	{
		QDir devDir(appPath);
		if (devDir.cdUp())
		{
			path = devDir.path();
		}
	}
	QFile styleFile(path + "/resources/style.qss");
	if (styleFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream stream(&styleFile);
		a.setStyleSheet(stream.readAll());
		styleFile.close();
		qDebug() << "Stylesheet applied successfully.";
	}
	else
	{
		qWarning() << "Could not load stylesheet.qss";
	}

	MainWindow w;
	w.show();

	return a.exec();
}
