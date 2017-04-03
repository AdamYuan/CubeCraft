#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in float tex;
layout(location=2) in float face;
layout(location=3) in vec2 lighting;
uniform mat4 matrix;
out float frag_face;
out float frag_tex;
out float frag_ao;
out float frag_light;
out vec3 pos;

const float AOcurve[4] = float[4](0.55, 0.7, 0.85, 1.0);

void main()
{
	frag_face=face;
	frag_tex=tex;
	frag_ao=AOcurve[int(lighting.x+0.5)];
	frag_light=((lighting.y+2.0)/17.0);

	gl_Position=matrix*position;

	pos=ivec3(position.xyz + 0.5);
}
