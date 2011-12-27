#ifndef WORLDBUILDER_REGION
#define WORLDBUILDER_REGION

#include "Point.hpp"

#include <vector>
#include <random>

enum Location
{
  NorthEast,
  North,
  NorthWest,
  East,
  Central,
  West,
  SouthEast,
  South,
  SouthWest
};


class Region
{
  public:
    Region(double t_width, double t_height);
    Region(const Point &t_p1, const Point &t_p2);
    Region(const Point &t_p1, double t_width, double t_height);

    Region get_location(Location t_location) const;

    std::vector<Region> subdivide(int t_horizontal, int t_vertical) const;

    double width() const;
    double height() const;
    Point top_left() const;
    Point bottom_right() const;

    bool contains(const Point &t_p) const;
    Point choose_point(std::mt19937 &t_engine) const;

  private:
    Point m_p1;
    Point m_p2;
};

#endif

