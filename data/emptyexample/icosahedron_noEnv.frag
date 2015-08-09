#version 150 core

in vec3 v_normal;
in vec3 v_vertex_loc;
in vec3 v_tangent;
in vec3 v_eye;
in vec3 v_lightDir;

uniform vec3 u_albedoColor;
uniform sampler2D u_albedoTex;
uniform bool u_useAlbedoColor;
uniform sampler2D u_normals;
uniform bool u_useNormalMap;
uniform vec3 diffColor;
uniform float u_microsurface;
uniform vec3 u_reflectivity;

const float c_pi = 3.1415926536;

vec3 lightColor = vec3(1.0);
vec3 ambientLight = vec3(0.15);

out vec4 fragColor;

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
    //N: interpolated vertex normal 
    //V:the view vector (vertex to eye)
    vec3 map = texture2D( u_normals, texcoord ).xyz;
    map = map * 2.0 - 1.0;
	
    map.y = -map.y;
	map.x = -map.x;	
	
    mat3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( TBN * map );
}

float frsl(float f0, vec3 n, vec3 l){
	return f0 + (1.0-f0) * pow(1.0- dot(n, l), 5.0);
}

float distribution(vec3 n, vec3 h, float roughness){
	float m= 2.0/(roughness*roughness) - 2.0;
	return (m+2.0) * pow( max(dot(n, h), 0.0), m) / (2.0 * 3.14159265);
}

float geometry(vec3 n, vec3 h, vec3 v, vec3 l, float roughness){
	return max(dot(n, l), 0.0) * max(dot(n, v), 0.0);
}

float diffuseEnergyRatio(float f0, vec3 n, vec3 l){
	return 1.0 - frsl(f0, n, l);
}

vec2 uvSphereMapping()
{
	return vec2
	(
		1.0 - (atan(v_vertex_loc.z, v_vertex_loc.x) / c_pi + 1.0) * 0.5,
		(asin(v_vertex_loc.y) / c_pi + 0.5)
	);
}

void main()
{
	
	vec2 uv = 1.0 * uvSphereMapping();
	
	vec3 normal = mix(v_normal, normalize(perturb_normal(v_normal, v_vertex_loc, uv)), u_useNormalMap);
	
	float reflectivity = u_reflectivity.x * 19.0 + 1.0;
	vec3 albedo = mix(u_albedoColor, texture(u_albedoTex, uv).xyz, u_useAlbedoColor);
	
	vec3 view = normalize(v_eye);
	
	float frsl0 = pow((1-reflectivity)/(1+reflectivity), 2.0);
	
	vec3 halfVec =  normalize(v_lightDir + view);
	float NdotL = dot(normal, v_lightDir);
	float NdotV = dot(normal, view);
	float NdotL_clamped = max(NdotL, 0.0);
	float NdotV_clamped = max(NdotV, 0.0);
	
	float brdf_spec = frsl(frsl0, halfVec, v_lightDir) 
			* geometry(normal, halfVec, view, v_lightDir, u_microsurface) 
			* distribution(normal, halfVec, u_microsurface)
			/ (4.0 * NdotL_clamped * NdotV_clamped);
	
	vec3 color_spec = NdotL_clamped * brdf_spec * lightColor;
	vec3 color_diff = NdotL_clamped * diffuseEnergyRatio(frsl0, normal, v_lightDir) * albedo * lightColor;
	
	fragColor = vec4( mix(clamp(color_diff + color_spec, 0.0, 1.0), albedo, ambientLight), 1.0);
}
