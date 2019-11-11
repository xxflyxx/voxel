#pragma once
#include <random>

class Rand
{
public:
	/*
	** return a random integer in the interval [a, b]
	*/
	static int UniformRandInt(int a, int b)
	{
		static std::default_random_engine e{ std::random_device{}() }; // avoid "Most vexing parse"
		static std::uniform_int_distribution<int> u;

		return u(e, decltype(u)::param_type(a, b));
	}

	static int RandInt(int a, int b) { return UniformRandInt(a, b); }


	/*
	** return a random real in the interval [a, b] (also [a, b))
	*/
	static float UniformRandFloat(float a, float b) {
		static std::default_random_engine e{ std::random_device{}() };
		static std::uniform_real_distribution<float> u;

		return u(e, decltype(u)::param_type(a, b));
	}

	static float RandFloat(float a, float b) { return UniformRandFloat(a, b); }
};
