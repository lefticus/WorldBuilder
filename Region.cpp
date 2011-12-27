#include "Region.hpp"

#include <cassert>


Region Region::get_location(Location t_location) const
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

std::vector<Region> Region::subdivide(int t_horizontal, int t_vertical) const
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

Region::Region(double t_width, double t_height)
 : m_p1(0,0), m_p2(t_width, t_height)
{
}

Region::Region(const Point &t_p1, const Point &t_p2)
 : m_p1(t_p1), m_p2(t_p2)
{
}

Region::Region(const Point &t_p1, double t_width, double t_height)
 : m_p1(t_p1), m_p2(t_p1.x + t_width, t_p1.y + t_height)
{
}

double Region::width() const
{
  return fabs(m_p1.x - m_p2.x);
}

double Region::height() const
{
  return fabs(m_p1.y - m_p2.y);
}

Point Region::top_left() const
{
  return Point(std::min(m_p1.x, m_p2.x), std::min(m_p1.y, m_p2.y));
}

Point Region::bottom_right() const
{
  return Point(std::max(m_p1.x, m_p2.x), std::max(m_p1.y, m_p2.y));
}


bool Region::contains(const Point &t_p) const
{
  return t_p >= top_left() && t_p <= bottom_right();
}

Point Region::choose_point(std::mt19937 &t_engine) const
{
  std::uniform_real_distribution<double> xdistribution(std::min(m_p1.x, m_p2.x), std::max(m_p1.x, m_p2.x));
  std::uniform_real_distribution<double> ydistribution(std::min(m_p1.y, m_p2.y), std::max(m_p1.y, m_p2.y));

  double x = xdistribution(t_engine);
  double y = ydistribution(t_engine);

  return Point(x, y);
}



