// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
// #include "voxel.h"
// #include <sstream>
// #include <streambuf>
// #include "single_include/entt/entt.hpp"
//#include "Rand.h"
// #include "compScene.h"
// #include "compDest.h"
// #include "compVoxelProxy.h"



// void TestVoxel()
// {
// 	std::stringbuf sb;
// 	std::stringstream ss;
// 	{
// 		TerrainData terr(3, 3, 3);
// 		float spans[] = { 0.f, 100.f, 300.f };
// 		for (uint32_t i = 0; i < 3; ++i)
// 			for (uint32_t j = 0; j < 3; ++j)
// 				terr.AddVoxels(i, j, 2, spans);
// 		std::ostream os(&sb);
// 		terr.Export(os);
// 	}
// 	{
// 		TerrainData terr(3, 3, 3);
// 		std::istream is(&sb);
// 		terr.Import(is);
// 		auto& voxel = terr.GetVoxels(1, 1);
// 		auto layer = terr.GetLayer(voxel, 10.f);
// 		for (uint8_t dir = 0; dir <= uint8_t(Direction::LF); dir++)
// 		{
// 			uint8_t rel = (uint8_t)terr.GetNeighborLayerRelation(voxel, 1, Direction(dir));
// 			std::cout << rel << std::endl;
// 		}
// 		auto hight = terr.GetHight(voxel, 1);
// 		std::cout << layer << hight << std::endl;
// 	}
// 
// }
// 
// 
// void UpdateRandMove(entt::registry& registry)
// {
// 	registry.view<CompScene, CompDest>().each([](auto &pos, auto &dest) {
// 		pos
// 		if (dest.m_arrived)
// 		{
// 			dest.x = Rand::RandFloat(0.f, 145.f);
// 			dest.y = Rand::RandFloat(0.f, 145.f);
// 			dest.z = 10.f;
// 		}
// 
// 	});
// }
// 
// void TestECS()
// {
// 	entt::registry registry;
// 	std::uint64_t dt = 16;
// 
// 	for (auto i = 0; i < 10; ++i) {
// 		auto entity = registry.create();
// 		registry.assign<CompScene>(entity, i * 10.f, i * 10.f, 10.f);
// 		registry.assign<CompDest>(entity);
// 		registry.assign<CompVexelProxy>(entity);
// 	}
// 
// 	UpdateRandMove(registry);
// 	update(registry);
// 
// }

int main()
{
// 	TestVoxel();
    std::cout << "Hello World!\n"; 
}

