
#include "pch.h"
#include "sysMoveByVelocity.h"
#include "compVoxelProxy.h"
#include "compScene.h"

void SysMoveByVelocity::Update(std::uint64_t dt, entt::registry &registry)
{
	registry.view<CompScene, CompVexelProxy>().each([dt](auto &scene, auto &vxl) {
		Location pos = scene.m_loc + scene.m_velocity * (float)dt;
		bool res = vxl.m_pxy->MoveTo(pos);
		scene.m_loc = vxl.m_pxy->GetLocation();
	});
}
