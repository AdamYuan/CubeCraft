#include "VertexObject.hpp"
#include <cstdarg>
namespace MyGL
{
	void VertexObject::init()
	{
		Elements = 0, DataNum = 0;
		inited = true;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
	}
	void VertexObject::clear()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}
	VertexObject::~VertexObject()
	{
		clear();
	}
	void VertexObject::beginRecord()
	{
		if(!inited)
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
		GLuint attributes[attr_count][2];
		va_start(vl, attr_count);
		for(int i=0;i<attr_count;++i)
		{
			GLuint attr_id=va_arg(vl, GLuint);
			unsigned size=va_arg(vl, unsigned);
			sum+=size;
			attributes[i][0]=attr_id;
			attributes[i][1]=size;
		}
		va_end(vl);
		AttributesLength=0;
		for(int i=0;i<attr_count;++i)
		{
			GLuint attr_id=attributes[i][0];
			int size=attributes[i][1];
			glVertexAttribPointer(attr_id, size, GL_FLOAT, GL_FALSE,
								  sum*sizeof(float), (void*)(AttributesLength*sizeof(float)));
			glEnableVertexAttribArray(attr_id);
			AttributesLength+=size;
		}
		endRecord();

		Elements = DataNum / AttributesLength;
	}
	void VertexObject::Render(GLenum mode)
	{
		if(Empty())
			return;
		glBindVertexArray(VAO);
		glDrawArrays(mode, 0, Elements);
		glBindVertexArray(0);
	}

	bool VertexObject::Empty() const
	{
		return Elements == 0;
	}
}
