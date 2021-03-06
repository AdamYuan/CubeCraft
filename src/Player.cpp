#include "Player.hpp"
#include "Game.hpp"

bool Player::hitTest(short face, bool doAction=true)
{
	Box movedPlayerBox(PlayerBox.Min + Position, PlayerBox.Max + Position);
	const int *mas=glm::value_ptr(static_cast<glm::ivec3>(
										  glm::floor(movedPlayerBox.Max - HITTEST_DELTA))),//Max search
			*mis=glm::value_ptr(static_cast<glm::ivec3>(
										glm::floor(movedPlayerBox.Min + HITTEST_DELTA)));//Min search
	const int search[2][3]={{mis[0],mis[1],mis[2]},{mas[0],mas[1],mas[2]}};
	bool use_max = !(face % 2);//is positive face ?
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
			if(!BlockUtil::HaveHitbox(Game::world.Voxels.GetBlock(block_pos)))
				continue;
			Box block_box;
			if(Util::Intersect(block_box = BlockUtil::GetBox(block_pos), movedPlayerBox))
			{
				if(doAction)
				{
					if(search_axis==0)
						Position.x = block_box.GetCenter().x+(use_max?1:-1)*backward;
					else if(search_axis==1)
						Position.y = block_box.GetCenter().y+(use_max?1:-1)*backward;
					else if(search_axis==2)
						Position.z = block_box.GetCenter().z+(use_max?1:-1)*backward;
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
	if(PHYSICS && std::fabs(dist)>1.0f)//deal with fast movement
	{
		bool return_v=false;

		int sign= Util::Sign(dist);
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
	if(PHYSICS && Position.x - lastPos.x != 0)
		if(hitTest(Position.x-lastPos.x>0?LEFT:RIGHT))
			return_v=true;

	Position.z -= std::cos(rad) * dist;
	if(PHYSICS && Position.z-lastPos.z != 0)
		if(hitTest(Position.z-lastPos.z>0?BACK:FRONT))
			return_v=true;

	return return_v;
}

bool Player::_moveUp(float dist)
{
	if(PHYSICS && std::fabs(dist)>1.0f)//deal with fast movement
	{
		bool return_v=false;

		int sign= Util::Sign(dist);
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
	if(PHYSICS)
		return hitTest(dist>0?BOTTOM:TOP);
	return false;
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

void Player::UpdatePosition()
{
	if(PHYSICS)
		Physics();
	glm::ivec3 i3 = SuperChunk::GetChunkPos(Position);
	ChunkPos = i3;
	Game::camera.Position = Position;
}

void Player::StartTimer()
{
	lastGravityTime = glfwGetTime();
}

inline float intBound(float s, float ds) {
	bool sIsInteger = glm::round(s) == s;
	if (ds < 0 && sIsInteger)
		return 0;
	return (ds > 0 ? (s == 0.0f ? 1.0f : glm::ceil(s)) - s : s - glm::floor(s)) / glm::abs(ds);
}
void Player::UpdateSelectedPosition()
{
	float radius = 10.0f;

	glm::vec3 origin = Position;
	// From "A Fast Voxel Traversal Algorithm for Ray Tracing"
	// by John Amanatides and Andrew Woo, 1987
	// <http://www.cse.yorku.ca/~amana/research/grid.pdf>
	// <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.42.3443>
	// Extensions to the described algorithm:
	//   • Imposed a distance limit.
	//   • The face passed through to reach the current cube is provided to
	//     the callback.

	// The foundation of this algorithm is a parameterized representation of
	// the provided ray,
	//                    origin + t * direction,
	// except that t is not actually stored; rather, at any given point in the
	// traversal, we keep track of the *greater* t values which we would have
	// if we took a step sufficient to cross a cube boundary along that axis
	// (i.e. change the integer part of the coordinate) in the variables
	// tMaxX, tMaxY, and tMaxZ.

	// Cube containing origin point.
	glm::vec3 xyz = glm::floor(origin);
	// Break out direction vector.
	glm::vec3 direction = glm::unProject(glm::vec3(Game::Width / 2.0f, Game::Height / 2.0f, 1.0f),
									glm::mat4(), Game::matrices.Projection3d * Game::camera.GetViewMatrix(),
									glm::vec4(0.0f, 0.0f, (float)Game::Width, (float)Game::Height)
	) - origin;
	// Direction to increment x,y,z when stepping.
	glm::vec3 step = glm::sign(direction);
	// See description above. The initial values depend on the fractional
	// part of the origin.
	glm::vec3 tMax = glm::vec3(intBound(origin.x, direction.x),
							   intBound(origin.y, direction.y),
							   intBound(origin.z, direction.z));
	// The change in t when taking a step (always positive).
	glm::vec3 tDelta = step / direction;
	// Buffer for reporting faces to the callback.
	glm::vec3 face;

	// Rescale from units of 1 cube-edge to units of 'direction' so we can
	// compare with 't'.
	radius /= glm::sqrt(direction.x*direction.x + direction.y*direction.y + direction.z*direction.z);

	while (true)
	{
		// Invoke the callback, unless we are not *yet* within the bounds of the
		// world.
		if (BlockUtil::HaveHitbox(Game::world.Voxels.GetBlock(xyz))) {
			SelectedPosition = xyz;
			SelectedFaceVec = face;
			return;
		}

		// tMaxX stores the t-value at which we cross a cube boundary along the
		// X axis, and similarly for Y and Z. Therefore, choosing the least tMax
		// chooses the closest cube boundary. Only the first case of the four
		// has been commented in detail.
		if (tMax.x < tMax.y) {
			if (tMax.x < tMax.z) {
				if (tMax.x > radius) break;
				// Update which cube we are now in.
				xyz.x += step.x;
				// Adjust tMaxX to the next X-oriented boundary crossing.
				tMax.x += tDelta.x;
				// Record the normal vector of the cube face we entered.
				face[0] = -step.x;
				face[1] = 0;
				face[2] = 0;
			} else {
				if (tMax.z > radius) break;
				xyz.z += step.z;
				tMax.z += tDelta.z;
				face[0] = 0;
				face[1] = 0;
				face[2] = -step.z;
			}
		} else {
			if (tMax.y < tMax.z) {
				if (tMax.y > radius) break;
				xyz.y += step.y;
				tMax.y += tDelta.y;
				face[0] = 0;
				face[1] = -step.y;
				face[2] = 0;
			} else {
				// Identical to the second case, repeated for simplicity in
				// the conditionals.
				if (tMax.z > radius) break;
				xyz.z += step.z;
				tMax.z += tDelta.z;
				face[0] = 0;
				face[1] = 0;
				face[2] = -step.z;
			}
		}
	}

	SelectedPosition = glm::ivec3(INT_MAX);
}
