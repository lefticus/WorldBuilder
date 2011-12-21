#ifndef WORLDBUIDLER_POINT_HPP
#define WORLDBUIDLER_POINT_HPP

#include <cmath>

struct Point
{
  Point(double t_x, double t_y)
    : x(t_x), y(t_y)
  {
  }


  double distance(Point t_p1)
  {
    return hypot(x - t_p1.x, y - t_p1.y);
  }

  bool operator>=(Point t_rhs) const
  {
    return x >= t_rhs.x && y >= t_rhs.y;
  }

  bool operator<=(Point t_rhs) const
  {
    return x <= t_rhs.x && y <= t_rhs.y;
  }



  double x;
  double y;
};

#endif

