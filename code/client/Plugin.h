//NODA: Copyright(c) NOMAD Group<nomad - group.net>

#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include <loader.hpp>

class NodaPlugin {
public:
  NodaPlugin();
  ~NodaPlugin();

private:
  static ssize_t idaapi UiHandler(void*, int, va_list);
};

extern NodaPlugin *g_Plugin;