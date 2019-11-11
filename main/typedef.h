#pragma once

#include "vector3.h"

struct Vector2
{
	float x, y;
};



struct Rotation
{
	float pitch;
	float yaw;
	float roll;
};

struct Location : Vector3
{

};

class Transform
{
public:
	Location loc;
	Rotation rot;
	//Vector3 scale;
};
