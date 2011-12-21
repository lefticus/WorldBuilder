#ifndef WORLDBUILDER_SDL_HPP
#define WORLDBUILDER_SDL_HPP

#include <stdexcept>
#include <string>

#include "World.hpp"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <iostream>

class Surface
{
  public:
    typedef char* (*ErrorFunc)();

    Surface(SDL_Surface *t_surf)
      : m_surface(t_surf)
    {
      if (!m_surface)
      {
        throw std::runtime_error(SDL_GetError());
      }
    }

    Surface(SDL_Surface *t_surf, ErrorFunc t_errfunc)
      : m_surface(t_surf)
    {
      if (!m_surface)
      {
        throw std::runtime_error(t_errfunc());
      }
    }

    void clear()
    {
      SDL_Rect dest;
      dest.x=0;
      dest.y=0;
      dest.w=m_surface->w;
      dest.h=m_surface->h;
      //      SDL_SetAlpha(m_surface, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
      SDL_FillRect(m_surface, &dest, SDL_MapRGBA(m_surface->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));
    }

    void flip()
    {
      SDL_Flip(m_surface);
    }

    ~Surface()
    {
      SDL_FreeSurface(m_surface);
    }

    void render(Surface &t_surface, int t_x, int t_y) const
    {
      SDL_Rect dest;
      dest.x = t_x;
      dest.y = t_y;
      dest.w = m_surface->w;
      dest.h = m_surface->h;

      SDL_BlitSurface(m_surface, NULL, t_surface.m_surface, &dest);
    }

    double width() const
    {
      return m_surface->w;
    }

    double height() const
    {
      return m_surface->h;
    }

  private:
    SDL_Surface *m_surface;
};

class Screen
{
  public:
    Screen()
      : m_initializer(), m_surface(SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_HWACCEL))
    {

    }


    Surface &getSurface()
    {
      return m_surface;
    }

  private:
    struct Initializer
    {
      Initializer()
      {
        if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
          throw std::runtime_error(std::string("Unable to init SDL: ") + SDL_GetError());
        }
      }

      ~Initializer()
      {
        SDL_Quit();
      }
    };

    Initializer m_initializer;
    Surface m_surface;
};

class Object
{
  public:
    Object(const std::string &t_filename)
      : m_surface(IMG_Load(t_filename.c_str()), &IMG_GetError)
    {
    }

    ~Object()
    {
    }

    void render(Surface &t_surface, int t_x, int t_y) const
    {
      m_surface.render(t_surface, t_x, t_y);
    }


  private:
    Object(const Object &);
    Object &operator=(const Object &);

    Surface m_surface;
};

class SDL_Engine
{
  public:
    SDL_Engine(const World &t_world)
      : m_world(t_world.render(16,16,640/16,480/16,0))
    {
    }


    void run()
    {
      int frame = 0;
      while (true)
      {
        render_sdl(m_screen, m_world.get_current_map());
        ++frame;

        if (time(0) % 5 == 0)
        {
          std::cout << "FPS: " << double(frame) / 5 << std::endl;
          frame = 0;
        }
      }
    }

    void render_sdl(Screen &t_screen, const Map_Instance &t_map) const
    {
      Object mountain("mountainvoxel.png");
      Object swamp("swampvoxel.png");
      Object water("watervoxel.png");
      Object forest("forestvoxel.png");
      Object plain("plainvoxel.png");
      Object cave("cavevoxel.png");
      Object town("townvoxel.png");


      t_screen.getSurface().clear();

      int width = t_map.num_horizontal();
      int height = t_map.num_vertical();

      for (int x = 0; x < width; ++x)
      {
        for (int y = 0; y < height; ++y)
        {

          int renderx = x * 16 - 4;
          int rendery = y * 16 - 4;


          switch (t_map.at(x,y).terrain_type)
          {
            case Mountain:
              mountain.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Swamp:
              swamp.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Water:
              water.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Forest:
              forest.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Plain:
              plain.render(t_screen.getSurface(), renderx, rendery);
              break;
          };

          switch (t_map.at(x,y).feature_type)
          {
            case Cave:
              cave.render(t_screen.getSurface(), renderx, rendery);
              break;
            case Town:
              town.render(t_screen.getSurface(), renderx, rendery);
              break;
            case None:
              break;
          }
        }
      }


      t_screen.getSurface().flip();
    }

  private:
    World_Instance m_world;
    Screen m_screen;

};

#endif



