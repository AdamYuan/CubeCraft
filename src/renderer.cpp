#include "MyGL/matrix.hpp"
#include "MyGL/camera.hpp"
#include "MyGL/frustum.hpp"
#include "renderer.hpp"
#include "resource.hpp"
#include "util.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>

//update the vertex mesh of a chunk
void renderer::applyChunkMesh(chunkPtr chk)
{
	//add the data array into the VAO,VBO
	chk->obj.setDataVec(chk->meshData);
	chk->obj.setAttribs(4,
						resource::blockShader_attr_position,3,
						resource::blockShader_attr_tex,1,
						resource::blockShader_attr_face,1,
						resource::blockShader_attr_lighting,2);

	chk->meshData.clear();
	chk->meshData.shrink_to_fit();
}

void renderer::renderWorld(world *wld)
{
	//shader
	resource::blockShader.use();

	//texture
	glActiveTexture(GL_TEXTURE0);
	resource::blockTexture.bind();
	glUniform1i(resource::blockShader_unif_sampler, 0);

	//other uniforms
	glUniform1f(resource::blockShader_unif_viewdis,VIEW_DISTANCE);
	glUniformMatrix4fv(resource::blockShader_unif_matrix,1,GL_FALSE,
					   glm::value_ptr(matrix::projection * matrix::view));
	glUniform3fv(resource::blockShader_unif_camera,1,glm::value_ptr(camera::position));

	//render
	for(glm::ivec3 pos:wld->chunkRenderList)
	{
		glm::vec3 center=(glm::vec3)pos*(float)CHUNK_SIZE+glm::vec3(CHUNK_SIZE/2);
		//Frustum Culling
		if(!frustum::cubeInFrustum(center, CHUNK_SIZE/2))
			continue;
		wld->voxels.getChunk(pos)->obj.render(GL_TRIANGLES);
	}
}

void renderer::renderCross()
{
	float p = 15.0;
	static const float vertices[] = {0, -p, 0, p, -p, 0, p, 0};
	object crossObj;
	crossObj.setDataArr(vertices, 8);
	crossObj.setAttribs(1, resource::lineShader_attr_position, 2);

	resource::lineShader.use();

	glUniformMatrix4fv(resource::lineShader_unif_matrix, 1, GL_FALSE, glm::value_ptr(matrix::matrix2d_center));

	glLineWidth(3.0f);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_INVERT); //invert color

	crossObj.render(GL_LINES);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_LOGIC_OP);
}

void renderer::renderText(const glm::vec2 &pos,const std::string &str,
						  int t_width,int t_height,
						  const glm::vec4 &text_color,
						  const glm::vec4 &bg_color,
						  const glm::mat4 &matrix,
						  textStyle style)
{
	std::vector<vert_text> vertices;
	object text_obj;
	float unit = 1.0f/96.0f;
	for(int c=0;c<(int)str.size();++c)
	{
		int coord=str[c]-32;
		//00--------10
		//| \       |
		//|    \    |
		//|       \ |
		//11--------01
		vert_text v00={pos.x+t_width*c, pos.y, coord*unit, 0.0f};
		vert_text v01={pos.x+t_width*(c+1), pos.y+t_height, (coord+1.0f)*unit, 1.0f};
		vert_text v10={pos.x+t_width*(c+1), pos.y, (coord+1.0f)*unit, 0.0f};
		vert_text v11={pos.x+t_width*c, pos.y+t_height, coord*unit, 1.0f};
		vertices.push_back(v00);
		vertices.push_back(v01);
		vertices.push_back(v10);

		vertices.push_back(v00);
		vertices.push_back(v11);
		vertices.push_back(v01);
	}
	text_obj.setDataVec(vertices);
	text_obj.setAttribs(2,
						resource::textShader_attr_position,2,
						resource::textShader_attr_texcoord,2);

	resource::textShader.use();

	//set matrix
	glUniformMatrix4fv(resource::textShader_unif_matrix,1,GL_FALSE,glm::value_ptr(matrix));

	//set colors
	glUniform4fv(resource::textShader_unif_text_color,1,glm::value_ptr(text_color));
	glUniform4fv(resource::textShader_unif_bg_color,1,glm::value_ptr(bg_color));

	//texture
	glActiveTexture(GL_TEXTURE0);
	if(style==textStyle::regular)
		resource::fontTexture.bind();
	else if(style==textStyle::bold)
		resource::fontBoldTexture.bind();
	glUniform1i(resource::textShader_unif_sampler,0);

	glDisable(GL_DEPTH_TEST);
	text_obj.render(GL_TRIANGLES);
	glEnable(GL_DEPTH_TEST);
}
