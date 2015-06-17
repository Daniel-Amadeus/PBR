#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Texture.h>

#include <gloperate/painter/Painter.h>

#include <glm/mat4x4.hpp>


namespace globjects
{
    class Program;
}

namespace gloperate
{
	class AdaptiveGrid;
	class ResourceManager;
    class Icosahedron;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class EmptyExample : public gloperate::Painter
{
public:
    EmptyExample(gloperate::ResourceManager & resourceManager);
    virtual ~EmptyExample();

	void EmptyExample::setupPropertyGroup();

    void setupProjection();

	float eyeX() const;
	void setEyeX(float eyeX);
	float eyeY() const;
	void setEyeY(float eyeY);
	float eyeZ() const;
	void setEyeZ(float eyeZ);

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;

protected:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    globjects::ref_ptr<gloperate::Icosahedron> m_icosahedron;
    globjects::ref_ptr<globjects::Program> m_program;
	//glm::vec3 m_pointLamp;
	gl::GLint m_transformLocation;
	gl::GLint m_eyeLocation;

	globjects::Texture *m_color;
	globjects::Texture *m_normals;
	globjects::Texture *m_envmap;
	glm::mat4x4 m_icoTransform;
};
