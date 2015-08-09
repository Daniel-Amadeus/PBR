#include "PhysicallyBasedRenderingExample.h"

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
	
	m_presetProperty = addProperty<Preset>("preset", this,
		&EmptyExample::getPreset,
		&EmptyExample::setPreset);
	m_presetProperty->setStrings({
			{ Preset::manual, "manual" },
			{ Preset::gold, "gold" } });

	//======= manual settings =======

	m_manualSettingsPropertyGroup = addGroup("manualSettings");

	m_programPresetProperty = m_manualSettingsPropertyGroup->addProperty<ProgramPreset>("ProgramPreset", this,
		&EmptyExample::getProgramPreset,
		&EmptyExample::setProgramPreset);
	m_programPresetProperty->setStrings({
			{ ProgramPreset::withoutEnvMap, "WithoutEnvMap" },
			{ ProgramPreset::withEnvMap, "WithEnvMap" } });

	//======= albedo color =======

	m_albedoPresetProperty = m_manualSettingsPropertyGroup->addProperty<AlbedoPreset>("albedo", this,
		&EmptyExample::albedoPreset,
		&EmptyExample::setAlbedoPreset);
	m_albedoPresetProperty->setStrings({
			{ AlbedoPreset::color, "color" },
			{ AlbedoPreset::metal, "metalTex" } });

	m_albedoColorGroup = m_manualSettingsPropertyGroup->addGroup("albedoColor");

	m_albedoColorGroup->addProperty<float>("albedoColorR", this,
		&EmptyExample::albedoColorR, &EmptyExample::setAlbedoColorR)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });
	m_albedoColorGroup->addProperty<float>("albedoColorG", this,
		&EmptyExample::albedoColorG, &EmptyExample::setAlbedoColorG)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });
	m_albedoColorGroup->addProperty<float>("albedoColorB", this,
		&EmptyExample::albedoColorB, &EmptyExample::setAlbedoColorB)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	m_microsurfaceProperty = m_manualSettingsPropertyGroup->addProperty<float>("microsurface", this,
		&EmptyExample::microsurface, &EmptyExample::setMicrosurface);
	m_microsurfaceProperty->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	//======= reflectivity =======

	m_reflectivityGroup = m_manualSettingsPropertyGroup->addGroup("reflectivity");


	m_reflectivityGroup->addProperty<float>("reflectivityR", this,
		&EmptyExample::reflectivityR, &EmptyExample::setReflectivityR)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	m_reflectivityGroup->addProperty<float>("reflectivityG", this,
		&EmptyExample::reflectivityG, &EmptyExample::setReflectivityG)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	m_reflectivityGroup->addProperty<float>("reflectivityB", this,
		&EmptyExample::reflectivityB, &EmptyExample::setReflectivityB)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	//======= normal map =======

	m_normalMapPresetProperty = m_manualSettingsPropertyGroup->addProperty<NormalMapPreset>("normalMap", this,
		&EmptyExample::normalMapPreset,
		&EmptyExample::setNormalMapPreset);
	m_normalMapPresetProperty->setStrings({
		{ NormalMapPreset::none, "none" },
		{ NormalMapPreset::metal, "metal" } });

	/*m_manualSettingsPropertyGroup->addProperty<bool>("useNormalMap", this,
		&EmptyExample::useNormalMap, &EmptyExample::setUseNormalMap);/**/
	
}

void EmptyExample::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

