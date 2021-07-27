uniform vec3 l_dir;//¹âÔ´Î»ÖÃ
uniform vec4 l_diffuse;
uniform vec4 l_ambient;
uniform vec4 l_specular;
uniform float l_shininess;

varying vec3 normal;
varying vec4 eye;

void main(){
	// set the specular term to black
    vec4 spec = vec4(0.0);

	// normalize both input vectors
	vec3 n = normalize(normal);
	vec3 e = normalize(vec3(eye));

	float intensity = max(dot(n,l_dir), 0.0);

	// if the vertex is lit compute the specular color
	if (intensity > 0.0){
		// compute the half vector
		vec3 h = normalize(l_dir + e);
		// compute the specular term into spec
		float intSpec = max(dot(h,n), 0.0);
		spec = l_specular * pow(intSpec,l_shininess);
	}

	gl_FragColor = max(intensity *  l_diffuse + spec, l_ambient);
}