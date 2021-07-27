varying vec3 normal;	//local space
varying vec4 eye;		//local space

void main(){
	normal = normalize(gl_NormalMatrix * gl_Normal);
	eye = -(gl_ModelViewMatrix * gl_Vertex);

	gl_Position = ftransform();
}