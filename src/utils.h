#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <string>


static const float PI = 3.14159265358979323846f;

template <typename T> inline bool fequal(T a, T b, T epsilon = 0.0001)
{
	if (a == b)
	{
		return true;
	}

	const T diff = std::abs(a - b);
	if (a * b == 0)
	{
		return diff < (epsilon * epsilon);
	}

	return diff / (std::abs(a) + std::abs(b)) < epsilon;
}

#endif // UTILS_H
