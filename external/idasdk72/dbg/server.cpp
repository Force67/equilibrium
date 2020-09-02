/*
       IDA remote debugger server
*/

#ifdef _WIN32
// We use the deprecated inet_ntoa() function for Windows XP compatibility.
//lint -e750 local macro '' not referenced
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <expr.hpp>

#include "server.h"

// Provide dummy versions for tinfo copy/clear. Debugger servers do not use them
#if !defined(__NT__)
void ida_export copy_tinfo_t(tinfo_t *, const tinfo_t &) {}
void ida_export clear_tinfo_t(tinfo_t *) {}
#endif

// We don't have a kernel. Provide envvar-based debug file directory retrieval.
#if defined(__LINUX__)
static bool _elf_debug_file_directory_resolved = false;
static qstring _elf_debug_file_directory;
idaman const char *ida_export get_elf_debug_file_directory()
{
  if ( !_elf_debug_file_directory_resolved )
  {
    if ( !qgetenv("DEBUG_FILE_DIRECTORY", &_elf_debug_file_directory) )
      qgetenv("ELF_DEBUG_FILE_DIRECTORY", &_elf_debug_file_directory);
    _elf_debug_file_directory_resolved = true;
  }
  return _elf_debug_file_directory.c_str();
}

//-------------------------------------------------------------------------
#ifdef TESTABLE_BUILD
void set_elf_debug_file_directory(const char *path)
{
  _elf_debug_file_directory = path;
  _elf_debug_file_directory_resolved = true;
}
#endif
#endif

//lint -esym(714, dump_udt) not referenced
void dump_udt(const char *, const struct udt_type_data_t &) {}


//--------------------------------------------------------------------------
// SERVER GLOBAL VARIABLES
#ifdef __SINGLE_THREADED_SERVER__
dbgsrv_dispatcher_t dispatcher(false);

static bool init_lock(void) { return true; }
bool lock_begin(void) { return true; }
bool lock_end(void) { return true; }
#else
dbgsrv_dispatcher_t dispatcher(true);

static qmutex_t g_mutex = NULL;
static bool init_lock(void) { g_mutex = qmutex_create(); return g_mutex != NULL; }
bool lock_begin(void) { return qmutex_lock(g_mutex); }
bool lock_end(void) { return qmutex_unlock(g_mutex); }
#endif

//--------------------------------------------------------------------------
dbg_rpc_handler_t *g_global_server = NULL;

//--------------------------------------------------------------------------
// perform an action (func) on all debuggers
int for_all_debuggers(debmod_visitor_t &v)
{
  int code = 0;
  dispatcher.clients_list->lock();
  {
    client_handlers_list_t::storage_t::iterator it;
    for ( it = dispatcher.clients_list->storage.begin();
          it != dispatcher.clients_list->storage.end();
          ++it )
    {
      dbg_rpc_handler_t *h = (dbg_rpc_handler_t *) it->first;
      code = v.visit(h->get_debugger_instance());
      if ( code != 0 )
        break;
    }
  }
  dispatcher.clients_list->unlock();
  return code;
}

//-------------------------------------------------------------------------
dbgsrv_dispatcher_t::dbgsrv_dispatcher_t(bool multi_threaded)
  : base_dispatcher_t(multi_threaded),
    broken_conns_supported(false),
    on_broken_conn(BCH_DEFAULT)
{
  port_number = DEBUGGER_PORT_NUMBER;
}

//-------------------------------------------------------------------------
void dbgsrv_dispatcher_t::collect_cmdopts(cmdopts_t *out)
{
  struct ida_local ns_t
  {
    static void _set_dpassword(
            base_dispatcher_t *_d,
            const char *value)
    {
      ((dbgsrv_dispatcher_t *) _d)->server_password = value;
    }
    static void _set_dbroken_connections_behavior(
            base_dispatcher_t *_d,
            const char *value)
    {
      dbgsrv_dispatcher_t *d = (dbgsrv_dispatcher_t *) _d;
      char c = value != NULL ? value[0] : '\0';
      if ( c == '\0' )
        d->on_broken_conn = BCH_KEEP_DEBMOD;
      else if ( c == 'k' )
        d->on_broken_conn = BCH_KILL_PROCESS;
    }
  };

  static const cmdopt_t cmdopts[] =
    {
      { 'P', "Password", ns_t::_set_dpassword, 1 },
    };
  static const cmdopt_t bc_cmdopts[] =
    {
      {
        'k',
        "Behavior on broken connections\n"
        "    -k keep debugger session alive\n"
        "    -kk kill process before closing debugger module",
        ns_t::_set_dbroken_connections_behavior,
        -1
      },
    };

  base_dispatcher_t::collect_cmdopts(out);
  for ( size_t i = 0; i < qnumber(cmdopts); ++i )
    out->push_back(cmdopts[i]);

  if ( broken_conns_supported )
    for ( size_t i = 0; i < qnumber(bc_cmdopts); ++i )
      out->push_back(bc_cmdopts[i]);
}

//-------------------------------------------------------------------------
client_handler_t *dbgsrv_dispatcher_t::new_client_handler(idarpc_stream_t *_irs)
{
  dbg_rpc_handler_t *h = new dbg_rpc_handler_t(_irs, this);
  h->verbose = verbose;
  h->set_debugger_instance(create_debug_session());
  g_global_server = h;
  return h;
}

//-------------------------------------------------------------------------
void dbgsrv_dispatcher_t::shutdown_gracefully(int signum)
{
  base_dispatcher_t::shutdown_gracefully(signum);
  term_subsystem();
}


//--------------------------------------------------------------------------
// debugger remote server - TCP/IP mode
int NT_CDECL main(int argc, char *argv[])
{
#ifdef ENABLE_LOWCNDS
  init_idc();
#endif

  // call the debugger module to initialize its subsystem once
  if ( !init_lock() || !init_subsystem() )
  {
    lprintf("Could not initialize subsystem!");
    return -1;
  }

  qstring password;
  if ( qgetenv("IDA_DBGSRV_PASSWD", &password) )
    dispatcher.server_password = password;

  lprintf("IDA " SYSTEM SYSBITS " remote debug server(" __SERVER_TYPE__ ") "
          "v1.%d. Hex-Rays (c) 2004-2018\n", IDD_INTERFACE_VERSION);

  dispatcher.broken_conns_supported = debmod_t::reuse_broken_connections;
  dispatcher.apply_cmdopts(argc, argv);
  dispatcher.install_signal_handlers();
  dispatcher.dispatch();
}
