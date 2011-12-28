#include "ChaiScript_Builder.hpp"
#include "ChaiScript_Creator.hpp"

#include "World.hpp"
#include <chaiscript/utility/utility.hpp>

std::shared_ptr<chaiscript::ChaiScript> ChaiScript_Builder::build()
{
  using namespace chaiscript;

  std::shared_ptr<chaiscript::ChaiScript> chai(ChaiScript_Creator::create());

  chaiscript::utility::add_class<Map_Terrain>(*chai,
      "Map_Terrain",
      { constructor<Map_Terrain(Location, Terrain_Type)>() },
      { {fun(&Map_Terrain::location), "location"},
        {fun(&Map_Terrain::type), "type"} }
      );

  chaiscript::utility::add_class<Map_Feature>(*chai,
      "Map_Feature",
      { constructor<Map_Feature(Location, Feature_Type)>() },
      { {fun(&Map_Feature::location), "location"},
        {fun(&Map_Feature::type), "type"} }
      );

  chaiscript::utility::add_class<Map>(*chai,
      "Map",
      { constructor<Map(Terrain_Type)>() },
      { {fun(&Map::add_terrain), "add_terrain"},
        {fun(&Map::add_map_feature), "add_map_feature"} }
      );

  chaiscript::utility::add_class<World>(*chai,
      "World",
      {  },
      { {fun(&World::add_map), "add_map"}
      }
      );


  chai->add(const_var(Swamp), "Swamp");
  chai->add(const_var(Mountain), "Mountain");
  chai->add(const_var(Plain), "Plain");
  chai->add(const_var(Water), "Water");
  chai->add(const_var(Forest), "Forest");

  chai->add(const_var(NorthWest), "NorthWest");
  chai->add(const_var(North), "North");
  chai->add(const_var(NorthEast), "NorthEast");
  chai->add(const_var(West), "West");
  chai->add(const_var(Central), "Central");
  chai->add(const_var(East), "East");
  chai->add(const_var(SouthWest), "SouthWest");
  chai->add(const_var(South), "South");
  chai->add(const_var(SouthEast), "SouthEast");

  chai->add(const_var(None), "None");
  chai->add(const_var(Cave), "Cave");
  chai->add(const_var(Town), "Town");

  return chai;
}


