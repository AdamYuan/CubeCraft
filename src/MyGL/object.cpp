#include "object.hpp"
#include <vector>
#include <cstdarg>
#include <iostream>
#include <GL/glew.h>
void object::init()
{
	if(inited)
		return;
	inited=true;
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
}
void object::clear()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
}
object::~object()
{
	clear();
}
void object::beginRecord()
{
	init();
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
}
void object::endRecord()
{
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
}
void object::setAttribs(int attr_count, ...)
{
	va_list vl;
	int sum=0;
	int attribs[attr_count][2];
	va_start(vl, attr_count);
	for(int i=0;i<attr_count;++i)
	{
		GLint attr_id=va_arg(vl, GLint);
		int size=va_arg(vl, int);
		sum+=size;
		attribs[i][0]=attr_id;
		attribs[i][1]=size;
	}
	va_end(vl);
	int now=0;
	for(int i=0;i<attr_count;++i)
	{
		GLint attr_id=attribs[i][0];
		int size=attribs[i][1];
		glVertexAttribPointer(attr_id,size,GL_FLOAT,GL_FALSE,
							  sum*sizeof(float),(void*)(now*sizeof(float)));
		glEnableVertexAttribArray(attr_id);
		now+=size;
	}
	elements=dataNum/now;

	endRecord();
}
void object::render(GLenum mode)
{
	glBindVertexArray(VAO);
	glDrawArrays(mode, 0, elements);
	glBindVertexArray(0);
}
