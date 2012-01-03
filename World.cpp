#include <chrono>
#include "World.hpp"
#include <iostream>

void Simulation::simulate(const Simulation_Status &t_new_status)
{
}

Simulation::Simulation(const Simulation_Status &t_status, const Map_Instance &t_map)
  : status(t_status), map(t_map)
{
}


World_Instance::World_Instance(int t_tile_width, int t_tile_height, int t_num_horizontal, int t_num_vertical, std::mt19937 &t_engine,
        const Map &t_map)
  : m_tile_width(t_tile_width), m_tile_height(t_tile_height), m_num_horizontal(t_num_horizontal), m_num_vertical(t_num_vertical),
    m_simulation(Simulation_Status(), t_map.render(t_tile_width, t_tile_height, t_num_horizontal, t_num_vertical, t_engine)),
    m_current_simulation(m_simulation)
{
};

Simulation World_Instance::get_current_simulation() const
{
  std::unique_lock<std::mutex> l(m_mutex);
  return m_current_simulation;
}

void World_Instance::set_current_simulation(const Simulation &t_simulation) 
{
  std::unique_lock<std::mutex> l(m_mutex);
  m_current_simulation = t_simulation;
}

void World_Instance::set_new_status(const Simulation_Status &t_status)
{
  std::unique_lock<std::mutex> l(m_mutex);

  m_status = std::shared_ptr<Simulation_Status>(new Simulation_Status(t_status));
}

Simulation_Status World_Instance::get_new_status(const Simulation_Status &t_status)
{
  std::unique_lock<std::mutex> l(m_mutex);

  std::shared_ptr<Simulation_Status> s = m_status;
  m_status.reset();

  if (s) 
  {
    return *s;
  } else {
    return t_status;
  }
}



void World_Instance::start()
{
  m_cont_simulation = true;
  m_thread = std::thread(std::bind(&World_Instance::simulate, this));
}


void World_Instance::simulate()
{
  typedef std::conditional<std::chrono::high_resolution_clock::is_monotonic, std::chrono::high_resolution_clock, std::chrono::monotonic_clock>::type clocktype;

  clocktype::time_point starttime = clocktype::now();
  clocktype::time_point t1 = starttime;

  int frame = 0;

  while (m_cont_simulation)
  {
    ++frame;

    clocktype::time_point t2 = clocktype::now();
    clocktype::duration frame_duration = t2 - t1;
    clocktype::duration total_duration = t2 - starttime;

    t1 = t2;

    double frame_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(frame_duration).count();
    double total_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(total_duration).count();

    Simulation &sim = m_simulation;
    Simulation_Status status = get_new_status(sim.status);
    status.frame_ms = frame_ms;
    status.total_ms = total_ms;
    sim.simulate(status);

    set_current_simulation(sim);

    if (frame_ms < 1)
    {
      std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1-frame_ms));
    }

    if (frame % 1000 == 0)
    {
      std::cout << "Simulation FPS: " << (1 / frame_ms) * 1000 << std::endl;
    }

  }
}

World::World()
{
}


void World::add_map(const Map &t_map)
{
  m_maps.push_back(t_map);
}

std::shared_ptr<World_Instance> World::render(int t_tile_width, int t_tile_height, 
    int t_num_horizontal, int t_num_vertical, int t_seed) const
{
  std::mt19937 engine(t_seed);
  std::shared_ptr<World_Instance> wi(new World_Instance(t_tile_width, t_tile_height, 
        t_num_horizontal, t_num_vertical, engine, m_maps.at(0)));

  return wi;
}


