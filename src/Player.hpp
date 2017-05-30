#pragma once
#include "Util.hpp"
class Player
{
private:
	Box playerBox=Box(glm::vec3(-0.3f,-1.6f,-0.3f), glm::vec3(0.3f));
	//if Player hit the world, then return true
	bool hitTest(short face,bool doAction);//face is the world face to collision by the Player
	bool _moveForward(float dist,int degree);
	bool _moveUp(float dist);
	bool isFloating(float delta);
	bool jumping=false;
	double lastGravityTime;
public:
	glm::vec3 Position=glm::vec3(0);
	glm::ivec3 ChunkPos=glm::ivec3(INT_MAX);
	bool MoveForward(float dist, int degree);
	bool MoveUp(float dist);
	void Jump();
	void Physics();
	void Update();
};
