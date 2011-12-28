#include "SDL.hpp"

#include "ChaiScript_Builder.hpp"

int main(int , char *argv[])
{
  World world;

  std::shared_ptr<chaiscript::ChaiScript> chai = ChaiScript_Builder::build();
  chai->add(chaiscript::var(std::ref(world)), "world");
  chai->eval_file(argv[1]);

  SDL_Engine e(world);
  e.run(); 

}



