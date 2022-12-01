#ifndef VECTOR_H
#define VECTOR_H
#include "compose_keys.hxx"
#include <cmath>

#define DEBUG

STRUCT(iVector)
{
public:
  int x;
  int y;
  [[gnu::always_inline]] constexpr   inline iVector operator+(const iVector rhs) const;
  [[gnu::always_inline]] constexpr   inline iVector operator-(const iVector rhs) const;
  [[gnu::always_inline]] constexpr   inline iVector operator*(const iVector rhs) const;
  [[gnu::always_inline]] constexpr   inline iVector operator*(int rhs) const;
  [[gnu::always_inline]] constexpr   inline iVector operator/(const iVector rhs) const;
  [[gnu::always_inline]] constexpr   inline iVector operator/(int rhs) const;
  [[gnu::always_inline]] constexpr   inline int abs() const;
  [[gnu::always_inline]] constexpr   inline iVector norm() const;
  [[gnu::always_inline]] inline static iVector random(int x, int y);
};

constexpr  iVector iVector::operator+(const iVector rhs) const {
	iVector res{.x = x + rhs.x, 
		   .y = y + rhs.y};
	return res;
}
constexpr  iVector iVector::operator-(const iVector rhs) const {
	iVector res{.x = x - rhs.x, 
		   .y = y - rhs.y};
	return res;
}
constexpr  iVector iVector::operator*(const iVector rhs) const {
	iVector res{.x = x * rhs.x, 
		   .y = y * rhs.y};
	return res;
}
constexpr  iVector iVector::operator*(int rhs) const {
	iVector res{.x = x * rhs, 
		   .y = y * rhs};
	return res;
}
constexpr  iVector iVector::operator/(const iVector rhs) const {
	iVector res{.x = x / rhs.x, 
		   .y = y / rhs.y};
	return res;
}
constexpr  iVector iVector::operator/(int rhs) const {
	iVector res{.x = x / rhs, 
		   .y = y / rhs};
	return res;
}
constexpr  int iVector::abs() const {
	return sqrt((long long int)x*(long long int)x+(long long int)y*(long long int)y);
}
constexpr  iVector iVector::norm() const {
	int d = std::max(abs(),1);
	return *this*10000/d;
}
iVector iVector::random(int x, int y) {
	iVector res{.x = rand()%(2*x)-x, 
		   .y = rand()%(2*y)-y};
	return res;
}



STRUCT(Vector)
{
  float x;
  float y;
  [[gnu::always_inline]] constexpr   inline Vector operator+(const Vector rhs) const;
  [[gnu::always_inline]] constexpr   inline Vector operator-(const Vector rhs) const;
  [[gnu::always_inline]] constexpr   inline Vector operator*(const Vector rhs) const;
  [[gnu::always_inline]] constexpr   inline Vector operator*(float rhs) const;
  [[gnu::always_inline]] constexpr   inline Vector operator/(const Vector rhs) const;
  [[gnu::always_inline]] constexpr   inline Vector operator/(float rhs) const;
  [[gnu::always_inline]] constexpr   inline float abs() const;
  [[gnu::always_inline]] constexpr   inline Vector norm() const;
  [[gnu::always_inline]] inline static Vector random(float x, float y);
};

constexpr  Vector Vector::operator+(const Vector rhs) const {
	Vector res{.x = x + rhs.x, 
		   .y = y + rhs.y};
	return res;
}
constexpr  Vector Vector::operator-(const Vector rhs) const {
	Vector res{.x = x - rhs.x, 
		   .y = y - rhs.y};
	return res;
}
constexpr  Vector Vector::operator*(const Vector rhs) const {
	Vector res{.x = x * rhs.x, 
		   .y = y * rhs.y};
	return res;
}
constexpr  Vector Vector::operator*(float rhs) const {
	Vector res{.x = x * rhs, 
		   .y = y * rhs};
	return res;
}
constexpr  Vector Vector::operator/(const Vector rhs) const {
	Vector res{.x = x / rhs.x, 
		   .y = y / rhs.y};
	return res;
}
constexpr  Vector Vector::operator/(float rhs) const {
	Vector res{.x = x / rhs, 
		   .y = y / rhs};
	return res;
}
constexpr  float Vector::abs() const {
	return sqrt(x*x+y*y);
}
constexpr  Vector Vector::norm() const {
	float d = fmaxf(abs(),1.f);
	return *this/d;
}
Vector Vector::random(float x, float y) {
	Vector res{.x = x*(float)rand()/RAND_MAX, 
		   .y = y*(float)rand()/RAND_MAX};
	return res;
}

#endif // VECTOR_H

