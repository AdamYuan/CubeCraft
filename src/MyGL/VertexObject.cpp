#include "VertexObject.hpp"
#include <cstdarg>
namespace MyGL
{
	void VertexObject::init()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
	}
	void VertexObject::clear()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
	VertexObject::~VertexObject()
	{
		clear();
	}
	void VertexObject::beginRecord()
	{
		init();
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
	}
	void VertexObject::endRecord()
	{
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindVertexArray(0);
	}
	void VertexObject::SetAttributes(int attr_count, ...)
	{
		va_list vl;
		int sum=0;
		GLuint attribs[attr_count][2];
		va_start(vl, attr_count);
		for(int i=0;i<attr_count;++i)
		{
			GLuint attr_id=va_arg(vl, GLuint);
			unsigned size=va_arg(vl, unsigned);
			sum+=size;
			attribs[i][0]=attr_id;
			attribs[i][1]=size;
		}
		va_end(vl);
		int now=0;
		for(int i=0;i<attr_count;++i)
		{
			GLuint attr_id=attribs[i][0];
			int size=attribs[i][1];
			glVertexAttribPointer(attr_id, size, GL_FLOAT, GL_FALSE,
								  sum*sizeof(float), (void*)(now*sizeof(float)));
			glEnableVertexAttribArray(attr_id);
			now+=size;
		}
		elements=dataNum/now;

		endRecord();
	}
	void VertexObject::Render(GLenum mode)
	{
		glBindVertexArray(VAO);
		glDrawArrays(mode, 0, elements);
		glBindVertexArray(0);
	}

	bool VertexObject::Empty() const
	{
		return elements == 0;
	}
}
