#version 150 core

in vec3 v_normal;
in vec3 v_vertex_loc;
in vec3 v_eye;

uniform vec3 u_albedoColor;
uniform sampler2D u_albedoTex;
uniform bool u_useAlbedoColor;
uniform sampler2D u_normals;
uniform bool u_useNormalMap;
uniform sampler2D u_envmap;
uniform float u_microsurface;
uniform vec3 u_reflectivity;

const float c_pi = 3.1415926536;

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

vec4 env(in vec3 eye)
{
	vec2 uv = vec2
	(
		(atan(eye.x, -eye.z)+c_pi) / (2 * c_pi),
		1.0 - 1.0 * acos(eye.y) / c_pi
	);
	return texture2D(u_envmap, uv);
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
	vec3 view = normalize(v_eye);
	vec3 reflectionDir = normalize(reflect(-view, normal));
	
	vec3 albedo = mix(u_albedoColor, texture(u_albedoTex, uv).xyz, u_useAlbedoColor);
	vec3 reflection = env(reflectionDir).rgb;
	
	float frsl0 = pow(1.0 - max(0.0, dot(normal, view)), u_microsurface * -3.0 + 5.0); 
	vec3 frsl = mix(vec3(frsl0), vec3(1.0), u_reflectivity);
	frsl = clamp(frsl, 0.0, 1.0);
	
	fragColor = vec4( mix(albedo, reflection, frsl), 1.0);
	//fragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
