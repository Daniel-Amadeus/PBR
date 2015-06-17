#version 150 core

in vec3 v_normal;
in vec3 v_vertex_loc;
in vec3 v_vertex_glob;
in vec3 v_tangent;
in vec3 v_eye;

uniform sampler2D tex;
uniform sampler2D normals;
uniform sampler2D envmap;

const float c_pi = 3.1415926536;
const float c_eta = 1.5;

vec3 pointLampPos = vec3(0.0, 1.0, 0.0);
vec3 pointLampI = vec3(0.9, 0.8, 0.6);

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
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    vec3 map = texture2D( normals, texcoord ).xyz;
    map = map * 255./127. - 128./127.; //*2.0 - 1.0
	
    map.y = -map.y;
	map.x = -map.x;	
	
    mat3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( TBN * map );
}

vec4 env(in vec3 eye)
{
	vec4 color;
	
	/*if(0 == mapping) 		// cube
	{
		// use texture function with the cubemap sampler
		color = texture(cubemap, vec3(eye.x, eye.y, -eye.z));
	}
	else if(1 == mapping) 	// polar
	{	
		// use texture function with the envmap sampler
		vec2 uv = vec2((atan(eye.x, -eye.z)+c_pi) / (2 * c_pi), 2 * acos(eye.y) / c_pi);
		color = texture2D(envmap, uv);
	}	
	else if(2 == mapping) 	// paraboloid
	{
		// use texture function with the envmap sampler
		float m = 2.0 + 2.0 * eye.y;
		vec2 uv = vec2(0.5 + eye.x/m, 0.5 - eye.z/m);
		color = texture2D(envmap, uv);
	}
	else if(3 == mapping) 	// sphere
	{
		// use texture function with the envmap sampler
		float m = 2.0 * sqrt(eye.x * eye.x + eye.y * eye.y + (1.0 + eye.z) * (1.0 + eye.z));
		vec2 uv = vec2(0.5 - eye.x/m, 0.5 - eye.y/m);
		color = texture2D(envmap, uv);
	}
	else if(4 == mapping) 	// sphere_2
	{*/
		// use texture function with the envmap sampler
		float m = -2.0 * sqrt(eye.x * eye.x + eye.y * eye.y + (1.0 + eye.z) * (1.0 + eye.z));
		vec2 uv = vec2(0.5 - eye.x/m, 0.5 - eye.y/m);
		color = texture2D(envmap, uv);
	//}
	return color;
}

void main()
{
	
	
	vec2 uv = vec2(1.0 - (atan(v_vertex_loc.z, v_vertex_loc.x) / c_pi + 1.0) * 0.5, (asin(v_vertex_loc.y) / c_pi + 0.5));
	
	vec2 normalUV = 4.0 * uv;
	vec3 N = normalize(perturb_normal(v_normal, v_vertex_loc, normalUV));
	//N = v_normal;
	
	vec3 ambientI = vec3(0.1, 0.1, 0.3);

	vec3 pointLampPos = 5.0 * vec3(2.0, 1.0, 0.0);
	vec3 pointLampI = vec3(0.9, 0.8, 0.6);
	
	vec3 kd = texture(tex, uv).xyz;
	//kd = vec3(0.5, 0.5, 0.5);
	vec3 ka = 1 * kd; //vec3(0.1, 0.9, 0.2);
	vec3 ks = vec3(0.7, 0.7, 0.9);
	float exp = 72.0;
	
	vec3 lightDir = normalize(pointLampPos - v_vertex_glob);
	vec3 n = normalize(N);
	vec3 r = reflect(-lightDir, n);
	vec3 v = normalize(v_eye);
	vec3 reflectionDir = normalize(reflect(-v_eye, n));
	vec3 refractionDir = normalize(refract(v_eye, n, 1.0 / c_eta));
	
	float frsl = pow(1.0 - max(0.0, dot(n, v)), c_eta);
	
	vec3 Ia = ambientI * ka;
	vec3 Id = clamp(pointLampI * kd * dot(lightDir, n), 0.0, 1.0);
	vec3 Is = clamp(pointLampI * ks * pow(clamp(dot(r, v), 0.0, 1.0), exp), 0.0, 1.0);
	
	vec3 reflection = env(reflectionDir).rgb;
	vec3 refrection = env(refractionDir).rgb;
	
	//fragColor = vec4(Ia + Id + Is, 1.0);
	fragColor = vec4(mix(Ia + Id + Is, reflection, frsl), 1.0);
	//fragColor = vec4(mix(Ia + Id + Is, reflection, frsl), 1.0);
	//fragColor = vec4(vec3(frsl), 1.0);
}
