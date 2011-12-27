#ifndef WORLDBUIDLER_POINT_HPP
#define WORLDBUIDLER_POINT_HPP


struct Point
{
  Point(double t_x, double t_y);

  double distance(const Point &t_p1) const;

  bool operator>=(const Point &t_rhs) const;

  bool operator<=(const Point &t_rhs) const;



  double x;
  double y;
};

#endif

