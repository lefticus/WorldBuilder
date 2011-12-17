#include <vector>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <map>
#include <iostream>
#include <algorithm>

#include <stdexcept>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

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

enum Terrain_Type
{
  Mountain,
  Plain,
  Water,
  Swamp
};

enum Feature_Type
{
  Cave,
  Town
};

class Surface
{
  public:
    typedef char* (*ErrorFunc)();

    Surface(SDL_Surface *t_surf)
      : m_surface(t_surf)
    {
      if (!m_surface)
      {
        throw std::runtime_error(SDL_GetError());
      }
    }

    Surface(SDL_Surface *t_surf, ErrorFunc t_errfunc)
      : m_surface(t_surf)
    {
      if (!m_surface)
      {
        throw std::runtime_error(t_errfunc());
      }
    }

    void clear()
    {
      SDL_Rect dest;
      dest.x=0;
      dest.y=0;
      dest.w=m_surface->w;
      dest.h=m_surface->h;
      //      SDL_SetAlpha(m_surface, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
      SDL_FillRect(m_surface, &dest, SDL_MapRGBA(m_surface->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));
    }

    void flip()
    {
      SDL_Flip(m_surface);
    }

    ~Surface()
    {
      SDL_FreeSurface(m_surface);
    }

    void render(Surface &t_surface, int t_x, int t_y) const
    {
      SDL_Rect dest;
      dest.x = t_x;
      dest.y = t_y;
      dest.w = m_surface->w;
      dest.h = m_surface->h;

      SDL_BlitSurface(m_surface, NULL, t_surface.m_surface, &dest);
    }

    double width() const
    {
      return m_surface->w;
    }

    double height() const
    {
      return m_surface->h;
    }

  private:
    SDL_Surface *m_surface;
};

class Screen
{
  public:
    Screen()
      : m_initializer(), m_surface(SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_HWACCEL))
    {

    }


    Surface &getSurface()
    {
      return m_surface;
    }

  private:
    struct Initializer
    {
      Initializer()
      {
        if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
          throw std::runtime_error(std::string("Unable to init SDL: ") + SDL_GetError());
        }
      }

      ~Initializer()
      {
        SDL_Quit();
      }
    };

    Initializer m_initializer;
    Surface m_surface;
};

class Object
{
  public:
    Object(const std::string &t_filename)
      : m_surface(IMG_Load(t_filename.c_str()), &IMG_GetError)
    {
    }

    ~Object()
    {
    }

    void render(Surface &t_surface, int t_x, int t_y) const
    {
      m_surface.render(t_surface, t_x, t_y);
    }


  private:
    Object(const Object &);
    Object &operator=(const Object &);

