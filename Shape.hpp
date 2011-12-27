#ifndef WORLDBUILDER_SHAPE
#define WORLDBUILDER_SHAPE

#include "Point.hpp"

#include <vector>
#include <random>

class Region;

class Shape
{
  public:
    Shape(const Region &t_region, std::mt19937 &t_engine);

    bool contains(const Point &t_p) const;

  private:
    struct Circle
    {
      Circle(const Point &t_center, double t_radius);

      Point center;
      double radius;

      bool contains(const Point &t_p) const;
    };

    std::vector<Circle> m_circles;
};

#endif

