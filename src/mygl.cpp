#include "mygl.h"
#include <glm_includes.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <iostream>

#include "texture.h"
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

#define MAX_WARP_THREADS 256
#define MAX_SHAFTS 16
#define MAX_TREADLES 16
#define MAX_WEFT_PICKS 256

#define MAX_THREADING_SIZE MAX_WARP_THREADS
#define MAX_TIEUP_SIZE (MAX_SHAFTS * MAX_TREADLES)
#define MAX_TREADLING_SIZE MAX_WEFT_PICKS

const int MAT_COTTON = 0;
const int MAT_SILK = 1;
const int MAT_WOOL_YARN = 2;
const int MAT_LINEN = 3;
const int MAT_METALLIC_GOLD = 4;
const int MAT_POLYESTER = 5;

MyGL::MyGL(QWidget* parent)
	: OpenGLContext(parent), curr_time(QDateTime::currentMSecsSinceEpoch()),
	  elapsed_time(0.), m_timer(), m_geomSquare(this), m_textureAlbedo(this),
	  m_textureMetallic(this), m_textureNormals(this), m_textureRoughness(this),
	  m_textureAO(this), m_textureDisplacement(this), m_geomCube(this),
	  m_hdrEnvMap(this), m_environmentCubemapFB(this, 1024, 1024, 1.),
	  m_diffuseIrradianceFB(this, 32, 32, 1.),
	  m_glossyIrradianceFB(this, 512, 512, 1.), m_brdfLookupTexture(this),
	  m_progPBR(this), m_progCubemapConversion(this),
	  m_progCubemapDiffuseConvolution(this),
	  m_progCubemapGlossyConvolution(this), m_progEnvMap(this), m_glCamera(),
	  m_mousePosPrev(), m_albedo(0.5f, 0., 0.), m_cubemapsNotGenerated(true)
{
	setFocusPolicy(Qt::StrongFocus);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
}

MyGL::~MyGL()
{
	makeCurrent();
	glDeleteVertexArrays(1, &vao);
	m_geomSquare.destroy();
}

QString qTextFileRead(const char* fileName)
{
	QString text;
	QFile file(fileName);
	if (file.open(QFile::ReadOnly))
	{
		QTextStream in(&file);
		text = in.readAll();
		text.append('\0');
	}
	return text;
}

void MyGL::initializeGL()
{
	initializeOpenGLFunctions();
	debugContextVersion();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glClearColor(0.5, 0.5, 0.5, 1);

	printGLErrorLog();

	glGenVertexArrays(1, &vao);

	m_geomSquare.create();
	QString path = getCurrentPath();
	path.append("/objs/");
	m_geomCube.create();

	//    m_progPBR.create(":/glsl/pbr.vert.glsl", ":/glsl/pbr.frag.glsl");
	//    m_progPBR.create(":/glsl/passthrough.vert.glsl",
	//    ":/glsl/sdf.frag.glsl");

	QString vertASCII = qTextFileRead(":/glsl/passthrough.vert.glsl");
	QString fragASCII = writeFullShaderFile();
	m_progPBR.create(vertASCII, fragASCII);

	m_progCubemapConversion.create(":/glsl/cubemap.vert.glsl",
								   ":/glsl/cubemap_uv_conversion.frag.glsl");
	m_progCubemapDiffuseConvolution.create(
		":/glsl/cubemap.vert.glsl", ":/glsl/diffuseConvolution.frag.glsl");
	m_progCubemapGlossyConvolution.create(":/glsl/cubemap.vert.glsl",
										  ":/glsl/glossyConvolution.frag.glsl");
	m_progEnvMap.create(":/glsl/envMap.vert.glsl", ":/glsl/envMap.frag.glsl");
	setupShaderHandles();

	path = getCurrentPath();
	path.append("/environment_maps/"
				"interior_atelier_soft_daylight.hdr");
	m_hdrEnvMap.create(path.toStdString().c_str(), false);

	m_environmentCubemapFB.create(true);
	m_diffuseIrradianceFB.create();
	m_glossyIrradianceFB.create(true);
	m_brdfLookupTexture.create(":/textures/brdfLUT.png", false);

	glBindVertexArray(vao);

	initPBRunifs();

	m_timer.start(16);

	emit glInitialized();
}