AlbedoPreset EmptyExample::albedoPreset() const
{
	return m_albedoPreset;
}
void EmptyExample::setAlbedoPreset(AlbedoPreset preset)
{
	m_albedoPreset = preset;
	m_manualSettingsPropertyGroup->clear();
	switch (m_albedoPreset)
	{
	case AlbedoPreset::color:
		m_material.setAlbedo(m_material.albedoColor());
		m_manualSettingsPropertyGroup->addProperty(m_programPresetProperty);
		m_manualSettingsPropertyGroup->addProperty(m_albedoPresetProperty);
		m_manualSettingsPropertyGroup->addProperty(m_albedoColorGroup);
		m_manualSettingsPropertyGroup->addProperty(m_microsurfaceProperty);
		m_manualSettingsPropertyGroup->addProperty(m_reflectivityGroup);
		m_manualSettingsPropertyGroup->addProperty(m_normalMapPresetProperty);
		break;
	case AlbedoPreset::metal:
		//m_material.setAlbedo(m_albedoColor);
		m_material.setAlbedo(m_material.albedoTex());
		m_manualSettingsPropertyGroup->addProperty(m_programPresetProperty);
		m_manualSettingsPropertyGroup->addProperty(m_albedoPresetProperty);
		m_manualSettingsPropertyGroup->addProperty(m_microsurfaceProperty);
		m_manualSettingsPropertyGroup->addProperty(m_reflectivityGroup);
		m_manualSettingsPropertyGroup->addProperty(m_normalMapPresetProperty);
		break;
	default:
		break;
	}
}

float EmptyExample::albedoColorR() const
{
	return m_material.albedoColor().r;// m_albedoColor.r;
}
void EmptyExample::setAlbedoColorR(float red)
{
	glm::vec3 color = m_material.albedoColor();
	color[0] = red;
	m_material.setAlbedo(color);
}
float EmptyExample::albedoColorG() const
{
	return m_material.albedoColor().g;// m_albedoColor.g;
}
void EmptyExample::setAlbedoColorG(float green)
{
	glm::vec3 color = m_material.albedoColor();
	color[1] = green;
	m_material.setAlbedo(color);
}
float EmptyExample::albedoColorB() const
{
	return m_material.albedoColor().b;// m_albedoColor.b;
}
void EmptyExample::setAlbedoColorB(float blue)
{
	glm::vec3 color = m_material.albedoColor();
	color[2] = blue;
	m_material.setAlbedo(color);
}

float EmptyExample::microsurface() const
{
	return m_material.microsurface();// m_microsurface;
}
void EmptyExample::setMicrosurface(float newMicrosurface)
{
	m_material.setMicrosurface(newMicrosurface);
}

float EmptyExample::reflectivityR() const
{
	return m_reflectivity[0];
}
void EmptyExample::setReflectivityR(float red)
{
	m_reflectivity[0] = red;
	m_material.setReflectivity(m_reflectivity);
}

float EmptyExample::reflectivityG() const
{
	return m_reflectivity[1];
}
void EmptyExample::setReflectivityG(float green)
{
	m_reflectivity[1] = green;
	m_material.setReflectivity(m_reflectivity);
}

float EmptyExample::reflectivityB() const
{
	return m_reflectivity[2];
}
void EmptyExample::setReflectivityB(float blue)
{
	m_reflectivity[2] = blue;
	m_material.setReflectivity(m_reflectivity);
}

NormalMapPreset EmptyExample::normalMapPreset() const
{
	return m_normalMapPreset;
}
void EmptyExample::setNormalMapPreset(NormalMapPreset preset)
{
	m_normalMapPreset = preset;
	switch (m_normalMapPreset)
	{
	case NormalMapPreset::none:
		m_material.setNormalMap(nullptr);
		break;
	case NormalMapPreset::metal:
		m_material.setNormalMap(m_normals);
		break;
	default:
		break;
	}
}

ProgramPreset EmptyExample::getProgramPreset() const
{
	return m_programPreset;
}
void EmptyExample::setProgramPreset(ProgramPreset programPreset)
{
	m_programPreset = programPreset;
	switch (m_programPreset)
	{
	case ProgramPreset::withoutEnvMap:
		m_material.setProgramByPreset(ProgramPreset::withoutEnvMap);
		break;
	case ProgramPreset::withEnvMap:
		m_material.setProgramByPreset(ProgramPreset::withEnvMap);
		break;
	default:
		break;
	}
}

