#ifndef WORLDBUILDER_WORLD_HPP
#define WORLDBUILDER_WORLD_HPP

#include <random>
#include <atomic>
#include <memory>
#include <thread>

#include "Map.hpp"

class Simulation_Status
{
  public:
    double frame_ms;
    double total_ms;
};

class Simulation
{
  public:
    Simulation_Status status;
    Map_Instance map;

    Simulation(const Simulation_Status &t_status, const Map_Instance &t_map);

    void simulate(const Simulation_Status &t_new_status);
};

class World_Instance
{
  public:
    World_Instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, std::mt19937 &t_engine,
        const Map &t_map);
    Simulation get_current_simulation() const;
    World_Instance(const World_Instance &) = delete;
    World_Instance &operator=(const World_Instance &) = delete;
    void start();
    void set_new_status(const Simulation_Status &t_status);

  private:
    int m_tile_width;
    int m_tile_height;
    int m_num_horizontal;
    int m_num_vertical;

    void set_current_simulation(const Simulation &t_simulation);
    Simulation_Status get_new_status(const Simulation_Status &t_status);

    Simulation m_simulation;
    Simulation m_current_simulation; //< \todo make this an std::atomic<> when the compiler allows for it
    std::shared_ptr<Simulation_Status> m_status; //< \todo make this an std::atomic<> when the compiler allows for it
    std::atomic_bool m_cont_simulation;

    std::thread m_thread;
    mutable std::mutex m_mutex;
    void simulate();
};

class World
{
  public:
    World();
    std::shared_ptr<World_Instance> render(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, int t_seed) const;
    void add_map(const Map &t_map);

  private:
    std::vector<Map> m_maps;
};

#endif

