#include "camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
glm::vec3 camera::position,camera::front;
float camera::yaw=0.0f,camera::pitch=0.0f;
void camera::lock()
{
	if(pitch>90.0f)
		pitch=90;
	else if(pitch<-90.0f)
		pitch=-90;
	if(yaw<0.0f)
		yaw+=360;
	else if(yaw>360.0f)
		yaw-=360;
}
glm::mat4 camera::getViewMatrix()
{
	glm::mat4 view;
	view=glm::rotate(view, glm::radians(-pitch), glm::vec3(1.0f,0.0f,0.0f));
	view=glm::rotate(view, glm::radians(-yaw), glm::vec3(0.0f,1.0f,0.0f));
	view=glm::translate(view, -position);

	return view;
}
void camera::moveForward(const float &dist, const float &dir)
{
	float rad=glm::radians(yaw+dir);
	position.x-=std::sin(rad)*dist;
	position.z-=std::cos(rad)*dist;
}
void camera::moveUp(const float &dist)
{
	position.y+=dist;
}