    Surface m_surface;
};


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


  double x;
  double y;
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

    for (int h = 0; h < t_vertical; ++h)
    {
      for (int w = 0; w < t_horizontal; ++w)
      {
        regions.push_back(
            {
              { regionwidth * w / t_horizontal, regionheight * h / t_vertical },
              { regionwidth * (w + 1) / t_horizontal, regionheight * (h + 1) / t_vertical }
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

  double width() const
  {
    return fabs(p1.x - p2.x);
  }

  double height() const
  {
    return fabs(p1.y - p2.y);
  }


  Point choose_point(std::mt19937 &t_engine) const
  {
    std::uniform_real_distribution<double> xdistribution(std::min(p1.x, p2.x), std::max(p1.x, p2.x));
    std::uniform_real_distribution<double> ydistribution(std::min(p1.y, p2.y), std::max(p1.y, p2.y));

    return Point(xdistribution(t_engine), ydistribution(t_engine));
  }

  Point p1;
  Point p2;
};



struct Map_Feature
{
  Map_Feature(Location t_location, Feature_Type t_type)
    : location(t_location), type(t_type)
  {
  }

  Location location;
  Feature_Type type;
};

struct Map_Terrain
{
  Map_Terrain(Location t_location, Terrain_Type t_type)
    : location(t_location), type(t_type)
  {
  }

  Location location;
  Terrain_Type type;
};

struct Map_Location
{
  Terrain_Type terrain;
  Feature_Type feature;
};

struct Shape
{
  struct Circle
  {
    Circle(Point t_center, double t_radius)
      : center(t_center), radius(t_radius)
    {
    }

    Point center;
    double radius;

    bool contains(Point t_p) const
    {
      return t_p.distance(center) <= radius;
    }
  };

  Shape(Region t_region, std::mt19937 &t_engine)
  {
    double max_radius = std::min(t_region.width(), t_region.height());

    int num_circles = std::uniform_int_distribution<int>(3, 6)(t_engine);
    std::uniform_real_distribution<double> radius(0, max_radius);

    for (int i = 0; i < num_circles; ++i)
    {
      Point p = t_region.choose_point(t_engine);
      double r = radius(t_engine);

      circles.push_back(Circle(p, r));
    }
  }

  std::vector<Circle> circles;

  bool contains(Point t_p) const
  {
    for (const Circle &circle: circles)
    {
      if (circle.contains(t_p))
      {
        return true;
      }
    }

    return false;
  }
};

struct Map_Data
{
  Map_Data(Terrain_Type t_background)
    : background(t_background)
  {
  }

  struct Map_Data_Terrain
  {
    Map_Data_Terrain(const Shape &t_shape, Terrain_Type t_type)
      : shape(t_shape), type(t_type)
    {
    }

    Shape shape;
    Terrain_Type type;
  };

  std::vector<Map_Data_Terrain> terrains;

  Terrain_Type background;

  void add_terrain(Terrain_Type t_type, Location t_location, std::mt19937 &t_engine)
  {
    Shape shape(region().get_location(t_location), t_engine);

    terrains.push_back(Map_Data_Terrain(shape, t_type));
  }

  Region region() const
  {
    return Region(1.0, 1.0);
  }

  Map_Location at(double t_width, double t_height, Point t_p) const
  {
    double region_width = region().width();
    double region_height = region().height();

    Point scaled_point(t_p.x / t_width * region_width, t_p.y / t_height * region_height);

    Map_Location loc;
    loc.terrain = background;

    for (auto itr = terrains.rbegin();
         itr != terrains.rend();
         ++itr)
    {
      if (itr->shape.contains(scaled_point))
      {
        loc.terrain = itr->type;
        break;
      }
    }

    return loc;
  }


};


class Map
{
  public:
    Map(Terrain_Type t_background)
      : m_background(t_background), m_seed(0), m_map(t_background)
    {
    }

    void set_seed(int t_seed)
    {
      m_seed = t_seed;
      m_map = render();
    }

    void add_terrain(Map_Terrain t_terrain)
    {
      m_terrains.push_back(t_terrain);
      m_map = render();
    }

    void add_map_feature(Map_Feature t_feature)
    {
      m_features.push_back(t_feature);
      m_map = render();
    }

    void render_sdl(Screen &t_screen) const
    {
      Object mountain("mountainvoxel.png");
      Object swamp("swampvoxel.png");
      Object water("watervoxel.png");

      t_screen.getSurface().clear();

      int width = t_screen.getSurface().width() / 16;
      int height = t_screen.getSurface().height() / 16;

      for (int x = 0; x < width; ++x)
      {
        for (int y = 0; y < height; ++y)
        {
          Terrain_Type t = m_map.at( width, height, Point(x,y) ).terrain;

          int renderx = x * 16 - 4;
          int rendery = y * 16 - 4;

          switch (t)
          {
            case Mountain:
              mountain.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Swamp:
              swamp.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Water:
              water.render(t_screen.getSurface(), renderx, rendery);
              break;
          };
        }
      }

      t_screen.getSurface().flip();
    }


  private:
    Terrain_Type m_background;
    std::vector<Map_Terrain> m_terrains;
    std::vector<Map_Feature> m_features;
    int m_seed;

    Map_Data m_map;


    Map_Data render() 
    {
      std::mt19937 engine(m_seed);

      Map_Data rendered_map(m_background);
      render_terrain(rendered_map, engine);
      render_features(rendered_map, engine);

      return rendered_map;
    }


    void render_terrain(Map_Data &t_map, std::mt19937 &t_engine)
    {
      for (const auto &terrain: m_terrains)
      {
        t_map.add_terrain(terrain.type, terrain.location, t_engine);
      }
    }

    void render_features(Map_Data &t_map, std::mt19937 &t_engine)
    {
      /*
      std::vector<Map_Feature> features = m_features;

      std::map<Location, std::vector<Map_Feature>> features_by_location;

      for (const auto &feature: m_features)
      {
        features_by_location[feature.location].push_back(feature);
      }

      for (const auto &locationfeature: features_by_location)
      {
        Location location = locationfeature.first;

        auto locationregion = get_position_range(m_width, m_height, location);

        int size = locationfeature.second.size();

        int griddivision = ceil(sqrt(size));
        std::vector<Region> subregions = locationregion.subdivide(griddivision, griddivision);

        std::random_shuffle(subregions.begin(), subregions.end(), 
            [&](int i){ return std::uniform_int_distribution<>(0,i-1)(t_engine);  } );

        for (int i = 0; i < size; ++i)
        {
          const Map_Feature &feature = locationfeature.second[i];

          Point p = subregions[i].choose_point(t_engine);

          t_map.at(p).feature = feature.type;
        }

      }

      */
    }



    std::string to_string(Terrain_Type t_t)
    {
      switch (t_t)
      {
        case Mountain:
          return "Mountain";
        case Plain:
          return "Plain";
        case Swamp:
          return "Swamp";
        case Water:
          return "Water";
      }

      assert(!"unknown type");
    }




};


int main()
{
  Map m(Swamp);


  m.add_terrain(Map_Terrain(Central, Mountain));
  m.add_terrain(Map_Terrain(NorthEast, Mountain));
  m.add_terrain(Map_Terrain(NorthWest, Water));
  m.add_terrain(Map_Terrain(South, Mountain));

  //m.dump_probabilities("probabilities.csv");

//  m.render_html("map.html", 25, 25);

  Screen screen;

  for (int i = 0; i < 100; ++i)
  {
    m.set_seed(i);
    m.render_sdl(screen);
  //  sleep(1);
  }

}



