#ifndef WORLDBUILDER_WORLD_HPP
#define WORLDBUILDER_WORLD_HPP

#include <random>

#include "Map.hpp"

class World_Instance
{
  public:
    World_Instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, std::mt19937 &t_engine,
        const Map &t_map);
    const Map_Instance &get_current_map() const;

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
    World();
    World_Instance render(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, int t_seed) const;
    void add_map(const Map &t_map);

  private:
    std::vector<Map> m_maps;
};

#endif

