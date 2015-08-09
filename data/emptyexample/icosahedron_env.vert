#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;
out vec3 v_vertex_loc;
out vec3 v_vertex_glob;
out vec3 v_eye;
out vec3 v_lightDir;

uniform mat4 projection;
uniform mat4 transform;
uniform vec3 a_eye;

vec3 lightDir = vec3(0.0, 2.0, 0.0);

void main()
{
	vec4 pos = transform * vec4(a_vertex, 1.0);
	v_vertex_loc = a_vertex;
	v_vertex_glob = pos.xyz / pos.w;
    v_normal = a_normal;
	v_eye = a_eye - v_vertex_glob;
	vec4 lightDir = projection * vec4(lightDir, 1.0);
	v_lightDir = lightDir.xyz / lightDir.w;
	gl_Position = projection * pos;
}
