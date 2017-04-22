#include "player.hpp"
#include "game.hpp"
#include "MyGL/camera.hpp"
#include "MyGL/framerate.hpp"
#include <cmath>

bool player::hitTest(short face,bool doAction=true)
{
	box pbox(playerBox.min+position,playerBox.max+position);
	const int *mas=glm::value_ptr(static_cast<glm::ivec3>(glm::floor(pbox.max-HITTEST_DELTA))),//max search
			*mis=glm::value_ptr(static_cast<glm::ivec3>(glm::floor(pbox.min+HITTEST_DELTA)));//min search
	const int search[2][3]={{mis[0],mis[1],mis[2]},{mas[0],mas[1],mas[2]}};
	bool use_max=!(face%2);//is positive face ?
	short search_axis=face/2,i_axis,j_axis;//is face of x,y or z
	if(search_axis==0)
		i_axis=1,j_axis=2;
	else if(search_axis==1)
		i_axis=0,j_axis=2;
	else
		i_axis=0,j_axis=1;

	float backword=0.5f;
	if(doAction)
	{
		//calculate backword value
		if(search_axis==0)
		{
			if(use_max)
				backword+=std::fabs(pbox.min.x-position.x);
			else
				backword+=std::fabs(pbox.max.x-position.x);
		}
		else if(search_axis==1)
		{
			if(use_max)
				backword+=std::fabs(pbox.min.y-position.y);
			else
				backword+=std::fabs(pbox.max.y-position.y);
		}
		else
		{
			if(use_max)
				backword+=std::fabs(pbox.min.z-position.z);
			else
				backword+=std::fabs(pbox.max.z-position.z);
		}
	}

	for(int i=search[0][i_axis];i<=search[1][i_axis];++i)
	{
		for(int j=search[0][j_axis];j<=search[1][j_axis];++j)
		{
			glm::ivec3 block_pos;
			if(search_axis==0)//x
			{
				block_pos.x=search[!use_max][0];
				block_pos.y=i,block_pos.z=j;
			}
			else if(search_axis==1)//y
			{
				block_pos.y=search[!use_max][1];
				block_pos.x=i,block_pos.z=j;
			}
			else//z
			{
				block_pos.z=search[!use_max][2];
				block_pos.x=i,block_pos.y=j;
			}
			if(game::wld.voxels.getBlock(block_pos)==blocks::air)
				continue;
			box block_box;
			if(funcs::intersect(block_box=block_m::getBox(block_pos), pbox))
			{
				if(doAction)
				{
					if(search_axis==0)
						position.x=block_box.getCenter().x+(use_max?1:-1)*backword;
					else if(search_axis==1)
						position.y=block_box.getCenter().y+(use_max?1:-1)*backword;
					else if(search_axis==2)
						position.z=block_box.getCenter().z+(use_max?1:-1)*backword;
				}
				return true;
			}
		}
	}
	return false;
}

bool player::isFloating(float delta=2*HITTEST_DELTA)
{
	bool floating;
	position.y-=delta;
	floating=!hitTest(TOP,false);
	position.y+=delta;
	return floating;
}

bool player::_moveForward(float dist, int degree)
{
	if(std::fabs(dist)>1.0f)//deal with fast movement
	{
		bool return_v=false;

		int sign=funcs::sgn(dist);
		if(std::fabs(dist) > MAX_MOVE_DIST)//deal with too big movement
			dist=sign*MAX_MOVE_DIST;
		double interg;
		float decimal=modf(std::fabs(dist),&interg);
		for(int c=0;c<interg;++c)
			if(_moveForward(sign*1.0f,degree))
				return_v=true;
		if(decimal != 0.0f)
			if(_moveForward(sign*decimal,degree))
				return_v=true;
		return return_v;
	}
	bool return_v=false;

	float rad=glm::radians(camera::yaw+degree);
	glm::vec3 lastPos=position;

	position.x-=std::sin(rad)*dist;
	if(position.x-lastPos.x!=0)
		if(hitTest(position.x-lastPos.x>0?LEFT:RIGHT))
			return_v=true;

	position.z-=std::cos(rad)*dist;
	if(position.z-lastPos.z!=0)
		if(hitTest(position.z-lastPos.z>0?BACK:FRONT))
			return_v=true;

	return return_v;
}

bool player::_moveUp(float dist)
{
	if(std::fabs(dist)>1.0f)//deal with fast movement
	{
		bool return_v=false;

		int sign=funcs::sgn(dist);
		if(std::fabs(dist) > MAX_MOVE_DIST)
			dist=sign*MAX_MOVE_DIST;
		double interg;
		float decimal=modf(std::fabs(dist),&interg);
		for(int c=0;c<interg;++c)
			if(_moveUp(sign*1.0f))
				return_v=true;
		if(decimal != 0.0f)
			if(_moveUp(sign*decimal))
				return_v=true;
		return return_v;
	}
	//dist=framerate::getDist(dist);

	position.y+=dist;
	return hitTest(dist>0?BOTTOM:TOP);
}

bool player::moveForward(float dist,int degree)
{
	return _moveForward(framerate::getDist(dist), degree);
}
bool player::moveUp(float dist)
{
	return _moveUp(framerate::getDist(dist));
}

void player::jump()
{
	if(!isFloating())
	{
		gravity_timer.restart();
		jumping=true;
		last_pos=position;
	}
}

void player::physics()
{
	static bool first_fall=false;
	//jumping
	if(jumping)
		jumping=!moveUp(JUMP_STEP);

	//gravity
	if(!first_fall)
		moveUp(-GRAVITY * gravity_timer.getElapsedTime().asSeconds());

	bool floating=isFloating();

	first_fall=false;
	if(!floating)
	{
		gravity_timer.restart();
		jumping=false;
		first_fall=true;
	}
}

void player::update()
{
	if(PHYSICS)
		physics();
	glm::ivec3 i3 = superChunk::getChunkPos(position);
	chunkPos = i3;
	camera::position=position;
}
