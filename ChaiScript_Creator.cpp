#include "ChaiScript_Creator.hpp"
#include <chaiscript/chaiscript_stdlib.hpp>

std::shared_ptr<chaiscript::ChaiScript> ChaiScript_Creator::create()
{
  return std::shared_ptr<chaiscript::ChaiScript>(new chaiscript::ChaiScript(chaiscript::Std_Lib::library()));
}


