// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "voxel.h"
#include <sstream>
#include <streambuf>

void testVoxel()
{
	std::stringbuf sb;
	std::stringstream ss;
	{
		TerrainData terr(3, 3, 3);
		float spans[] = { 0.f, 20.f, 50.f };
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
		auto hight = terr.GetHight(voxel, 1);
		std::cout << layer << hight << std::endl;
	}

}


int main()
{
	testVoxel();
    std::cout << "Hello World!\n"; 
}

