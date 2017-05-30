#pragma once
#include <vector>
#include <GL/glew.h>
namespace MyGL
{
	class VertexObject
	{
	private:
		GLuint VAO, VBO, elements, dataNum;
		void init();
		void clear();
		void beginRecord();
		void endRecord();
	public:
		~VertexObject();

		template<class T>
		void SetDataVec(std::vector<T> vec);

		template<class T>
		void SetDataArr(const T *array, int arrSize);

		void SetAttributes(int attr_count, ...);
		void Render(GLenum mode);

		bool Empty() const;
	};
	template<class T>
	void VertexObject::SetDataVec(std::vector<T> vec)
	{
		if(vec.empty())
			return;
		beginRecord();
		glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(T), &vec[0], GL_STATIC_DRAW);
		dataNum = (GLuint) (vec.size() * (sizeof(T) / sizeof(float)));
	}
	template<class T>
	void VertexObject::SetDataArr(const T *array, int arrSize)
	{
		if(arrSize==0)
			return;
		beginRecord();
		glBufferData(GL_ARRAY_BUFFER,arrSize*sizeof(T),array,GL_STATIC_DRAW);
		dataNum = arrSize*(sizeof(T)/sizeof(float));
	}
}
