#include "SDL.hpp"

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



