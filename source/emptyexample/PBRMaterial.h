#include <glm/vec3.hpp>
#include <globjects/Texture.h>
#include <globjects/Program.h>

enum class ProgramPreset { withEnvMap, withoutEnvMap, custom};


class PBRMaterial
{

public:
	PBRMaterial();
	PBRMaterial(
		ProgramPreset programPreset,
		globjects::Texture *envmap,
		globjects::Texture *normalMap,
		glm::vec3 albedoColor,
		float microsurface,
		glm::vec3 reflectivity);
	PBRMaterial(
		ProgramPreset programPreset,
		globjects::Texture *envmap,
		globjects::Texture *normalMap,
		globjects::Texture *albedoTex,
		float microsurface,
		glm::vec3 reflectivity);
	PBRMaterial clone();
	void copySettings(PBRMaterial material);

	~PBRMaterial();

	void use();
	void release();

	ProgramPreset programPreset();
	void setProgramByPreset(ProgramPreset programPreset);

	globjects::ref_ptr<globjects::Program> program();
	void setProgram(globjects::ref_ptr<globjects::Program> program);

	globjects::Texture *normalMap();
	void setNormalMap(globjects::Texture *normalMap);

	glm::vec3 albedoColor() const;
	globjects::Texture *albedoTex() const;
	bool hasAlbedoTex();
	void setAlbedo(glm::vec3 color);
	void setAlbedo(globjects::Texture *tex);

	float microsurface() const;
	void setMicrosurface(float microsurface);

	glm::vec3 reflectivity() const;
	void setReflectivity(glm::vec3 reflectivity);
	
protected:

private:

	PBRMaterial(
		globjects::ref_ptr<globjects::Program> program,
		ProgramPreset programPreset,
		globjects::Texture *envMap,
		globjects::Texture *normalMap,
		glm::vec3 albedoColor,
		globjects::Texture *albedoTex,
		float lod,
		float microsurface,
		glm::vec3 reflectivity);

	ProgramPreset m_programPreset;

	void prepareEnvMap();

	globjects::ref_ptr<globjects::Program> m_program;

	globjects::Texture *m_envMap;

	globjects::Texture *m_normalMap;

	glm::vec3 m_albedoColor;
	globjects::Texture *m_albedoTex;

	float m_lod;
	float m_microsurface;
	glm::vec3 m_reflectivity;
};