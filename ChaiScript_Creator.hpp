
#ifndef WORLDCREATOR_CHAISCRIPTCREATOR_HPP
#define WORLDCREATOR_CHAISCRIPTCREATOR_HPP

#include <chaiscript/chaiscript.hpp>

class ChaiScript_Creator
{
  public:
    static std::shared_ptr<chaiscript::ChaiScript> create();

};

#endif
