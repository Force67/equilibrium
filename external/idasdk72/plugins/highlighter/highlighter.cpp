// Highlighter plugin v1.0
// Highlights executed instructions

// This plugin will display a colored box at the executed instructions.
// It will take into account only the instructions where the application
// has been suspended.

// http://www.hexblog.com/2005/11/the_highlighter.html

// Copyright 2005 Ilfak Guilfanov, <ig@hexblog.com>

#include <ida.hpp>
#include <idp.hpp>
#include <dbg.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <set>

//--------------------------------------------------------------------------
// List of executed addresses
typedef std::set<ea_t> easet_t;
static easet_t execset;

// To manage the processed event of the debugger
struct my_post_events_t : public post_event_visitor_t
{
  virtual ssize_t idaapi handle_post_event(
          ssize_t code,
          int notification_code,
          va_list va);
};
static my_post_events_t my_post_events;

//--------------------------------------------------------------------------
// A sample how to generate user-defined line prefixes
static const int prefix_width = 1;
static const char highlight_prefix[] = { COLOR_INV, ' ', COLOR_INV, 0 };

static void idaapi get_user_defined_prefix(
        qstring *buf,
        ea_t ea,
        int lnnum,
        int indent,
        const char *line)
{
  buf->qclear();        // empty prefix by default

  // We want to display the prefix only the lines which
  // contain the instruction itself

  if ( indent != -1 )
    return;           // a directive
  if ( line[0] == '\0' )
    return;        // empty line
  if ( tag_advance(line,1)[-1] == ash.cmnt[0] )
    return; // comment line...

  // We don't want the prefix to be printed again for other lines of the
  // same instruction/data. For that we remember the line number
  // and compare it before generating the prefix

  static ea_t old_ea = BADADDR;
  static int old_lnnum = 0;
  if ( old_ea == ea && old_lnnum == lnnum )
    return;

  if ( execset.find(ea) != execset.end() )
    *buf = highlight_prefix;

  // Remember the address and line number we produced the line prefix for:
  old_ea = ea;
  old_lnnum = lnnum;
}

//--------------------------------------------------------------------------
ssize_t idaapi my_post_events_t::handle_post_event(
        ssize_t retcode,
        int notification_code,
        va_list va)
{
  switch ( notification_code )
  {
    case debugger_t::ev_get_debug_event:
      {
        gdecode_t *code = va_arg(va, gdecode_t *);
        debug_event_t *event = va_arg(va, debug_event_t *);
        if ( *code == GDE_ONE_EVENT )    // got an event?
          execset.insert(event->ea);
      }
      break;
  }
  return retcode;
}

//--------------------------------------------------------------------------
bool idaapi run(size_t)
{
  info("AUTOHIDE NONE\n"
       "This is the highlighter plugin.\n"
       "It highlights executed instructions if a debug event occurs at them.\n"
       "The plugins is fully automatic and has no parameters.\n");
  return true;
}

//--------------------------------------------------------------------------
static ssize_t idaapi callback(void *, int notification_code, va_list)
{
  // We set our debug event handler at the beginning and remove it at the end
  // of a debug session
  switch ( notification_code )
  {
    case dbg_process_start:
    case dbg_process_attach:
      set_user_defined_prefix(prefix_width, get_user_defined_prefix);
      register_post_event_visitor(HT_IDD, my_post_events, &PLUGIN);
      break;
    case dbg_process_exit:
      unregister_post_event_visitor(HT_IDD, my_post_events);
      set_user_defined_prefix(0, NULL);
      execset.clear();
      break;
  }
  return 0;
}

//--------------------------------------------------------------------------
int idaapi init(void)
{
  hook_to_notification_point(HT_DBG, callback);
  return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
void idaapi term(void)
{
  unhook_from_notification_point(HT_DBG, callback);
}

//--------------------------------------------------------------------------
static const char wanted_name[] = "Highlighter";
static const char wanted_hotkey[] = "";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,                    // plugin flags
  init,                 // initialize

  term,                 // terminate. this pointer may be NULL.

  run,                  // invoke plugin

  wanted_name,          // long comment about the plugin
                        // it could appear in the status line
                        // or as a hint

  wanted_name,          // multiline help about the plugin

  wanted_name,          // the preferred short name of the plugin
  wanted_hotkey         // the preferred hotkey to run the plugin
};
