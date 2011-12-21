#ifndef WORLDBUILDER_MAP
#define WORLDBUILDER_MAP

#include "Shape.hpp"
#include "Region.hpp"

#include <algorithm>

#include <vector>
#include <map>

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


#endif


