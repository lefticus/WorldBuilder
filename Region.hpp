#ifndef WORLDBUILDER_REGION
#define WORLDBUILDER_REGION

#include "Point.hpp"

#include <cassert>

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


struct Region
{
  Region get_location(Location t_location)
  {
    std::vector<Region> regions = subdivide(3,3);

    assert(regions.size() == 9u && "Unexpected number of regions!");

    switch (t_location)
    {
      case NorthWest:
        return regions[0];
      case North:
        return regions[1];
      case NorthEast:
        return regions[2];
      case West:
        return regions[3];
      case Central:
        return regions[4];
      case East:
        return regions[5];
      case SouthWest:
        return regions[6];
      case South:
        return regions[7];
      case SouthEast:
        return regions[8];
    }
    assert(!"Unknown location type");
  }

  std::vector<Region> subdivide(int t_horizontal, int t_vertical)
  {
    std::vector<Region> regions;

    double regionwidth = width();
    double regionheight = height();

    Point topleft = top_left();

    for (int h = 0; h < t_vertical; ++h)
    {
      for (int w = 0; w < t_horizontal; ++w)
      {
        regions.push_back(
            {
              { regionwidth * w / t_horizontal + topleft.x, regionheight * h / t_vertical + topleft.y },
              { regionwidth * (w + 1) / t_horizontal + topleft.x, regionheight * (h + 1) / t_vertical + topleft.y }
            }
            );

      }
    }



    return regions;
  }

  Region(double t_width, double t_height)
    : p1(0,0), p2(t_width, t_height)
  {
  }

  Region(Point t_p1, Point t_p2)
    : p1(t_p1), p2(t_p2)
  {
  }

  Region(Point t_p1, double t_width, double t_height)
    : p1(t_p1), p2(p1.x + t_width, p1.y + t_height)
  {
  }

  double width() const
  {
    return fabs(p1.x - p2.x);
  }

  double height() const
  {
    return fabs(p1.y - p2.y);
  }

  Point top_left() const
  {
    return Point(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
  }

  Point bottom_right() const
  {
    return Point(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
  }


  bool contains(Point t_p) const
  {
    return t_p >= top_left() && t_p <= bottom_right();
  }

  Point choose_point(std::mt19937 &t_engine) const
  {
    std::uniform_real_distribution<double> xdistribution(std::min(p1.x, p2.x), std::max(p1.x, p2.x));
    std::uniform_real_distribution<double> ydistribution(std::min(p1.y, p2.y), std::max(p1.y, p2.y));

    double x = xdistribution(t_engine);
    double y = ydistribution(t_engine);

    return Point(x, y);
  }

  Point p1;
  Point p2;
};

#endif

