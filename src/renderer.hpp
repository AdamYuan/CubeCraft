#pragma once
#include "world.hpp"
class renderer
{
  public:
	enum textStyle {regular=0,bold};
	static void applyChunkMesh(chunk *chk);
	static void renderWorld(world *wld);
	static void renderCross();
	static void renderText(const glm::vec2 &pos,const std::string &str,
						   int t_width,int t_height,
						   const glm::vec4 &text_color,
						   const glm::vec4 &bg_color,
						   const glm::mat4 &matrix,
						   textStyle style);
};
