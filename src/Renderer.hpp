#pragma once
#include "World.hpp"
class Renderer
{
public:
	enum TextStyle {regular=0, bold};
	static void ApplyChunkMesh(ChunkPtr chk);
	static void RenderWorld(World *wld);
	static void RenderCross();
	static void RenderText(const glm::vec2 &pos, const std::string &str,
						   int t_width, int t_height,
						   const glm::vec4 &text_color,
						   const glm::vec4 &bg_color,
						   const glm::mat4 &matrix,
						   TextStyle style);
};
