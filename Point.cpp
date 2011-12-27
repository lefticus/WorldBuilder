#include "Point.hpp"
#include <cmath>

Point::Point(double t_x, double t_y)
  : x(t_x), y(t_y)
{
}


double Point::distance(const Point &t_p1) const
{
  return hypot(x - t_p1.x, y - t_p1.y);
}

bool Point::operator>=(const Point &t_rhs) const
{
  return x >= t_rhs.x && y >= t_rhs.y;
}

bool Point::operator<=(const Point &t_rhs) const
{
  return x <= t_rhs.x && y <= t_rhs.y;
}




