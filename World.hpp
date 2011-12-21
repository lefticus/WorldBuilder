#ifndef WORLDBUILDER_WORLD_HPP
#define WORLDBUILDER_WORLD_HPP

#include "Map.hpp"

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

#endif

