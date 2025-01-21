#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>

class Vector2 {
public:
  float x = 0;
  float y = 0;

public:
  Vector2() = default;

  ~Vector2() = default;

  Vector2(float x, float y) : x(x), y(y) {}

  Vector2 operator+(const Vector2 &v) const {
    return Vector2(x + v.x, y + v.y);
  }
  void operator+=(const Vector2 &v) {
    x += v.x;
    y += v.y;
  }
  void operator-=(const Vector2 &v) {
    x -= v.x;
    y -= v.y;
  }
  Vector2 operator-(const Vector2 &v) const {
    return Vector2(x - v.x, y - v.y);
  }

  Vector2 operator*(float scalar) const {
    return Vector2(x * scalar, y * scalar);
  }
  Vector2 operator*=(float scalar) const {
    return Vector2(x * scalar, y * scalar);
  }
  Vector2 operator/=(float scalar) const {
    return Vector2(x / scalar, y / scalar);
  }
  float operator*(const Vector2 &v) const { return x * v.x + y * v.y; }
  Vector2 operator/(float scalar) const {
    return Vector2(x / scalar, y / scalar);
  }
  float length() const { return sqrt(x * x + y * y); }

  Vector2 normalize() const {
    float len = length();
    if (len == 0)
      return Vector2(0, 0);
    return Vector2(x / len, y / len);
  }

  bool approx(const Vector2 &v) { return (*this - v).length() < 0.0001f; }
};

#endif