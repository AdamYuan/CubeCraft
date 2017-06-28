#version 330 core
layout(location=0) in vec4 position;
layout(location=2) in float tex;
layout(location=3) in float face;
layout(location=4) in vec2 lighting;
uniform mat4 matrix;
out float frag_face;
out float frag_tex;
out float frag_ao;
out float frag_light;
out vec3 pos;

const float AOcurve[4] = float[4](0.6, 0.7333333, 0.8666666, 1.0);

void main()
{
	frag_face=face;
	frag_tex=tex;
	frag_ao=AOcurve[int(lighting.x+0.5)];
	frag_light=1.0f;//((lighting.y+2.0)/17.0);

	gl_Position=matrix*position;

   	pos=round(position.xyz);
}
