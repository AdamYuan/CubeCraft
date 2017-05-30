#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace MyGL
{
	class Frustum
	{
	private:
		float planes[6][4];

	public:
		void CalculatePlanes(glm::mat4 mvp);

		bool PointInFrustum(float x, float y, float z);
		bool PointInFrustum(glm::vec3 pos);

		bool SphereInFrustum(float x, float y, float z, float rad);
		bool SphereInFrustum(glm::vec3 pos, float rad);

		bool CubeInFrustum(float x, float y, float z, float size);
		bool CubeInFrustum(glm::vec3 pos, float size);
	};
}
