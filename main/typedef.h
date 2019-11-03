#pragma once

struct Vector2
{
	float x, y;
};

struct Vector3
{
	float x, y, z;
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
