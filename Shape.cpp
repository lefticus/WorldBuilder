#include "Shape.hpp"
#include "Region.hpp"
#include "Point.hpp"

Shape::Circle::Circle(const Point &t_center, double t_radius)
  : center(t_center), radius(t_radius)
{
}

bool Shape::Circle::contains(const Point &t_p) const
{
  return t_p.distance(center) <= radius;
}

Shape::Shape(const Region &t_region, std::mt19937 &t_engine)
{
  double max_radius = std::min(t_region.width(), t_region.height());

  int num_circles = std::uniform_int_distribution<int>(3, 6)(t_engine);
  std::uniform_real_distribution<double> radius(0, max_radius);

  for (int i = 0; i < num_circles; ++i)
  {
    Point p = t_region.choose_point(t_engine);
    double r = radius(t_engine);

    m_circles.push_back(Circle(p, r));
  }
}


bool Shape::contains(const Point &t_p) const
{
  for (const Circle &circle: m_circles)
  {
    if (circle.contains(t_p))
    {
      return true;
    }
  }

  return false;
}


