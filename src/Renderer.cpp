#include <algorithm>
#include "Renderer.hpp"
#include "Resource.hpp"
#include "Game.hpp"

//update the vertex mesh of a chunk
void Renderer::ApplyChunkMesh(ChunkPtr chk) {
	//add the data array into the VAO,VBO
	chk->SolidMeshObject->SetDataVec(chk->SolidMeshData);
	chk->SolidMeshObject->SetAttributes(4,
										Resource::Attributes::Position, 3,
										Resource::Attributes::Texcoord, 3,
										Resource::Attributes::BFacing, 1,
										Resource::Attributes::BLighting, 2);

	chk->SemitransMeshObject->SetDataVec(chk->SemitransMeshData);
	chk->SemitransMeshObject->SetAttributes(4,
										Resource::Attributes::Position, 3,
										Resource::Attributes::Texcoord, 3,
										Resource::Attributes::BFacing, 1,
										Resource::Attributes::BLighting, 2);
}

void Renderer::RenderWorld(World *wld) {
	//shader
	Resource::BlockShader.Use();

	//texture
	glActiveTexture(GL_TEXTURE0);
	Resource::BlockTexture.Bind();

	Resource::BlockShader.PassInt(Resource::UniformSampler, 0);

	//other uniforms
	Resource::BlockShader.PassFloat(Resource::BlockUniformViewDistance, VIEW_DISTANCE);
	Resource::BlockShader.PassMat4(Resource::UniformMatrix, Game::matrices.Projection3d * Game::camera.GetViewMatrix());
	Resource::BlockShader.PassVec3(Resource::BlockUniformCamera, Game::camera.Position);

	std::vector<ChunkPtr> visibleChunks;

	//Render
	for (glm::ivec3 pos:wld->ChunkRenderList) {
		glm::vec3 center = (glm::vec3) pos * (float) CHUNK_SIZE + glm::vec3(CHUNK_SIZE / 2);
		//Frustum Culling
		if (!Game::frustum.CubeInFrustum(center, CHUNK_SIZE / 2))
			continue;

		ChunkPtr chk = wld->Voxels.GetChunk(pos);
		//if(chk->SolidMeshObject->Empty() && chk->SemitransMeshObject->Empty())
		//	continue;

		visibleChunks.push_back(chk);
	}
	std::sort(visibleChunks.begin(), visibleChunks.end(),
			  [=](const ChunkPtr &a, const ChunkPtr &b)
			  {
				  return glm::distance(glm::vec3(Game::player.ChunkPos), glm::vec3(a->ChunkPos)) <
						  glm::distance(glm::vec3(Game::player.ChunkPos), glm::vec3(b->ChunkPos));
			  });

	glEnable(GL_POLYGON_OFFSET_FILL);

	glPolygonOffset(1, 1);

	Resource::BlockShader.PassInt(Resource::BlockUniformIsSemitransparent, 0);
	for(ChunkPtr chk : visibleChunks)
		chk->SolidMeshObject->Render(GL_TRIANGLES);

	glDepthMask(GL_FALSE);

	Resource::BlockShader.PassInt(Resource::BlockUniformIsSemitransparent, 1);
	for(ChunkPtr chk : visibleChunks)
		chk->SemitransMeshObject->Render(GL_TRIANGLES);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
}

void Renderer::RenderCross() {
	float p = 10.0, ht = 1.0f;
	static const float vertices[] = {-ht, -p, -ht, p, ht, -p,
									 ht, -p, -ht, p, ht, p,
									 -p, ht, p, -ht, -p, -ht,
									 p, ht, p, -ht, -p, ht,
									 -ht, -ht, -ht, ht, ht, -ht,
									 ht, -ht, -ht, ht, ht, ht};
	MyGL::VertexObject crossObj;
	crossObj.SetDataArr(vertices, 36);
	crossObj.SetAttributes(1, Resource::Attributes::Position, 2);

	Resource::LineShader.Use();

	Resource::LineShader.PassMat4(Resource::UniformMatrix, Game::matrices.Matrix2dCenter);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_INVERT); //invert color

	crossObj.Render(GL_TRIANGLES);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_LOGIC_OP);
}

void Renderer::RenderText(const glm::vec2 &pos, const std::string &str,
						  int t_width, int t_height,
						  const glm::vec4 &text_color,
						  const glm::vec4 &bg_color,
						  const glm::mat4 &matrix,
						  TextStyle style) {
	std::vector<vert_text> vertices;
	MyGL::VertexObject text_obj;
	float unit = 1.0f / 96.0f;
	for (int c = 0; c < (int) str.size(); ++c) {
		int coord = str[c] - 32;
		//00--------10
		//| \       |
		//|    \    |
		//|       \ |
		//11--------01
		vert_text v00 = {pos.x + t_width * c, pos.y, coord * unit, 0.0f};
		vert_text v01 = {pos.x + t_width * (c + 1), pos.y + t_height, (coord + 1.0f) * unit, 1.0f};
		vert_text v10 = {pos.x + t_width * (c + 1), pos.y, (coord + 1.0f) * unit, 0.0f};
		vert_text v11 = {pos.x + t_width * c, pos.y + t_height, coord * unit, 1.0f};
		vertices.push_back(v00);
		vertices.push_back(v01);
		vertices.push_back(v10);

		vertices.push_back(v00);
		vertices.push_back(v11);
		vertices.push_back(v01);
	}
	text_obj.SetDataVec(vertices);
	text_obj.SetAttributes(2,
						   Resource::Attributes::Position, 2,
						   Resource::Attributes::Texcoord, 2);

	Resource::TextShader.Use();

	//set matrix
	Resource::TextShader.PassMat4(Resource::UniformMatrix, matrix);

	//set colors
	Resource::TextShader.PassVec4(Resource::TextUniformTextColor, text_color);
	Resource::TextShader.PassVec4(Resource::TextUniformBgColor, bg_color);

	//texture
	glActiveTexture(GL_TEXTURE0);
	if (style == TextStyle::regular)
		Resource::FontTexture.Bind();
	else if (style == TextStyle::bold)
		Resource::FontBoldTexture.Bind();

	Resource::TextShader.PassInt(Resource::UniformSampler, 0);

	glDisable(GL_DEPTH_TEST);
	text_obj.Render(GL_TRIANGLES);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::RenderSelectionBox()
{
	std::vector<float> vertices =
			{
					1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
					1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
					1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
					0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
					1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
					1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
					1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			};

	MyGL::VertexObject box;

	box.SetDataVec(vertices);
	box.SetAttributes(1, Resource::Attributes::Position, 3);

	Resource::LineShader.Use();
	Resource::LineShader.PassMat4(Resource::UniformMatrix,
								  Game::matrices.Projection3d * Game::camera.GetViewMatrix() * glm::translate((glm::vec3)(Game::player.SelectedPosition)));

	box.Render(GL_LINES);
}
