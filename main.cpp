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
  Swamp,
  Forest
};

enum Feature_Type
{
  None,
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


class Map_Instance
{
  public:
    struct Map_Tile
    {
      Terrain_Type terrain_type;
      Feature_Type feature_type;
    };

    Map_Instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical)
      : m_tiles(t_num_horizontal * t_num_vertical),
        m_tile_width(t_tile_width), m_tile_height(t_tile_height), m_num_horizontal(t_num_horizontal), m_num_vertical(t_num_vertical)
    {
    }

    const Map_Tile &at(int x, int y) const
    {
      if (x >= m_num_horizontal || y >= m_num_vertical || x < 0 || y < 0)
      {
        throw std::range_error("Outside of map range");
      }

      return m_tiles[y * m_num_horizontal + x];
    }

    int num_horizontal() const
    {
      return m_num_horizontal;
    }

    int num_vertical() const
    {
      return m_num_vertical;
    }

    Map_Tile &at(int x, int y) 
    {
      if (x >= m_num_horizontal || y >= m_num_vertical || x < 0 || y < 0)
      {
        throw std::range_error("Outside of map range");
      }

      return m_tiles[y * m_num_horizontal + x];
    }

  private:
    std::vector<Map_Tile> m_tiles;

    int m_tile_width;
    int m_tile_height;
    int m_num_horizontal;
    int m_num_vertical;

};

class Map
{
  struct Map_Rendered
  {
    Map_Rendered(Terrain_Type t_background, double t_aspect_ratio)
      : background(t_background), aspect_ratio(t_aspect_ratio)
    {
    }

    struct Map_Rendered_Terrain
    {
      Map_Rendered_Terrain(const Shape &t_shape, Terrain_Type t_type)
        : shape(t_shape), type(t_type)
      {
      }

      Shape shape;
      Terrain_Type type;
    };

    struct Map_Rendered_Feature
    {
      Map_Rendered_Feature(const Point &t_point, Feature_Type t_type)
        : point(t_point), type(t_type)
      {
      }

      Point point;
      Feature_Type type;
    };

    std::vector<Map_Rendered_Terrain> terrains;
    std::vector<Map_Rendered_Feature> features;

    Terrain_Type background;
    double aspect_ratio;

    void add_terrain(Terrain_Type t_type, Location t_location, std::mt19937 &t_engine)
    {
      Shape shape(region().get_location(t_location), t_engine);

      terrains.push_back(Map_Rendered_Terrain(shape, t_type));
    }

    void add_feature(Feature_Type t_type, Point t_p)
    {
      features.push_back(Map_Rendered_Feature(t_p, t_type));
    }

    Region region() const
    {
      return Region(aspect_ratio, 1.0);
    }

    std::vector<std::pair<Point, Feature_Type>> map_features(double t_width, double t_height) const
    {
      double region_width = region().width();
      double region_height = region().height();

      std::vector<std::pair<Point, Feature_Type>> retval;

      for (const Map_Rendered_Feature &feature: features)
      {
        retval.push_back(std::make_pair(Point(t_width * (feature.point.x / region_width), t_height * (feature.point.y / region_height)), feature.type));
      }

      return retval;
    }

    Map_Location at(double t_width, double t_height, Point t_p) const
    {
      double region_width = region().width();
      double region_height = region().height();

      Point scaled_point(t_p.x / t_width * region_width, t_p.y / t_height * region_height);

      Map_Location loc;
      loc.terrain = background;
      loc.feature = None;


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

      Region r(scaled_point, region_width / t_width, region_height / t_height);


      for (const auto &feature: features)
      {
        if (r.contains(feature.point))
        {
          loc.feature = feature.type;
          break;
        }
      }

      return loc;
    }
  };
  public:
  Map(Terrain_Type t_background)
    : m_background(t_background)
  {
  }

  void add_terrain(Map_Terrain t_terrain)
  {
    m_terrains.push_back(t_terrain);
  }

  void add_map_feature(Map_Feature t_feature)
  {
    m_features.push_back(t_feature);
  }

  Map_Instance render(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, std::mt19937 &t_engine) const
  {
    Map_Rendered rendered_map(m_background, double(t_tile_width * t_num_horizontal) / double(t_tile_height * t_num_vertical));
    render_terrain(rendered_map, t_engine);
    render_features(rendered_map, t_engine);
    return make_instance(t_tile_width, t_tile_height, t_num_horizontal, t_num_vertical, rendered_map);
  }


  private:
  Terrain_Type m_background;
  std::vector<Map_Terrain> m_terrains;
  std::vector<Map_Feature> m_features;
  int m_seed;

  void render_terrain(Map_Rendered &t_map, std::mt19937 &t_engine) const
  {
    for (const auto &terrain: m_terrains)
    {
      t_map.add_terrain(terrain.type, terrain.location, t_engine);
    }
  }

