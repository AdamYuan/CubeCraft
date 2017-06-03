#include "Player.hpp"
#include "Game.hpp"

bool Player::hitTest(short face,bool doAction=true)
{
	Box movedPlayerBox(playerBox.Min + Position, playerBox.Max + Position);
	const int *mas=glm::value_ptr(static_cast<glm::ivec3>(
										  glm::floor(movedPlayerBox.Max - HITTEST_DELTA))),//Max search
			*mis=glm::value_ptr(static_cast<glm::ivec3>(
										glm::floor(movedPlayerBox.Min + HITTEST_DELTA)));//Min search
	const int search[2][3]={{mis[0],mis[1],mis[2]},{mas[0],mas[1],mas[2]}};
	bool use_max=!(face % 2);//is positive face ?
	short search_axis, i_axis, j_axis;
	search_axis = (short) (face / 2);//is face of x,y or z
	if(search_axis==0)
		i_axis=1, j_axis=2;
	else if(search_axis==1)
		i_axis=0, j_axis=2;
	else
		i_axis=0, j_axis=1;

	float backward=0.5f;
	if(doAction)
	{
		//calculate backward value
		if(search_axis==0)
		{
			if(use_max)
				backward+=std::fabs(movedPlayerBox.Min.x-Position.x);
			else
				backward+=std::fabs(movedPlayerBox.Max.x-Position.x);
		}
		else if(search_axis==1)
		{
			if(use_max)
				backward+=std::fabs(movedPlayerBox.Min.y-Position.y);
			else
				backward+=std::fabs(movedPlayerBox.Max.y-Position.y);
		}
		else
		{
			if(use_max)
				backward+=std::fabs(movedPlayerBox.Min.z-Position.z);
			else
				backward+=std::fabs(movedPlayerBox.Max.z-Position.z);
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
			if(Game::world.Voxels.GetBlock(block_pos)==Blocks::Air)
				continue;
			Box block_box;
			if(Funcs::Intersect(block_box = BlockMethods::GetBox(block_pos), movedPlayerBox))
			{
				if(doAction)
				{
					if(search_axis==0)
						Position.x= block_box.GetCenter().x+(use_max?1:-1)*backward;
					else if(search_axis==1)
						Position.y= block_box.GetCenter().y+(use_max?1:-1)*backward;
					else if(search_axis==2)
						Position.z= block_box.GetCenter().z+(use_max?1:-1)*backward;
				}
				return true;
			}
		}
	}
	return false;
}

bool Player::isFloating(float delta=2*HITTEST_DELTA)
{
	bool floating;
	Position.y-=delta;
	floating=!hitTest(TOP,false);
	Position.y+=delta;
	return floating;
}

bool Player::_moveForward(float dist, int degree)
{
	if(std::fabs(dist)>1.0f)//deal with fast movement
	{
		bool return_v=false;

		int sign= Funcs::Sign(dist);
		if(std::fabs(dist) > MAX_MOVE_DIST)//deal with too big movement
			dist=sign*MAX_MOVE_DIST;
		double integer;
		float decimal = (float) modf(std::fabs(dist), &integer);
		for(int c=0; c<integer; ++c)
			if(_moveForward(sign*1.0f, degree))
				return_v=true;
		if(decimal != 0.0f)
			if(_moveForward(sign*decimal, degree))
				return_v=true;
		return return_v;
	}
	bool return_v=false;

	float rad = glm::radians(Game::camera.Yaw + degree);
	glm::vec3 lastPos = Position;

	Position.x -= std::sin(rad) * dist;
	if(Position.x - lastPos.x != 0)
		if(hitTest(Position.x-lastPos.x>0?LEFT:RIGHT))
			return_v=true;

	Position.z -= std::cos(rad) * dist;
	if(Position.z-lastPos.z != 0)
		if(hitTest(Position.z-lastPos.z>0?BACK:FRONT))
			return_v=true;

	return return_v;
}

bool Player::_moveUp(float dist)
{
	if(std::fabs(dist)>1.0f)//deal with fast movement
	{
		bool return_v=false;

		int sign= Funcs::Sign(dist);
		if(std::fabs(dist) > MAX_MOVE_DIST)
			dist=sign*MAX_MOVE_DIST;
		double integer;
		float decimal = (float) modf(std::fabs(dist), &integer);
		for(int c=0;c<integer;++c)
			if(_moveUp(sign*1.0f))
				return_v=true;
		if(decimal != 0.0f)
			if(_moveUp(sign*decimal))
				return_v=true;
		return return_v;
	}

	Position.y+=dist;
	return hitTest(dist>0?BOTTOM:TOP);
}

bool Player::MoveForward(float dist, int degree)
{
	return _moveForward(Game::frameRateManager.GetMovementDistance(dist), degree);
}
bool Player::MoveUp(float dist)
{
	return _moveUp(Game::frameRateManager.GetMovementDistance(dist));
}

void Player::Jump()
{
	if(!isFloating())
	{
		lastGravityTime = glfwGetTime();
		jumping=true;
	}
}

void Player::Physics()
{
	static bool first_fall=false;
	//jumping
	if(jumping)
		jumping=!MoveUp(JUMP_STEP);

	//gravity
	if(!first_fall)
		MoveUp((float) (-GRAVITY * (glfwGetTime() - lastGravityTime)));

	bool floating=isFloating();

	first_fall=false;
	if(!floating)
	{
		lastGravityTime = glfwGetTime();
		jumping=false;
		first_fall=true;
	}
}

void Player::Update()
{
	if(PHYSICS)
		Physics();
	glm::ivec3 i3 = SuperChunk::GetChunkPos(Position);
	ChunkPos = i3;
	Game::camera.Position = Position;
}
