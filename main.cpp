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
  Point(int t_x, int t_y)
    : x(t_x), y(t_y)
  {
  }

  int x;
  int y;
};

struct Region
{
  std::vector<Region> subdivide(int t_horizontal, int t_vertical)
  {
    std::cout << " subdivide " << t_horizontal << " " << t_vertical << std::endl;
    std::vector<Region> regions;

    int width = abs(p2.x - p1.x);
    int height = abs(p2.y - p1.y);

    for (int w = 0; w < t_horizontal; ++w)
    {
      for (int h = 0; h < t_vertical; ++h)
      {
        regions.push_back(
            {
              { width * w / t_horizontal, height * h / t_vertical },
              { width * (w + 1) / t_horizontal - ((w==t_horizontal-1)?0:1), height * (h + 1) / t_vertical - ((h==t_vertical-1)?0:1)}
            }
            );


        std::cout << " subdivide " << regions.back().p1.x << " " << regions.back().p1.y << " " << regions.back().p2.x << " " << regions.back().p2.y << std::endl;


      }
    }



    return regions;
  }

  Region(int t_width, int t_height)
    : p1(0,0), p2(t_width-1, t_height-1)
  {
  }

  Region(Point t_p1, Point t_p2)
    : p1(t_p1), p2(t_p2)
  {
  }


  Point choose_point(std::mt19937 &t_engine)
  {
    std::uniform_int_distribution<int> xdistribution(std::min(p1.x, p2.x), std::max(p1.x, p2.x));
    std::uniform_int_distribution<int> ydistribution(std::min(p1.y, p2.y), std::max(p1.y, p2.y));

    return Point(xdistribution(t_engine), ydistribution(t_engine));
  }

  Point p1;
  Point p2;
};


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

  std::vector<std::pair<double, Terrain_Type>> probabilities;
};

struct Map_Data
{
  Map_Data(int t_width, int t_height)
    : locations(create_map(t_width, t_height))
  {
  }

  std::vector<std::vector<Map_Location>> locations;

  Map_Location &at(Point t_p)
  {
    return locations.at(t_p.y).at(t_p.x);
  }

  const Map_Location &at(Point t_p) const
  {
    return locations.at(t_p.y).at(t_p.x);
  }


  static std::vector<std::vector<Map_Location>> create_map(int t_width, int t_height)
  {
    std::vector<std::vector<Map_Location>> result;
    for (int height = 0; height < t_height; ++height)
    {
      result.push_back(std::vector<Map_Location>(t_width));
    }

    return result;
  }
};


class Map
{
  public:
    Map(int t_width, int t_height, Terrain_Type t_background)
      : m_width(t_width), m_height(t_height), m_background(t_background), m_seed(0),
        m_map(t_width, t_height)
    {
      m_map = render();
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

      for (int x = 0; x < m_width; ++x)
      {
        for (int y = 0; y < m_height; ++y)
        {
          Terrain_Type t = m_map.at( {x, y} ).terrain;

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

    void dump_probabilities(const std::string &t_filename)
    {
      std::ofstream ofs(t_filename.c_str());

      for (int x = 0; x < m_width; ++x)
      {
        for (int y = 0; y < m_height; ++y)
        {
          for (const auto &probability: m_map.at( {x, y} ).probabilities)
          {
            ofs << probability.first << " " << to_string(probability.second) << " ";
          }

          ofs << ", ";
        }

        ofs << "\n";
      }

    }


  private:
    int m_width;
    int m_height;
    Terrain_Type m_background;
    std::vector<Map_Terrain> m_terrains;
    std::vector<Map_Feature> m_features;
    int m_seed;

    Map_Data m_map;



    void render_terrain_probabilities(Map_Data &t_map, std::mt19937 &t_engine)
    {
      int max_distance = std::min(m_width, m_height) / 2;

      for (const Map_Terrain &terrain: m_terrains)
      {
        auto position = get_position(m_width, m_height, terrain.location, t_engine);

        for (int x = 0; x < m_width; ++x)
        {
          for (int y = 0; y < m_height; ++y)
          {
            int distance = get_distance(position.x, position.y, x, y);
            double weight = distance>max_distance?0:(double(max_distance - distance) / double(max_distance));

            if (t_map.at( {x, y} ).probabilities.empty())
            {
              // add background probability
              t_map.at( {x, y} ).probabilities.push_back(std::make_pair(.05, m_background));
            }

            if (weight > 0)
            {
              t_map.at( {x, y} ).probabilities.push_back(std::make_pair(weight, terrain.type));
            }
          }
        }
      }

      normalize_probabilities(t_map);
    }

    Map_Data render() 
    {
      std::mt19937 engine(m_seed);

      Map_Data probability_map(m_width, m_height);
      render_terrain_probabilities(probability_map, engine);
      render_probabilities(probability_map, engine);
      render_features(probability_map, engine);

      return probability_map;
    }

    static Point get_position(int t_map_width, int t_map_height, Location t_location, std::mt19937 &t_engine)
    {
      return get_position_range(t_map_width, t_map_height, t_location).choose_point(t_engine);
    }


    static Region get_position_range(int t_map_width, int t_map_height, Location t_location)
    {
      std::vector<Region> regions = Region(t_map_width, t_map_height).subdivide(3,3);

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

    int get_distance(int t_x1, int t_y1, int t_x2, int t_y2)
    {
      return floor(hypot(abs(t_x1 - t_x2), abs(t_y1 - t_y2)) + .5);
    }

    void normalize_probabilities(Map_Data &t_map) const
    {
      for (int x = 0; x < m_width; ++x)
      {
        for (int y = 0; y < m_height; ++y)
        {
          double sum_of_probabilities = 0;

          Map_Location &loc = t_map.at(Point(x, y));

          for (const auto &probability: loc.probabilities)
          {
            sum_of_probabilities += probability.first;
          }

          for (auto &probability: loc.probabilities)
          {
            probability.first = probability.first / sum_of_probabilities;
          }
        }
      }
    }

    void render_features(Map_Data &t_map, std::mt19937 &t_engine)
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
    }

    void render_probabilities(Map_Data &t_map, std::mt19937 &t_engine)
    {
      std::uniform_real_distribution<double> distribution(0, 1.0);

      for (int x = 0; x < m_width; ++x)
      {
        for (int y = 0; y < m_height; ++y)
        {
          double val = distribution(t_engine);

          double total = 0;

          Map_Location &loc = t_map.at(Point(x, y));

          for (const auto &probability: loc.probabilities)
          {
            if (val > total && val <= probability.first + total)
            {
              loc.terrain = probability.second;
              break;
            }

            total += probability.first;
          }

        }
      }
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
  Map m(40,30, Swamp);


  m.add_terrain(Map_Terrain(Central, Mountain));
  m.add_terrain(Map_Terrain(NorthEast, Mountain));
  m.add_terrain(Map_Terrain(NorthWest, Water));
  m.add_terrain(Map_Terrain(South, Mountain));

  m.dump_probabilities("probabilities.csv");

//  m.render_html("map.html", 25, 25);

  Screen screen;

  for (int i = 0; i < 100; ++i)
  {
    m.set_seed(i);
    m.render_sdl(screen);
  //  sleep(1);
  }

}



