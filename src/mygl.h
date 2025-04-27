#ifndef MYGL_H
#define MYGL_H

#include "camera.h"
#include "framebuffer.h"
#include "scene/cube.h"
#include "scene/squareplane.h"
#include "texture.h"
#include <openglcontext.h>
#include <shaderprogram.h>
#include <utils.h>

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QTimer>

// -X, +X, -Y, +Y, -Z, +Z
const static std::array<glm::mat4, 6> views = {
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec3(0.0f, -1.0f, 0.0f))};


class MyGL : public OpenGLContext
{
	Q_OBJECT
private:
	int64_t curr_time;
	float elapsed_time;
	QTimer m_timer;
	SquarePlane m_geomSquare;
	Texture2D m_textureAlbedo;
	Texture2D m_textureMetallic;
	Texture2D m_textureNormals;
	Texture2D m_textureRoughness;
	Texture2D m_textureAO;
	Texture2D m_textureDisplacement;

	Cube m_geomCube;
	Texture2DHDR m_hdrEnvMap;
	CubeMapFrameBuffer m_environmentCubemapFB;
	CubeMapFrameBuffer m_diffuseIrradianceFB;
	CubeMapFrameBuffer m_glossyIrradianceFB;
	Texture2D m_brdfLookupTexture;

	ShaderProgram m_progPBR;
	ShaderProgram m_progCubemapConversion;
	ShaderProgram m_progCubemapDiffuseConvolution;
	ShaderProgram m_progCubemapGlossyConvolution;
	ShaderProgram m_progEnvMap;

	GLuint vao;
	Camera m_glCamera;
	glm::vec2 m_mousePosPrev;

	glm::vec3 m_albedo;

	bool m_cubemapsNotGenerated;

	QString getCurrentPath() const;


public:
	explicit MyGL(QWidget* parent = nullptr);
	~MyGL();

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void initPBRunifs();

	QString writeFullShaderFile() const;

protected:
	void keyPressEvent(QKeyEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);

	void renderCubeMapToTexture();
	void renderConvolvedDiffuseCubeMapToTexture();
	void renderConvolvedGlossyCubeMapToTexture();
	void renderEnvironmentMap();

	void setupShaderHandles();

public slots:
	void slot_setRed(int);
	void slot_setGreen(int);
	void slot_setBlue(int);
	void slot_setMetallic(int);
	void slot_setRoughness(int);
	void slot_setAO(int);
	void slot_setDisplacement(double);
	void slot_loadEnvMap();
	void slot_updateLoomState(int warp_ends, int shafts, int treadles,
							  int weft_picks, const QVector<int>& threading,
							  const QVector<int>& tieup,
							  const QVector<int>& treadling,
							  const glm::vec2& patternScale, int warpMaterial,
							  const glm::vec3& warpColor, int weftMaterial,
							  const glm::vec3& weftColor);

private slots:
	void tick();

signals:
	void glInitialized();
};


#endif // MYGL_H
