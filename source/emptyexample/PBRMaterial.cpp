#include <PBRMaterial.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/globjects.h>


#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

using namespace gl;
using namespace glm;
using namespace globjects;

PBRMaterial::PBRMaterial()
{
	setProgramByPreset(ProgramPreset::withoutEnvMap);
	m_envMap = nullptr;
	m_normalMap = nullptr;
	m_albedoColor = glm::vec3(0.0f, 0.0f, 0.0f);
	m_albedoTex = nullptr;
	setMicrosurface(0.5f);
	m_reflectivity = glm::vec3(1.0f, 1.0f, 1.0f);

}

PBRMaterial::PBRMaterial(
	ProgramPreset programPreset,
	globjects::Texture *envmap,
	globjects::Texture *normalMap,
	glm::vec3 albedoColor,
	float microsurface,
	glm::vec3 reflectivity)
{
	setProgramByPreset(programPreset);
	m_envMap = envmap;
	m_normalMap = normalMap;
	m_albedoColor = albedoColor;
	m_albedoTex = nullptr;
	setMicrosurface(microsurface);
	m_reflectivity = reflectivity;

	prepareEnvMap();
}

PBRMaterial::PBRMaterial(
	ProgramPreset programPreset,
	globjects::Texture *envmap,
	globjects::Texture *normalMap,
	globjects::Texture *albedoTex,
	float microsurface,
	glm::vec3 reflectivity)
{
	setProgramByPreset(programPreset);
	m_envMap = envmap;
	m_normalMap = normalMap;
	m_albedoColor = glm::vec3(0.0f, 0.0f, 0.0f);
	m_albedoTex = albedoTex;
	setMicrosurface(microsurface);
	m_reflectivity = reflectivity;

	prepareEnvMap();
}

PBRMaterial::PBRMaterial(
	globjects::ref_ptr<globjects::Program> program,
	ProgramPreset programPreset,
	globjects::Texture *envMap,
	globjects::Texture *normalMap,
	glm::vec3 albedoColor,
	globjects::Texture *albedoTex,
	float lod,
	float microsurface,
	glm::vec3 reflectivity)
{
	globjects::ref_ptr<globjects::Program> m_program = program;
	m_programPreset = programPreset;
	globjects::Texture *m_envMap = envMap;
	globjects::Texture *m_normalMap = normalMap;
	glm::vec3 m_albedoColor = albedoColor;
	globjects::Texture *m_albedoTex = albedoTex;
	float m_lod = lod;
	float m_microsurface = microsurface;
	glm::vec3 m_reflectivity = reflectivity;
}

PBRMaterial PBRMaterial::clone()
{
	return PBRMaterial(
		m_program,
		m_programPreset,
		m_envMap,
		m_normalMap,
		m_albedoColor,
		m_albedoTex,
		m_lod,
		m_microsurface,
		m_reflectivity);
}

void PBRMaterial::copySettings(PBRMaterial material)
{
	m_program = material.m_program;
	m_programPreset = material.m_programPreset;
	m_envMap = material.m_envMap;
	m_normalMap = material.m_normalMap;
	m_albedoColor = material.m_albedoColor;
	m_albedoTex = material.m_albedoTex;
	m_lod = material.m_lod;
	m_microsurface = material.m_microsurface;
	m_reflectivity = material.m_reflectivity;
}

PBRMaterial::~PBRMaterial() = default;

void PBRMaterial::use()
{
	if (m_envMap) m_envMap->setParameter(GL_TEXTURE_MIN_LOD, m_lod);

	if (m_albedoTex)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		m_albedoTex->bind();
	}

	if (m_normalMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		m_normalMap->bind();
	}

	if (m_envMap)
	{
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		m_envMap->bind();
	}


	m_program->setUniform("u_albedoColor", m_albedoColor);
	m_program->setUniform("u_albedoTex", 0);
	m_program->setUniform("u_useAlbedoColor", static_cast<bool>(m_albedoTex));
	m_program->setUniform("u_normals", 1);
	m_program->setUniform("u_useNormalMap", static_cast<bool>(m_normalMap));
	m_program->setUniform("u_envmap", 2);
	m_program->setUniform("u_microsurface", m_microsurface);
	m_program->setUniform("u_reflectivity", m_reflectivity);
}

void PBRMaterial::release()
{

	if (m_envMap)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	if (m_normalMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	if (m_albedoTex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

ProgramPreset PBRMaterial::programPreset()
{
	return m_programPreset;
}


globjects::ref_ptr<globjects::Program> PBRMaterial::program()
{
	return m_program;
}
void PBRMaterial::setProgram(globjects::ref_ptr<globjects::Program> program)
{
	m_program = program;
}

globjects::Texture *PBRMaterial::normalMap()
{
	return m_normalMap;
}
void PBRMaterial::setNormalMap(globjects::Texture *normalMap)
{
	m_normalMap = normalMap;
}

glm::vec3 PBRMaterial::albedoColor() const
{
	return m_albedoColor;
}

globjects::Texture *PBRMaterial::albedoTex() const
{
	return m_albedoTex;
}

bool PBRMaterial::hasAlbedoTex()
{
	return static_cast<bool>(m_albedoTex);
}

void PBRMaterial::setAlbedo(glm::vec3 color)
{
	m_albedoTex = nullptr;
	m_albedoColor = color;
}
void PBRMaterial::setAlbedo(globjects::Texture *tex)
{
	m_albedoTex = tex;
}

float PBRMaterial::microsurface() const
{
	return m_microsurface;
}

void PBRMaterial::setMicrosurface(float microsurface)
{
	m_lod = microsurface * -10.0f + 10.0; // 0.0->10.0 1.0->0.0
	m_microsurface = microsurface;
}

glm::vec3 PBRMaterial::reflectivity() const
{
	return m_reflectivity;
}

void PBRMaterial::setReflectivity(glm::vec3 reflectivity)
{
	m_reflectivity = reflectivity;
}

void PBRMaterial::setProgramByPreset(ProgramPreset programPreset)
{
	m_programPreset = programPreset;
	m_program = new Program{};

	switch (programPreset)
	{
	case ProgramPreset::withEnvMap:
		m_program->attach(
			Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron_env.vert"),
			Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron_env.frag")
			);
		break;
	case ProgramPreset::custom:
	case ProgramPreset::withoutEnvMap:
		m_program->attach(
			Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron_noEnv.vert"),
			Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron_noEnv.frag")
			);
		break;
	default:
		break;
	}

	
}

void PBRMaterial::prepareEnvMap()
{
	if (m_envMap)
	{
		m_envMap->generateMipmap();
		m_envMap->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		m_envMap->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		m_envMap->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		m_envMap->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}