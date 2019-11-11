#include "pch.h"
#include <math.h>
#include "vector3.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
//#include <cmath>
// #include <cfloat>
//#include <iostream>

using namespace std;

//默认构造函数,初始一个零向量
Vector3::Vector3() {
	x = 0;
	y = 0;
	z = 0;
}
//复制构造函数
Vector3::Vector3(const Vector3 &a) {
	x = a.x;
	y = a.y;
	z = a.z;
}
//带参数的构造函数，用三个值完成初始化
Vector3::Vector3(float nx, float ny, float nz) {
	x = nx;
	y = ny;
	z = nz;
}
//析构函数
Vector3::~Vector3() {};

//标准对象操作
//重载赋值运算符，并返回引用，以实现左值。
Vector3& Vector3::operator=(const Vector3 &a) {
	x = a.x;
	y = a.y;
	z = a.z;
	return *this;
}

//重载"=="操作符
bool Vector3::operator==(const Vector3 &a) const {
	return x == a.x && y == a.y && z == a.z;
}

//重载"!="操作符
bool Vector3::operator!=(const Vector3 &a) const {
	return x != a.x || y != a.y || z != a.z;
}

//置为零向量
void Vector3::Zero() {
	x = y = z = 0.0f;
}
//重载一元"-"运算符
Vector3 Vector3::operator-()const
{
	return Vector3(-x, -y, -z);
}

//重载二元"+"和"-"运算符
Vector3 Vector3::operator+(const Vector3 &a) const {

	return Vector3(x + a.x, y + a.y, z + a.z);
}
Vector3 Vector3::operator-(const Vector3 &a) const {
	return Vector3(x - a.x, y - a.y, z - a.z);
}

//标量的乘、除法
Vector3 Vector3::operator*(float a) const {
	return Vector3(x*a, y*a, z*a);

}
Vector3 Vector3::operator/(float a) const {
	float oneOverA = 1.0f / a;//注意这里不对"除零"进行处理
	return Vector3(x*oneOverA, y*oneOverA, z*oneOverA);
}

//重载自反运算符
Vector3& Vector3::operator +=(const Vector3 &a) {

	x += a.x;
	y += a.y;
	z += a.z;
	return *this;
}

Vector3& Vector3::operator -=(const Vector3 &a) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	return *this;
}
Vector3& Vector3::operator *=(float a) {
	x *= a;
	y *= a;
	z *= a;
	return *this;
}
Vector3& Vector3::operator /=(float a) {
	float oneOverA = 1.0f / a;
	x *= oneOverA;
	y *= oneOverA;
	z *= oneOverA;
	return *this;
}

//向量标准化
void Vector3::Normalize() {
	float magSq = x * x + y * y + z * z;
	if (magSq > 0.0f) {//检查除零
		float oneOverMag = 1.0f / sqrt(magSq);
		x *= oneOverMag;
		y *= oneOverMag;
		z *= oneOverMag;
	}
}

//向量点乘，重载标准的乘法运算符
float Vector3::operator *(const Vector3 &a) const {
	return x * a.x + y * a.y + z * a.z;
}
void Vector3::PrintVector3() {
	std::cout << "(" << x << "," << y << "," << z << ")" << std::endl;

}
//求向量模
float Vector3::VectorMag(const Vector3 &a) {
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

/*
Cross Product叉乘公式
aXb = | i   j   k  |
	  | a.x a.y a.z|
	  | b.x b.y b.z| = (a.y*b.z -a.z*b.y)i + (a.z*b.x - a.x*b.z)j + (a.x+b.y - a.y*b.x)k
*/

//计算两向量的叉乘
Vector3 Vector3::CrossProduct(const Vector3 &a, const Vector3 &b) {
	return Vector3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

//计算两点间的距离
float Vector3::Distance(const Vector3 &a, const Vector3 &b) {

	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}
————————————————
版权声明：本文为CSDN博主「gongchao1212」的原创文章，遵循 CC 4.0 BY - SA 版权协议，转载请附上原文出处链接及本声明。
原文链接：https ://blog.csdn.net/shi_tou_ge/article/details/45576869