Preset EmptyExample::getPreset() const
{
	return m_preset;
}
void EmptyExample::setPreset(Preset newPreset)
{
	m_preset = newPreset;
	clear();
	addProperty(m_presetProperty);
	PBRMaterial newMaterial = PBRMaterial();
	switch (newPreset)
	{
	case Preset::manual:
		newMaterial.copySettings(m_material);
		m_material = newMaterial;
		addProperty(m_manualSettingsPropertyGroup);
		break;
	case Preset::gold:
		m_material = m_gold;
		break;
	default:
		break;
	}
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

	m_icosahedron = new gloperate::Icosahedron{ 3 };

	m_programWithEnv = new Program{};
	m_programWithEnv->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron_env.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron_env.frag")
		);

	m_programNoEnv = new Program{};
	m_programNoEnv->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron_noEnv.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron_noEnv.frag")
		);

	m_transformLocation = m_programNoEnv->getUniformLocation("transform");
	m_eyeLocation = m_programNoEnv->getUniformLocation("a_eye");
	m_programPreset = ProgramPreset::withEnvMap;

	//std::cout << "m_eyeLocation = " << m_eyeLocation << std::endl;

	m_icoTransform = glm::mat4x4();
	m_icoTransform = glm::translate(m_icoTransform, glm::vec3(0.0f, 1.0f, 0.0f));

	m_cameraCapability->setEye(glm::vec3(-1.2f, 2.1f, -2.8));
	m_cameraCapability->setCenter(glm::vec3(0.9f, 0.5f, 2.0));

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    setupProjection();


	m_albedoTex = m_resourceManager.load<globjects::Texture>("data/emptyexample/metal/MetalScratches_COLOR.png");
	m_albedoTex->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_albedoTex->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);


	m_normals = m_resourceManager.load<globjects::Texture>("data/emptyexample/metal/MetalScratches_NRM.png");
	m_normals->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normals->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	m_envmap = m_resourceManager.load<globjects::Texture>("data/emptyexample/Panorama_0.png");
	/*m_envmap->generateMipmap();
	m_envmap->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_envmap->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_envmap->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_envmap->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);*/

	//m_albedoColor = glm::vec3(0.22, 0.32, 0.50); //plastic
	//m_albedoColor = glm::vec3(0.0, 0.0, 0.0); //gold

	//setMicrosurface(0.46f); //plastic
	setMicrosurface(0.86f); //gold
	//m_reflectivity = glm::vec3(0.34, 0.41, 0.53);//plastic
	m_reflectivity = glm::vec3(1.0, 0.86, 0.58);//gold
	//m_useNormalMap = true;

	//m_preset = preset::gold;
	m_albedoPreset = AlbedoPreset::color;
	m_normalMapPreset = NormalMapPreset::metal;
	setPreset(Preset::gold);

	m_gold = PBRMaterial(ProgramPreset::withEnvMap, m_envmap, m_normals, glm::vec3(0.0f, 0.0f, 0.0f), 0.86f, glm::vec3(1.0f, 0.86f, 0.58f));
	
}

void EmptyExample::onPaint()
{
	globjects::ref_ptr<globjects::Program> program;
	
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

	switch (m_material.programPreset())
	{
	case ProgramPreset::withoutEnvMap:
		m_material.setProgramByPreset(ProgramPreset::withoutEnvMap);
		break;
	case ProgramPreset::withEnvMap:
		m_material.setProgramByPreset(ProgramPreset::withEnvMap);
		break;
	default:
		break;
	}
	program = m_material.program();

	program->use();
	program->setUniform("projection", transform);
	program->setUniform("transform", m_icoTransform);
	program->setUniform(m_eyeLocation, m_cameraCapability->eye());

	m_material.use();

	m_icosahedron->draw();

	program->release();

	m_material.release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
