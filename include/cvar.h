#pragma once

#include <unordered_map>
#include <string>

namespace mc
{
  template<typename type>
  struct cvar_base
  {
    typedef type type_t;
    type_t value;
  };

  template<typename type>
  struct cvars
  {
    typedef type type_t;
    typedef cvars<type> cvars_t;
    typedef cvar_base<type> cvar_base_t;

    static cvars_t* get_instance()
    {
      static cvars_t cvars;
      return &cvars;
    }

    std::unordered_map<std::string, cvar_base_t*> cvar_map;

    type_t& get(const char* name) const
    {
      return cvar_map.at(name)->value;
    }

    cvar_base_t *get_cvar(const char* name) const
    {
      return cvar_map.at(name);
    }

    void add(const char* name, cvar_base_t *cvar)
    {
      cvar_map.emplace(name, cvar);
    }

    void remove(const char* name, cvar_base_t *cvar)
    {
      cvar_map.erase(name);
    }
  };

  template<typename type>
  struct cvar: public cvar_base<type>
  {
    typedef type type_t;
    typedef cvars<type> cvars_t;

    const char* name;
    const char* desc;

    cvar(const char* name, const char* desc, type_t value)
    {
      this->name = name;
      this->desc = desc;
      this->value = value;

      cvars_t::get_instance()->add(name, this);
    }

    ~cvar()
    {
      cvars_t::get_instance()->remove(name, this);
    }
  };
};

