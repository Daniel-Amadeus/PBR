#include "EmptyExample.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/resources/ResourceManager.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/Icosahedron.h>


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

EmptyExample::EmptyExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
	setupPropertyGroup();
}

EmptyExample::~EmptyExample() = default;

void EmptyExample::setupPropertyGroup()
{
	addProperty<float>("eyeX", this,
		&EmptyExample::eyeX, &EmptyExample::setEyeX)->setOptions({
			{ "minimum", -5.0f },
			{ "maximum", 5.0f },
			{ "step", 0.1f },
			{ "precision", 1u } });
	addProperty<float>("eyeY", this,
		&EmptyExample::eyeY, &EmptyExample::setEyeY)->setOptions({
			{ "minimum", -5.0f },
			{ "maximum", 5.0f },
			{ "step", 0.1f },
			{ "precision", 1u } });
	addProperty<float>("eyeZ", this,
		&EmptyExample::eyeZ, &EmptyExample::setEyeZ)->setOptions({
			{ "minimum", -5.0f },
			{ "maximum", 5.0f },
			{ "step", 0.1f },
			{ "precision", 1u } });
}

void EmptyExample::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

float EmptyExample::eyeX() const
{
	return m_cameraCapability->eye().x;
}
void EmptyExample::setEyeX(float eyeX)
{
	glm::vec3 eye = m_cameraCapability->eye();
	eye.x = eyeX;
	m_cameraCapability->setEye(eye);
}
float EmptyExample::eyeY() const
{
	return m_cameraCapability->eye().y;
}
void EmptyExample::setEyeY(float eyeY)
{
	glm::vec3 eye = m_cameraCapability->eye();
	eye.y = eyeY;
	m_cameraCapability->setEye(eye);
}
float EmptyExample::eyeZ() const
{
	return m_cameraCapability->eye().z;
}
void EmptyExample::setEyeZ(float eyeZ)
{
	glm::vec3 eye = m_cameraCapability->eye();
	eye.z = eyeZ;
	m_cameraCapability->setEye(eye);
}

void EmptyExample::onInitialize()
{
    // create program

    globjects::init();

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({0.6f, 0.6f, 0.6f});

    m_icosahedron = new gloperate::Icosahedron{3};

    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron.frag")
    );

	m_transformLocation = m_program->getUniformLocation("transform");
	m_eyeLocation =  m_program->getUniformLocation("a_eye");

	std::cout << "m_eyeLocation = " << m_eyeLocation << std::endl;

	m_icoTransform = glm::mat4x4();
	m_icoTransform = glm::translate(m_icoTransform, glm::vec3(0.0f, 1.0f, 0.0f));

	m_cameraCapability->setEye(glm::vec3(-1.2f, 2.1f, -2.8));
	m_cameraCapability->setCenter(glm::vec3(0.9f, 0.5f, 2.0));

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    setupProjection();


	m_color = m_resourceManager.load<globjects::Texture>("data/emptyexample/test_grid.png");

	m_normals = m_resourceManager.load<globjects::Texture>("data/emptyexample/normals.png");
	m_normals->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normals->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	m_envmap = m_resourceManager.load<globjects::Texture>("data/emptyexample/env_sphere_2.png");
}

void EmptyExample::onPaint()
{
    if (m_viewportCapability->hasChanged())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
    }

    auto fbo = m_targetFramebufferCapability->framebuffer();

    if (!fbo)
        fbo = globjects::Framebuffer::defaultFBO();

    fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
	
	const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();

    m_grid->update(eye, transform);
    m_grid->draw();

	/*std::cout << m_cameraCapability->eye()[0] << " "
		<< m_cameraCapability->eye()[1] << " "
		<< m_cameraCapability->eye()[2] << " " << std::endl
		<< m_cameraCapability->center()[0] << " "
		<< m_cameraCapability->center()[1] << " "
		<< m_cameraCapability->center()[2] << " " << std::endl << std::endl;/**/

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	m_color->bind();

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	m_normals->bind();

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	m_envmap->bind();

	m_program->use();
	m_program->setUniform("projection", transform);
	m_program->setUniform("transform", m_icoTransform);
	m_program->setUniform(m_eyeLocation, m_cameraCapability->eye());
	m_program->setUniform("tex", 0);
	m_program->setUniform("normals", 1);
	m_program->setUniform("envmap", 2);

    m_icosahedron->draw();


    m_program->release();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