  void render_features(Map_Rendered &t_map, std::mt19937 &t_engine) const
  {
    std::vector<Map_Feature> features = m_features;

    std::map<Location, std::vector<Map_Feature>> features_by_location;

    for (const auto &feature: m_features)
    {
      features_by_location[feature.location].push_back(feature);
    }

    for (const auto &locationfeature: features_by_location)
    {
      Location location = locationfeature.first;

      auto locationregion = t_map.region().get_location(location);

      int size = locationfeature.second.size();

      int griddivision = ceil(sqrt(size));
      std::vector<Region> subregions = locationregion.subdivide(griddivision, griddivision);

      std::random_shuffle(subregions.begin(), subregions.end(), 
          [&](int i){ return std::uniform_int_distribution<>(0,i-1)(t_engine);  } );

      for (int i = 0; i < size; ++i)
      {
        const Map_Feature &feature = locationfeature.second[i];

        Point p = subregions[i].choose_point(t_engine);

        t_map.add_feature(feature.type, p);
      }
    }
  }

  Map_Instance make_instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, Map_Rendered &t_map) const
  {

    Map_Instance instance(t_tile_width, t_tile_height, t_num_horizontal, t_num_vertical);

    for (int x = 0; x < t_num_horizontal; ++x)
    {
      for (int y = 0; y < t_num_vertical; ++y)
      {
        auto tile = t_map.at( t_num_horizontal, t_num_vertical, Point(x,y) );

        instance.at(x, y).terrain_type = tile.terrain;
        instance.at(x, y).feature_type = tile.feature;
      }
    }

    return instance;

  }
};

class World_Instance
{
  public:
    World_Instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, std::mt19937 &t_engine,
        const Map &t_map)
      : m_tile_width(t_tile_width), m_tile_height(t_tile_height), m_num_horizontal(t_num_horizontal), m_num_vertical(t_num_vertical),
        m_map(t_map.render(t_tile_width, t_tile_height, t_num_horizontal, t_num_vertical, t_engine))
    {
    };

    const Map_Instance &get_current_map() const
    {
      return m_map;
    }

  private:
    int m_tile_width;
    int m_tile_height;
    int m_num_horizontal;
    int m_num_vertical;
    Map_Instance m_map;
};

class World
{
  public:
    World(const Map &t_map)
      : m_map(t_map)
    {
    }

    World_Instance render(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, int t_seed) const
    {
      std::mt19937 engine(t_seed);
      World_Instance wi(t_tile_width, t_tile_height, t_num_horizontal, t_num_vertical, engine, m_map);

      return wi;
    }

  private:
    Map m_map;
};

class SDL_Engine
{
  public:
    SDL_Engine(const World &t_world)
      : m_world(t_world.render(16,16,640/16,480/16,0))
    {
    }


    void run()
    {
      int frame = 0;
      while (true)
      {
        render_sdl(m_screen, m_world.get_current_map());
        ++frame;

        if (time(0) % 5 == 0)
        {
          std::cout << "FPS: " << double(frame) / 5 << std::endl;
          frame = 0;
        }
      }
    }

    void render_sdl(Screen &t_screen, const Map_Instance &t_map) const
    {
      Object mountain("mountainvoxel.png");
      Object swamp("swampvoxel.png");
      Object water("watervoxel.png");
      Object forest("forestvoxel.png");
      Object plain("plainvoxel.png");
      Object cave("cavevoxel.png");
      Object town("townvoxel.png");


      t_screen.getSurface().clear();

      int width = t_map.num_horizontal();
      int height = t_map.num_vertical();

      for (int x = 0; x < width; ++x)
      {
        for (int y = 0; y < height; ++y)
        {

          int renderx = x * 16 - 4;
          int rendery = y * 16 - 4;


          switch (t_map.at(x,y).terrain_type)
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
            case Forest:
              forest.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Plain:
              plain.render(t_screen.getSurface(), renderx, rendery);
              break;
          };

          switch (t_map.at(x,y).feature_type)
          {
            case Cave:
              cave.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Town:
              town.render(t_screen.getSurface(), renderx, rendery);
              break;
            case None:
              break;
          }
        }
      }


      t_screen.getSurface().flip();
    }

  private:
    World_Instance m_world;
    Screen m_screen;

};

int main()
{
  Map m(Swamp);

  m.add_terrain(Map_Terrain(East, Forest));
  m.add_terrain(Map_Terrain(West, Plain));
  m.add_terrain(Map_Terrain(Central, Mountain));
  m.add_terrain(Map_Terrain(NorthEast, Mountain));
  m.add_terrain(Map_Terrain(NorthWest, Water));
  m.add_terrain(Map_Terrain(South, Mountain));

  m.add_map_feature(Map_Feature(SouthWest, Town));
  m.add_map_feature(Map_Feature(SouthWest, Town));
  m.add_map_feature(Map_Feature(SouthWest, Cave));
  m.add_map_feature(Map_Feature(NorthEast, Town));

  World world(m);

  SDL_Engine e(world);
  e.run(); 

}



