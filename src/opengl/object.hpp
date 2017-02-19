#pragma once
#include <vector>
#include <GL/glew.h>
class object
{
private:
	bool inited=false;
	void init();
	void clear();
public:
	GLuint VAO,VBO,elements,dataNum;
	~object();
	void beginRecord();

	template<class T>
	void setDataVec(std::vector<T> vec);

	template<class T>
	void setDataArr(const T *array, int arrSize);

	void setAttribs(int attr_count, ...);
	void endRecord();
	void render(GLenum mode);
};
template<class T>
void object::setDataVec(std::vector<T> vec)
{
	if(vec.empty())
		return;
	glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(T), &vec[0], GL_STATIC_DRAW);
	dataNum= (GLuint) (vec.size() * (sizeof(T) / sizeof(float)));
}
template<class T>
void object::setDataArr(const T *array, int arrSize)
{
	if(arrSize==0)
		return;
	glBufferData(GL_ARRAY_BUFFER,arrSize*sizeof(T),array,GL_STATIC_DRAW);
	dataNum=arrSize*(sizeof(T)/sizeof(float));
}
