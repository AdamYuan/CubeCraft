#version 330 core
uniform sampler2D sampler;
uniform vec4 bg_color;
uniform vec4 text_color;
in vec2 coord2d;

out vec4 color;

void main()
{
	vec4 c=texture(sampler,coord2d);
	if(coord2d.y<=0.05)
		color=bg_color;
	else
		color=mix(bg_color,text_color,c.z+0.3);
}
