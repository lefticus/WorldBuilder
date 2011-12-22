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
  Map_Feature(Location t_location, Feature_Type t_type);

  Location location;
  Feature_Type type;
};

struct Map_Terrain
{
  Map_Terrain(Location t_location, Terrain_Type t_type);

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

    Map_Instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical);

    const Map_Tile &at(int x, int y) const;

    int num_horizontal() const;
    int num_vertical() const;
    Map_Tile &at(int x, int y);

  private:
    std::vector<Map_Tile> m_tiles;

    int m_tile_width;
    int m_tile_height;
    int m_num_horizontal;
    int m_num_vertical;

};

class Map
{
  public:
  Map(Terrain_Type t_background);

  void add_terrain(Map_Terrain t_terrain);

  void add_map_feature(Map_Feature t_feature);

  Map_Instance render(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, std::mt19937 &t_engine) const;

  private:
  struct Map_Rendered;
  Terrain_Type m_background;
  std::vector<Map_Terrain> m_terrains;
  std::vector<Map_Feature> m_features;
  int m_seed;

  void render_terrain(Map_Rendered &t_map, std::mt19937 &t_engine) const;
  void render_features(Map_Rendered &t_map, std::mt19937 &t_engine) const;

  Map_Instance make_instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, Map_Rendered &t_map) const;
};


#endif