void MyGL::resizeGL(int w, int h)
{
	m_glCamera = Camera(w, h);
	glm::mat4 viewproj = m_glCamera.getViewProj();

	m_progPBR.setUnifMat4("u_ViewProj", viewproj);
	m_progPBR.setUnifVec2("u_ScreenDims", glm::vec2(w, h));

	printGLErrorLog();
}

void MyGL::paintGL()
{
	if (m_cubemapsNotGenerated)
	{
		m_cubemapsNotGenerated = false;
		renderCubeMapToTexture();
		printGLErrorLog();

		m_environmentCubemapFB.generateMipMaps();
		printGLErrorLog();

		renderConvolvedDiffuseCubeMapToTexture();
		printGLErrorLog();

		renderConvolvedGlossyCubeMapToTexture();
		printGLErrorLog();
	}

	glViewport(0, 0, this->width() * this->devicePixelRatio(),
			   this->height() * this->devicePixelRatio());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderEnvironmentMap();

	m_progPBR.setUnifMat4("u_ViewProj", m_glCamera.getViewProj());
	m_progPBR.setUnifVec3("u_CamPos", m_glCamera.eye);
	m_progPBR.setUnifVec3("u_Forward", m_glCamera.look);
	m_progPBR.setUnifVec3("u_Right", m_glCamera.right);
	m_progPBR.setUnifVec3("u_Up", m_glCamera.up);

	m_progPBR.setUnifMat4("u_Model", glm::mat4(1.));
	m_progPBR.setUnifMat4("u_ModelInvTr", glm::mat4(1.));

	m_diffuseIrradianceFB.bindToTextureSlot(DIFFUSE_IRRADIANCE_CUBE_TEX_SLOT);
	m_progPBR.setUnifInt("u_DiffuseIrradianceMap",
						 DIFFUSE_IRRADIANCE_CUBE_TEX_SLOT);

	m_glossyIrradianceFB.bindToTextureSlot(GLOSSY_IRRADIANCE_CUBE_TEX_SLOT);
	m_progPBR.setUnifInt("u_GlossyIrradianceMap",
						 GLOSSY_IRRADIANCE_CUBE_TEX_SLOT);

	m_brdfLookupTexture.bind(BRDF_LUT_TEX_SLOT);
	printGLErrorLog();
	m_progPBR.setUnifInt("u_BRDFLookupTexture", BRDF_LUT_TEX_SLOT);

	if (m_textureAlbedo.m_isCreated)
	{
		m_textureAlbedo.bind(ALBEDO_TEX_SLOT);
		m_progPBR.setUnifInt("u_AlbedoMap", ALBEDO_TEX_SLOT);
		m_progPBR.setUnifInt("u_UseAlbedoMap", 1);
	}
	else
	{
		m_progPBR.setUnifInt("u_UseAlbedoMap", 0);
	}
	if (m_textureMetallic.m_isCreated)
	{
		m_textureMetallic.bind(METALLIC_TEX_SLOT);
		m_progPBR.setUnifInt("u_MetallicMap", METALLIC_TEX_SLOT);
		m_progPBR.setUnifInt("u_UseMetallicMap", 1);
	}
	else
	{
		m_progPBR.setUnifInt("u_UseMetallicMap", 0);
	}
	if (m_textureRoughness.m_isCreated)
	{
		m_textureRoughness.bind(ROUGHNESS_TEX_SLOT);
		m_progPBR.setUnifInt("u_RoughnessMap", ROUGHNESS_TEX_SLOT);
		m_progPBR.setUnifInt("u_UseRoughnessMap", 1);
	}
	else
	{
		m_progPBR.setUnifInt("u_UseRoughnessMap", 0);
	}
	if (m_textureAO.m_isCreated)
	{
		m_textureAO.bind(AO_TEX_SLOT);
		m_progPBR.setUnifInt("u_AOMap", AO_TEX_SLOT);
		m_progPBR.setUnifInt("u_UseAOMap", 1);
	}
	else
	{
		m_progPBR.setUnifInt("u_UseAOMap", 0);
	}
	if (m_textureNormals.m_isCreated)
	{
		m_textureNormals.bind(NORMALS_TEX_SLOT);
		m_progPBR.setUnifInt("u_NormalMap", NORMALS_TEX_SLOT);
		m_progPBR.setUnifInt("u_UseNormalMap", 1);
	}
	else
	{
		m_progPBR.setUnifInt("u_UseNormalMap", 0);
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_progPBR.draw(m_geomSquare);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void printMat(const glm::mat4& m)
{
	std::cout << glm::to_string(m[0]) << std::endl;
	std::cout << glm::to_string(m[1]) << std::endl;
	std::cout << glm::to_string(m[2]) << std::endl;
	std::cout << glm::to_string(m[3]) << std::endl;
}

void MyGL::renderCubeMapToTexture()
{
	m_progCubemapConversion.useMe();

	m_progCubemapConversion.setUnifInt("u_EquirectangularMap",
									   ENV_MAP_FLAT_TEX_SLOT);

	m_hdrEnvMap.bind(ENV_MAP_FLAT_TEX_SLOT);
	glViewport(0, 0, m_environmentCubemapFB.width(),
			   m_environmentCubemapFB.height());
	m_environmentCubemapFB.bindFrameBuffer();

	for (int i = 0; i < 6; ++i)
	{
		m_progCubemapConversion.setUnifMat4("u_ViewProj", views[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							   m_environmentCubemapFB.getCubemapHandle(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_progCubemapConversion.draw(m_geomCube);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
}

void MyGL::renderConvolvedDiffuseCubeMapToTexture()
{
	m_progCubemapDiffuseConvolution.useMe();
	m_progCubemapDiffuseConvolution.setUnifInt("u_EnvironmentMap",
											   ENV_MAP_CUBE_TEX_SLOT);
	m_environmentCubemapFB.bindToTextureSlot(ENV_MAP_CUBE_TEX_SLOT);
	glViewport(0, 0, m_diffuseIrradianceFB.width(),
			   m_diffuseIrradianceFB.height());
	m_diffuseIrradianceFB.bindFrameBuffer();

	for (int i = 0; i < 6; ++i)
	{
		m_progCubemapDiffuseConvolution.setUnifMat4("u_ViewProj", views[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							   m_diffuseIrradianceFB.getCubemapHandle(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_progCubemapDiffuseConvolution.draw(m_geomCube);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
}

void MyGL::renderConvolvedGlossyCubeMapToTexture()
{
	m_progCubemapGlossyConvolution.useMe();
	m_progCubemapGlossyConvolution.setUnifInt("u_EnvironmentMap",
											  ENV_MAP_CUBE_TEX_SLOT);
	m_environmentCubemapFB.bindToTextureSlot(ENV_MAP_CUBE_TEX_SLOT);
	glViewport(0, 0, m_glossyIrradianceFB.width(),
			   m_glossyIrradianceFB.height());
	m_glossyIrradianceFB.bindFrameBuffer();

	const unsigned int maxMipLevels = 5;
	for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel)
	{
		unsigned int mipWidth = static_cast<unsigned int>(
			m_glossyIrradianceFB.width() * std::pow(0.5, mipLevel));
		unsigned int mipHeight = static_cast<unsigned int>(
			m_glossyIrradianceFB.height() * std::pow(0.5, mipLevel));
		m_glossyIrradianceFB.bindRenderBuffer(mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness =
			static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1);
		m_progCubemapGlossyConvolution.setUnifFloat("u_Roughness", roughness);

		for (int i = 0; i < 6; ++i)
		{
			m_progCubemapGlossyConvolution.setUnifMat4("u_ViewProj", views[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
								   m_glossyIrradianceFB.getCubemapHandle(),
								   mipLevel);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_progCubemapGlossyConvolution.draw(m_geomCube);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
}

void MyGL::renderEnvironmentMap()
{
	m_environmentCubemapFB.bindToTextureSlot(ENV_MAP_CUBE_TEX_SLOT);
	m_progEnvMap.setUnifInt("u_EnvironmentMap", ENV_MAP_CUBE_TEX_SLOT);
	m_progEnvMap.setUnifMat4("u_ViewProj", m_glCamera.getViewProj_OrientOnly());
	m_progEnvMap.draw(m_geomCube);
}

void MyGL::setupShaderHandles()
{
	m_progPBR.addUniform("u_Time");
	m_progPBR.addUniform("u_Model");
	m_progPBR.addUniform("u_ModelInvTr");
	m_progPBR.addUniform("u_ViewProj");
	m_progPBR.addUniform("u_CamPos");
	m_progPBR.addUniform("u_Forward");
	m_progPBR.addUniform("u_Right");
	m_progPBR.addUniform("u_Up");
	m_progPBR.addUniform("u_ScreenDims");
	m_progPBR.addUniform("u_Albedo");
	m_progPBR.addUniform("u_Metallic");
	m_progPBR.addUniform("u_Roughness");
	m_progPBR.addUniform("u_AmbientOcclusion");
	m_progPBR.addUniform("u_AlbedoMap");
	m_progPBR.addUniform("u_MetallicMap");
	m_progPBR.addUniform("u_RoughnessMap");
	m_progPBR.addUniform("u_AOMap");
	m_progPBR.addUniform("u_NormalMap");
	m_progPBR.addUniform("u_UseAlbedoMap");
	m_progPBR.addUniform("u_UseMetallicMap");
	m_progPBR.addUniform("u_UseRoughnessMap");
	m_progPBR.addUniform("u_UseAOMap");
	m_progPBR.addUniform("u_UseNormalMap");
	m_progPBR.addUniform("u_DiffuseIrradianceMap");
	m_progPBR.addUniform("u_GlossyIrradianceMap");
	m_progPBR.addUniform("u_BRDFLookupTexture");
	m_progPBR.addUniform("u_WarpThreads");
	m_progPBR.addUniform("u_Shafts");
	m_progPBR.addUniform("u_Treadles");
	m_progPBR.addUniform("u_WeftPicks");
	m_progPBR.addUniform("u_Threading");
	m_progPBR.addUniform("u_Tieup");
	m_progPBR.addUniform("u_Treadling");
	m_progPBR.addUniform("u_WarpMaterial");
	m_progPBR.addUniform("u_WeftMaterial");
	m_progPBR.addUniform("u_WarpColorBase");
	m_progPBR.addUniform("u_WeftColorBase");
	m_progPBR.addUniform("u_PatternScale");

	m_progCubemapConversion.addUniform("u_EquirectangularMap");
	m_progCubemapConversion.addUniform("u_ViewProj");

	m_progEnvMap.addUniform("u_EnvironmentMap");
	m_progEnvMap.addUniform("u_ViewProj");

	m_progCubemapDiffuseConvolution.addUniform("u_EnvironmentMap");
	m_progCubemapDiffuseConvolution.addUniform("u_ViewProj");

	m_progCubemapGlossyConvolution.addUniform("u_EnvironmentMap");
	m_progCubemapGlossyConvolution.addUniform("u_Roughness");
	m_progCubemapGlossyConvolution.addUniform("u_ViewProj");
}

void MyGL::initPBRunifs()
{
	m_progPBR.setUnifVec3("u_Albedo", m_albedo);
	m_progPBR.setUnifFloat("u_AmbientOcclusion", 1.);
	m_progPBR.setUnifInt("u_WarpMaterial", MAT_COTTON);
	m_progPBR.setUnifInt("u_WeftMaterial", MAT_COTTON);
	m_progPBR.setUnifVec3("u_WarpColorBase", glm::vec3(0.8f, 0.8f, 0.8f));
	m_progPBR.setUnifVec3("u_WeftColorBase", glm::vec3(0.3f, 0.3f, 0.3f));
	m_progPBR.setUnifVec2("u_PatternScale", glm::vec2(0.2f, 0.2f));
}

void MyGL::slot_setRed(int r)
{
	m_albedo.r = r / 100.;
	m_progPBR.setUnifVec3("u_Albedo", m_albedo);
	update();
}
void MyGL::slot_setGreen(int g)
{
	m_albedo.g = g / 100.;
	m_progPBR.setUnifVec3("u_Albedo", m_albedo);
	update();
}
void MyGL::slot_setBlue(int b)
{
	m_albedo.b = b / 100.;
	m_progPBR.setUnifVec3("u_Albedo", m_albedo);
	update();
}

void MyGL::slot_setMetallic(int m)
{
	m_progPBR.setUnifFloat("u_Metallic", m / 100.);
	update();
}
void MyGL::slot_setRoughness(int r)
{
	m_progPBR.setUnifFloat("u_Roughness", r / 100.);
	update();
}
void MyGL::slot_setAO(int a)
{
	m_progPBR.setUnifFloat("u_AmbientOcclusion", a / 100.);
	update();
}
void MyGL::slot_setDisplacement(double d)
{
	m_progPBR.setUnifFloat("u_DisplacementMagnitude", d);
	update();
}

void MyGL::slot_loadEnvMap()
{
	QString path = getCurrentPath();
	path.append("/environment_maps/");
	QString filepath = QFileDialog::getOpenFileName(
		0, QString("Load Environment Map"), path, tr("*.hdr"));
	Texture2DHDR tex(this);
	try
	{
		tex.create(filepath.toStdString().c_str(), false);
	}
	catch (std::exception& e)
	{
		std::cout << "Error: Failed to load HDR image" << std::endl;
		return;
	}
	this->m_hdrEnvMap.destroy();
	this->m_hdrEnvMap = tex;
	this->m_cubemapsNotGenerated = true;
	update();
}

QString MyGL::getCurrentPath() const
{
	QString appPath = QCoreApplication::applicationDirPath();
	QDir currentDir(appPath);
	QString potentialPath;

	if (currentDir.dirName() == "MacOS")
	{
		QDir bundleDir = currentDir;
		bundleDir.cdUp();
		potentialPath = bundleDir.absoluteFilePath("Resources");
		QFileInfo bundleCheck(potentialPath);
		if (bundleCheck.exists() && bundleCheck.isDir())
		{
			return potentialPath;
		}
		potentialPath.clear();
	}

	QDir devDir(appPath);
	if (devDir.cdUp())
	{
		return devDir.path();
	}

	qCritical() << "ERROR: Could not find the resources directory!";
	return QDir::currentPath();
}

void MyGL::tick()
{
	int64_t prev_time = curr_time;
	curr_time = QDateTime::currentMSecsSinceEpoch();
	elapsed_time += (curr_time - prev_time) * 0.001f;
	m_progPBR.setUnifFloat("u_Time", elapsed_time);
	update();
}

QString MyGL::writeFullShaderFile() const
{
	std::vector<const char*> fragfile_sections = {":/glsl/sdf.pbr.glsl",
												  ":/glsl/sdf.frag.glsl"};

	QString qFragSource = qTextFileRead(":/glsl/sdf.defines.glsl");
	qFragSource.chop(1);

	std::vector<const char*> scenefile_sections = {":/glsl/sdf.loom.glsl",
												   ":/glsl/sdf.scene.glsl"};

	int lineCount = 0;
	for (auto& c : scenefile_sections)
	{
		QString section = qTextFileRead(c);
		lineCount += section.count("\n");
		qFragSource.chop(1);
		qFragSource = qFragSource + "\n" + section;
	}

	for (auto& c : fragfile_sections)
	{
		QString section = qTextFileRead(c);
		//        std::cout << "First line number in " << c << ": " << lineCount
		//        << std::endl;
		lineCount += section.count("\n");
		qFragSource.chop(1);
		qFragSource = qFragSource + "\n" + section;
	}

	QString path = getCurrentPath();
	QFile entireShader(path + "/glsl/sdf.all.glsl");
	if (entireShader.open(QIODevice::ReadWrite))
	{
		entireShader.resize(0);
		QTextStream stream(&entireShader);
		stream << qFragSource << Qt::endl;
	}
	entireShader.close();
	return qFragSource;
}

void MyGL::slot_updateLoomState(int warp_ends, int shafts, int treadles,
								int weft_picks, const QVector<int>& threading,
								const QVector<int>& tieup,
								const QVector<int>& treadling,
								const glm::vec2& patternScale, int warpMaterial,
								const glm::vec3& warpColor, int weftMaterial,
								const glm::vec3& weftColor)
{
	qDebug() << "MyGL received full loom state update.";

	makeCurrent();
	if (!isValid())
	{
		qDebug()
			<< "Error: OpenGL context invalid in slot_updateDraftDataArrays!";
		return;
	}

	printGLErrorLog();

	if (m_progPBR.prog <= 0)
	{
		qDebug() << "Error: PBR shader program handle is invalid!";
		return;
	}
	m_progPBR.useMe();
	printGLErrorLog();

	m_progPBR.setUnifInt("u_WarpThreads", warp_ends);
	m_progPBR.setUnifInt("u_Shafts", shafts);
	m_progPBR.setUnifInt("u_Treadles", treadles);
	m_progPBR.setUnifInt("u_WeftPicks", weft_picks);
	printGLErrorLog();

	GLint threadLoc = -1, tieupLoc = -1, treadleLoc = -1;
	try
	{
		threadLoc = m_progPBR.m_unifs.at("u_Threading");
		tieupLoc = m_progPBR.m_unifs.at("u_Tieup");
		treadleLoc = m_progPBR.m_unifs.at("u_Treadling");
	}
	catch (const std::out_of_range& oor)
	{
		qDebug() << "Error: Could not find required loom uniform location! "
				 << oor.what();
		return;
	}

	qDebug() << "Uniform Locations - Threading:" << threadLoc
			 << "Tieup:" << tieupLoc << "Treadling:" << treadleLoc;

	if (threadLoc != -1)
	{
		int count = qMin(threading.size(), MAX_THREADING_SIZE);
		if (count > 0)
		{
			glUniform1iv(threadLoc, count, threading.constData());
			printGLErrorLog();
			qDebug() << "Sent u_Threading -" << count << "elements";
		}
		else
		{
			qDebug() << "Skipping u_Threading - count is 0";
		}
		if (threading.size() > MAX_THREADING_SIZE)
		{
			qDebug() << "Warning: Threading data size" << threading.size()
					 << "exceeds shader max" << MAX_THREADING_SIZE;
		}
	}
	else
	{
		qDebug() << "Warning: u_Threading uniform location is invalid (-1)!";
	}

	if (tieupLoc != -1)
	{
		int count = qMin(tieup.size(), MAX_TIEUP_SIZE);
		if (count > 0)
		{
			glUniform1iv(tieupLoc, count, tieup.constData());
			printGLErrorLog();
			qDebug() << "Sent u_Tieup -" << count << "elements";
		}
		else
		{
			qDebug() << "Skipping u_Tieup - count is 0";
		}
		if (tieup.size() > MAX_TIEUP_SIZE)
		{
			qDebug() << "Warning: Tie-up data size" << tieup.size()
					 << "exceeds shader max" << MAX_TIEUP_SIZE;
		}
	}
	else
	{
		qDebug() << "Warning: u_Tieup uniform location is invalid (-1)!";
	}

	if (treadleLoc != -1)
	{
		int count = qMin(treadling.size(), MAX_TREADLING_SIZE);
		if (count > 0)
		{
			glUniform1iv(treadleLoc, count, treadling.constData());
			printGLErrorLog();
			qDebug() << "Sent u_Treadling -" << count << "elements";
		}
		else
		{
			qDebug() << "Skipping u_Treadling - count is 0";
		}
		if (treadling.size() > MAX_TREADLING_SIZE)
		{
			qDebug() << "Warning: Treadling data size" << treadling.size()
					 << "exceeds shader max" << MAX_TREADLING_SIZE;
		}
	}
	else
	{
		qDebug() << "Warning: u_Treadling uniform location is invalid (-1)!";
	}

	m_progPBR.setUnifVec2("u_PatternScale", patternScale);
	m_progPBR.setUnifInt("u_WarpMaterial", warpMaterial);
	m_progPBR.setUnifVec3("u_WarpColorBase", warpColor);
	m_progPBR.setUnifInt("u_WeftMaterial", weftMaterial);
	m_progPBR.setUnifVec3("u_WeftColorBase", weftColor);

	printGLErrorLog();

	qDebug() << "Finished updating loom state uniforms.";
	update();
}
