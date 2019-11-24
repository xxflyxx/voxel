
#include "pch.h"
#include "sysMoveByVelocity.h"
#include "compVoxelProxy.h"
#include "compScene.h"

void SysMoveByVelocity::Update(float dt, entt::registry &registry)
{
	registry.view<CompScene, CompVexelProxy>().each([dt](auto &scene, auto &vxl) {
		Location pos = scene.m_loc + scene.m_velocity * dt;
		bool res = vxl.m_pxy->MoveTo(pos);
		scene.m_loc = vxl.m_pxy->GetLocation();
		std::cout << scene.m_loc.x << scene.m_loc.y << scene.m_loc.z << std::endl;
	});
}
