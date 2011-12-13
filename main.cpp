#include <vector>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cmath>


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
  Terrain_Type type;

  std::vector<std::pair<double, Terrain_Type>> probabilities;
};

class Map
{
  public:
    Map(int t_width, int t_height, Terrain_Type t_background)
      : m_width(t_width), m_height(t_height), m_background(t_background), m_seed(0)
    {
      m_map = render();
    }

    void add_terrain(Map_Terrain t_terrain)
    {
      m_terrains.push_back(t_terrain);
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
          Terrain_Type t = m_map.at(y).at(x).type;

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
          for (const auto &probability: m_map.at(y).at(x).probabilities)
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
    int m_seed;

    std::vector<std::vector<Map_Location>> m_map;

    static std::vector<std::vector<Map_Location>> create_map(int t_width, int t_height)
    {
      std::vector<std::vector<Map_Location>> result;
      for (int height = 0; height < t_height; ++height)
      {
        result.push_back(std::vector<Map_Location>(t_width));
      }

      return result;
    }

    void render_terrain_probabilities(std::vector<std::vector<Map_Location>> &t_map, std::mt19937 &t_engine)
    {
      int max_distance = std::min(m_width, m_height) / 2;

      for (const Map_Terrain &terrain: m_terrains)
      {
        auto position = get_position(m_width, m_height, terrain.location, t_engine);

        for (int x = 0; x < m_width; ++x)
        {
          for (int y = 0; y < m_height; ++y)
          {
            int distance = get_distance(position.first, position.second, x, y);
            double weight = distance>max_distance?0:(double(max_distance - distance) / double(max_distance));

            if (t_map.at(y).at(x).probabilities.empty())
            {
              // add background probability
              t_map.at(y).at(x).probabilities.push_back(std::make_pair(.05, m_background));
            }

            if (weight > 0)
            {
              t_map.at(y).at(x).probabilities.push_back(std::make_pair(weight, terrain.type));
            }
          }
        }
      }

      normalize_probabilities(t_map);
    }

    std::vector<std::vector<Map_Location>> render() 
    {
      std::mt19937 engine(m_seed);

      auto probability_map = create_map(m_width, m_height);
      render_terrain_probabilities(probability_map, engine);
      render_probabilities(probability_map, engine);

      return probability_map;
    }

    static std::pair<int, int> get_position(int t_map_width, int t_map_height, Location t_location, std::mt19937 &t_engine) 
    {
      switch (t_location)
      {
        case NorthEast:
          return std::make_pair(t_map_width * 5 / 6, t_map_height / 6);
        case North:
          return std::make_pair(t_map_width / 2, t_map_height / 6);
        case NorthWest:
          return std::make_pair(t_map_width / 6, t_map_height / 6);
        case East:
          return std::make_pair(t_map_width * 5 / 6, t_map_height / 2);
        case Central:
          return std::make_pair(t_map_width / 2, t_map_height / 2);
        case West:
          return std::make_pair(t_map_width / 6, t_map_height / 2);
        case SouthEast:
          return std::make_pair(t_map_width * 5 / 6, t_map_height * 5 / 6);
        case South:
          return std::make_pair(t_map_width / 2, t_map_height * 5 / 6);
        case SouthWest:
          return std::make_pair(t_map_width / 6, t_map_height * 5 / 6);
      }
      assert(!"Unknown location type");
    }

    int get_distance(int t_x1, int t_y1, int t_x2, int t_y2)
    {
      double xdist = abs(t_x1 - t_x2);
      double ydist = abs(t_y1 - t_y2);

      return floor(sqrt(xdist * xdist + ydist * ydist) + .5);
    }

    void normalize_probabilities(std::vector<std::vector<Map_Location>> &t_map) const
    {
      for (auto &outer: t_map)
      {
        for (auto &inner: outer)
        {
          double sum_of_probabilities = 0;

          for (const auto &probability: inner.probabilities)
          {
            sum_of_probabilities += probability.first;
          }

          for (auto &probability: inner.probabilities)
          {
            probability.first = probability.first / sum_of_probabilities;
          }
        }
      }
    }

    void render_probabilities(std::vector<std::vector<Map_Location>> &t_map, std::mt19937 &t_engine)
    {
      std::uniform_real_distribution<double> distribution(0, 1.0);

      for (auto &outer: t_map)
      {
        for (auto &inner: outer)
        {
          double val = distribution(t_engine);

          double total = 0;

          for (const auto &probability: inner.probabilities)
          {
            if (val > total && val <= probability.first + total)
            {
              inner.type = probability.second;
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


       /*
       Terrain_Type render_position(int t_map_width, int t_map_height, int t_x, int t_y)
       {
       std::vector<std::pair<double, Terrain_Type> > probabilities = get_probabilities(t_map_width, t_map_height, t_x, t_y);

       double val = double(rand()) / double(RAND_MAX);

       double total = 0;

       for (const auto &probability: probabilities)
       {
       if (val > total && val <= probability.first + total)
       {
       return probability.second;
       }

       total += probability.first;
       }

       assert(!"We cannot reach here");
       }

       void render_html(const std::string &t_filename, int t_map_width, int t_map_height)
       {

       std::ofstream ofs(t_filename.c_str());

       ofs << "<table>";
       for (int x = 0; x < t_map_width; ++x)
       {
       ofs << "<tr>";
       for (int y = 0; y < t_map_height; ++y)
       {
       ofs << "<td>";
       Terrain_Type t = render_position(t_map_width, t_map_height, x, y);

       ofs << to_string(t);
       ofs << "</td>";
       }

       ofs << "</tr>";
       }
       ofs << "</table>";
       }
       */



};


int main()
{
  Map m(40,30, Swamp);


  m.add_terrain(Map_Terrain(Central, Mountain));
  m.add_terrain(Map_Terrain(NorthEast, Mountain));
  m.add_terrain(Map_Terrain(NorthWest, Water));
  m.add_terrain(Map_Terrain(South, Swamp));

  m.dump_probabilities("probabilities.csv");

//  m.render_html("map.html", 25, 25);

  Screen screen;

  for (int i = 0; i < 100; ++i)
  {
    m.render_sdl(screen);
  //  sleep(1);
  }

}



