#pragma once


class Vector3
{
public:
	float x, y, z;
	//构造函数
	//默认构造函数，初始一个零向量
	Vector3();
	//复制构造函数
	Vector3(const Vector3 &a);
	//带参数的构造函数，用三个值完成初始化
	Vector3(float nx, float ny, float nz);
	//析构函数
	~Vector3();

	//标准对象操作
	//重载赋值运算符，并返回引用，以实现左值。
	Vector3& operator=(const Vector3 &a);

	//重载"=="操作符
	bool operator==(const Vector3 &a) const;
	//重载"!="操作符
	bool operator!=(const Vector3 &a) const;

	//向量运算

	//置为零向量
	void Zero();
	//重载一元"-"运算符
	Vector3 operator-()const;

	//重载二元"+"和"-"运算符
	Vector3 operator+(const Vector3 &a) const;
	Vector3 operator-(const Vector3 &a) const;
	//标量的乘、除法
	Vector3 operator*(float a) const;
	Vector3 operator/(float a) const;

	//重载自反运算符
	Vector3& operator+=(const Vector3 &a);
	Vector3& operator-=(const Vector3 &a);
	Vector3& operator*=(float a);
	Vector3& operator/=(float a);

	//向量标准化
	void Normalize();

	//向量点乘，重载标准的乘法运算符
	float operator*(const Vector3 &a) const;

	//求向量模
	float VectorMag(const Vector3 &a);
	//计算两向量的叉乘
	static Vector3 CrossProduct(const Vector3 &a, const Vector3 &b);
	//计算两点间的距离
	static float Distance(const Vector3 &a, const Vector3 &b);
	//打印向量
	void PrintVector3();
};
