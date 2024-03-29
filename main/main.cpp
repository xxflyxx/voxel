// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "voxel.h"
#include <sstream>
#include <streambuf>
#include "single_include/entt/entt.hpp"
#include "utils/rand.h"
#include "utils/math.h"
#include "compScene.h"
#include "compDest.h"
#include "compVoxelProxy.h"
#include <thread>
#include <chrono>
#include "sysMoveByVelocity.h"



void TestVoxel()
{
	std::stringbuf sb;
	std::stringstream ss;
	{
		TerrainData terr(3, 3, 3);
		float spans[] = { 0.f, 100.f, 300.f };
		for (uint32_t i = 0; i < 3; ++i)
			for (uint32_t j = 0; j < 3; ++j)
				terr.AddVoxels(i, j, 2, spans);
		std::ostream os(&sb);
		terr.Export(os);
	}
	{
		TerrainData terr(3, 3, 3);
		std::istream is(&sb);
		terr.Import(is);
		auto& voxel = terr.GetVoxels(1, 1);
		auto layer = terr.GetLayer(voxel, 10.f);
		for (uint8_t dir = 0; dir <= uint8_t(Direction::LF); dir++)
		{
			uint8_t rel = (uint8_t)terr.GetNeighborLayerRelation(voxel, 1, Direction(dir));
			std::cout << rel << std::endl;
		}
		auto hight = terr.GetHight(voxel, 1);
		std::cout << layer << hight << std::endl;
	}

}


void UpdateRandMove(entt::registry& registry)
{
	registry.view<CompScene, CompDest>().each([](auto &pos, auto &dest) {
		//Math::Distance(pos, dest.m_loc) < 10.f
		if (dest.m_arrived)
		{
			dest.m_loc.x = Rand::RandFloat(0.f, 145.f);
			dest.m_loc.y = Rand::RandFloat(0.f, 145.f);
			dest.m_loc.z = 10.f;
		}

	});
}

void TestECS()
{
	TerrainData terr(3, 3, 3);
	float spans[] = { 0.f, 100.f, 300.f };
	for (uint32_t i = 0; i < 3; ++i)
		for (uint32_t j = 0; j < 3; ++j)
			terr.AddVoxels(i, j, 2, spans);
	terr.BuildNeighbor();

	TerrainInstance terr_ins(&terr);

	entt::registry registry;
	float dt = 0.016f;

	for (auto i = 0; i < 1; ++i) {
		auto entity = registry.create();
		registry.assign<CompScene>(entity, Location(1.f, 1.f, 1.f), Vector3(10.f, 0.f, 0.f));
		registry.assign<CompDest>(entity);
		registry.assign<CompVexelProxy>(entity, new VoxelProxy(&terr_ins, Location(1.f, 1.f, 1.f)));
	}

	for (;;)
	{
		//UpdateRandMove(registry);
		SysMoveByVelocity::Update(dt, registry);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

int main()
{
// 	TestVoxel();
	TestECS();
    std::cout << "Hello World!\n"; 
}

