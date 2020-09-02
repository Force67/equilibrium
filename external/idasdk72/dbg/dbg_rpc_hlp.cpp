
#include <segment.hpp>
#include <typeinf.hpp>

#include "dbg_rpc_hlp.h"

//--------------------------------------------------------------------------
void append_memory_info(bytevec_t &s, const memory_info_t *meminf)
{
  append_ea64(s, meminf->sbase);
  append_ea64(s, meminf->start_ea - (meminf->sbase << 4));
  append_ea64(s, meminf->size());
  append_dd(s, meminf->perm | (meminf->bitness<<4));
  append_str(s, meminf->name.c_str());
  append_str(s, meminf->sclass.c_str());
}

//--------------------------------------------------------------------------
void extract_memory_info(const uchar **ptr, const uchar *end, memory_info_t *meminf)
{
  meminf->sbase    = unpack_ea64(ptr, end);
  meminf->start_ea = (meminf->sbase << 4) + unpack_ea64(ptr, end);
  meminf->end_ea   = meminf->start_ea + unpack_ea64(ptr, end);
  int v = unpack_dd(ptr, end);
  meminf->perm    = uchar(v) & SEGPERM_MAXVAL;
  meminf->bitness = uchar(v>>4);
  meminf->name    = extract_cstr(ptr, end);
  meminf->sclass  = extract_cstr(ptr, end);
}

//--------------------------------------------------------------------------
void append_scattered_segm(bytevec_t &s, const scattered_segm_t *ss)
{
  append_ea64(s, ss->start_ea);
  append_ea64(s, ss->end_ea);
  append_str(s, ss->name.c_str());
}

//--------------------------------------------------------------------------
void extract_scattered_segm(const uchar **ptr, const uchar *end, scattered_segm_t *ss)
{
  ss->start_ea = unpack_ea64(ptr, end);
  ss->end_ea = unpack_ea64(ptr, end);
  ss->name = extract_cstr(ptr, end);
}

//--------------------------------------------------------------------------
void append_process_info_vec(bytevec_t &s, const procinfo_vec_t *procs)
{
  size_t size = procs->size();
  append_dd(s, size);
  for ( size_t i = 0; i < size; i++ )
  {
    const process_info_t &pi = procs->at(i);
    append_dd(s, pi.pid);
    append_str(s, pi.name.c_str());
  }
}

//--------------------------------------------------------------------------
void extract_process_info_vec(const uchar **ptr, const uchar *end, procinfo_vec_t *procs)
{
  size_t size = unpack_dd(ptr, end);
  for ( size_t i = 0; i < size; i++ )
  {
    process_info_t &pi = procs->push_back();
    pi.pid = unpack_dd(ptr, end);
    pi.name = extract_cstr(ptr, end);
  }
}

//--------------------------------------------------------------------------
void append_module_info(bytevec_t &s, const modinfo_t *modinf)
{
  append_str(s, modinf->name);
  append_ea64(s, modinf->base);
  append_ea64(s, modinf->size);
  append_ea64(s, modinf->rebase_to);
}

//--------------------------------------------------------------------------
void extract_module_info(const uchar **ptr, const uchar *end, modinfo_t *modinf)
{
  modinf->name = extract_cstr(ptr, end);
  modinf->base = unpack_ea64(ptr, end);
  modinf->size = unpack_ea64(ptr, end);
  modinf->rebase_to = unpack_ea64(ptr, end);
}

//--------------------------------------------------------------------------
void append_exception(bytevec_t &s, const excinfo_t *e)
{
  append_dd(s, e->code);
  append_dd(s, e->can_cont);
  append_ea64(s, e->ea);
  append_str(s, e->info);
}

//--------------------------------------------------------------------------
void extract_exception(const uchar **ptr, const uchar *end, excinfo_t *exc)
{
  exc->code     = unpack_dd(ptr, end);
  exc->can_cont = unpack_dd(ptr, end) != 0;
  exc->ea       = unpack_ea64(ptr, end);
  exc->info     = extract_cstr(ptr, end);
}

//--------------------------------------------------------------------------
void extract_debug_event(const uchar **ptr, const uchar *end, debug_event_t *ev)
{
  ev->set_eid(event_id_t(unpack_dd(ptr, end)));
  ev->pid     = unpack_dd(ptr, end);
  ev->tid     = unpack_dd(ptr, end);
  ev->ea      = unpack_ea64(ptr, end);
  ev->handled = unpack_dd(ptr, end) != 0;
  switch ( ev->eid() )
  {
    case NO_EVENT:         // Not an interesting event
    case STEP:             // One instruction executed
    case PROCESS_DETACHED: // Detached from process
    default:
      break;
    case PROCESS_STARTED:  // New process started
    case PROCESS_ATTACHED: // Attached to running process
    case LIB_LOADED:       // New library loaded
      extract_module_info(ptr, end, &ev->modinfo());
      break;
    case PROCESS_EXITED:   // Process stopped
    case THREAD_EXITED:    // Thread stopped
      ev->exit_code() = unpack_dd(ptr, end);
      break;
    case BREAKPOINT:       // Breakpoint reached
      extract_breakpoint(ptr, end, &ev->bpt());
      break;
    case EXCEPTION:        // Exception
      extract_exception(ptr, end, &ev->exc());
      break;
    case THREAD_STARTED:   // New thread started
    case LIB_UNLOADED:     // Library unloaded
    case INFORMATION:      // User-defined information
      ev->info() = extract_cstr(ptr, end);
      break;
  }
}

//--------------------------------------------------------------------------
void append_debug_event(bytevec_t &s, const debug_event_t *ev)
{
  append_dd(s, ev->eid());
  append_dd(s, ev->pid);
  append_dd(s, ev->tid);
  append_ea64  (s, ev->ea);
  append_dd(s, ev->handled);
  switch ( ev->eid() )
  {
    case NO_EVENT:         // Not an interesting event
    case STEP:             // One instruction executed
    case PROCESS_DETACHED: // Detached from process
    default:
      break;
    case PROCESS_STARTED:  // New process started
    case PROCESS_ATTACHED: // Attached to running process
    case LIB_LOADED:       // New library loaded
      append_module_info(s, &ev->modinfo());
      break;
    case PROCESS_EXITED:   // Process stopped
    case THREAD_EXITED:    // Thread stopped
      append_dd(s, ev->exit_code());
      break;
    case BREAKPOINT:       // Breakpoint reached
      append_breakpoint(s, &ev->bpt());
      break;
    case EXCEPTION:        // Exception
      append_exception(s, &ev->exc());
      break;
    case THREAD_STARTED:   // New thread started
    case LIB_UNLOADED:     // Library unloaded
    case INFORMATION:      // User-defined information
      append_str(s, ev->info());
      break;
  }
}

//--------------------------------------------------------------------------
exception_info_t *extract_exception_info(
        const uchar **ptr,
        const uchar *end,
        int qty)
{
  exception_info_t *extable = NULL;
  if ( qty > 0 )
  {
    extable = OPERATOR_NEW(exception_info_t, qty);
    for ( int i=0; i < qty; i++ )
    {
      extable[i].code  = unpack_dd(ptr, end);
      extable[i].flags = unpack_dd(ptr, end);
      extable[i].name  = extract_cstr(ptr, end);
      extable[i].desc  = extract_cstr(ptr, end);
    }
  }
  return extable;
}

//--------------------------------------------------------------------------
void append_exception_info(bytevec_t &s, const exception_info_t *table, int qty)
{
  for ( int i=0; i < qty; i++ )
  {
    append_dd(s, table[i].code);
    append_dd(s, table[i].flags);
    append_str(s, table[i].name.c_str());
    append_str(s, table[i].desc.c_str());
  }
}

//--------------------------------------------------------------------------
void extract_call_stack(const uchar **ptr, const uchar *end, call_stack_t *trace)
{
  int n = unpack_dd(ptr, end);
  trace->resize(n);
  for ( int i=0; i < n; i++ )
  {
    call_stack_info_t &ci = (*trace)[i];
    ci.callea = unpack_ea64(ptr, end);
    ci.funcea = unpack_ea64(ptr, end);
    ci.fp     = unpack_ea64(ptr, end);
    ci.funcok = unpack_dd(ptr, end) != 0;
  }
}

//--------------------------------------------------------------------------
void append_call_stack(bytevec_t &s, const call_stack_t &trace)
{
  int n = trace.size();
  append_dd(s, n);
  for ( int i=0; i < n; i++ )
  {
    const call_stack_info_t &ci = trace[i];
    append_ea64(s, ci.callea);
    append_ea64(s, ci.funcea);
    append_ea64(s, ci.fp);
    append_dd(s, ci.funcok);
  }
}

//--------------------------------------------------------------------------
void extract_regobjs(
        const uchar **ptr,
        const uchar *end,
        regobjs_t *regargs,
        bool with_values)
{
  int n = unpack_dd(ptr, end);
  regargs->resize(n);
  for ( int i=0; i < n; i++ )
  {
    regobj_t &ro = (*regargs)[i];
    ro.regidx = unpack_dd(ptr, end);
    int size = unpack_dd(ptr, end);
    ro.value.resize(size);
    if ( with_values )
    {
      ro.relocate = unpack_dd(ptr, end);
      extract_memory(ptr, end, ro.value.begin(), size);
    }
  }
}

//--------------------------------------------------------------------------
static void extract_relobj(
        const uchar **ptr,
        const uchar *end,
        relobj_t *stkargs)
{
  int n = unpack_dd(ptr, end);
  stkargs->resize(n);
  extract_memory(ptr, end, stkargs->begin(), n);

  stkargs->base = unpack_ea64(ptr, end);

  n = unpack_dd(ptr, end);
  stkargs->ri.resize(n);
  extract_memory(ptr, end, stkargs->ri.begin(), n);
}

//--------------------------------------------------------------------------
void extract_appcall(
        const uchar **ptr,
        const uchar *end,
        regobjs_t *regargs,
        relobj_t *stkargs,
        regobjs_t *retregs)
{
  extract_regobjs(ptr, end, regargs, true);
  extract_relobj(ptr, end, stkargs);
  if ( retregs != NULL )
    extract_regobjs(ptr, end, retregs, false);
}

//--------------------------------------------------------------------------
void append_regobjs(bytevec_t &s, const regobjs_t &regargs, bool with_values)
{
  append_dd(s, regargs.size());
  for ( size_t i=0; i < regargs.size(); i++ )
  {
    const regobj_t &ro = regargs[i];
    append_dd(s, ro.regidx);
    append_dd(s, ro.value.size());
    if ( with_values )
    {
      append_dd(s, ro.relocate);
      append_memory(s, ro.value.begin(), ro.value.size());
    }
  }
}

//--------------------------------------------------------------------------
static void append_relobj(bytevec_t &s, const relobj_t &stkargs)
{
  append_dd(s, stkargs.size());
  append_memory(s, stkargs.begin(), stkargs.size());

  append_ea64(s, stkargs.base);

  append_dd(s, stkargs.ri.size());
  append_memory(s, stkargs.ri.begin(), stkargs.ri.size());
}

//--------------------------------------------------------------------------
void append_appcall(
        bytevec_t &s,
        const regobjs_t &regargs,
        const relobj_t &stkargs,
        const regobjs_t *retregs)
{
  append_regobjs(s, regargs, true);
  append_relobj(s, stkargs);
  if ( retregs != NULL )
    append_regobjs(s, *retregs, false);
}

//--------------------------------------------------------------------------
static void append_regval(bytevec_t &s, const regval_t *value)
{
  append_dd(s, value->rvtype+2);
  if ( value->rvtype == RVT_INT )
  {
    append_dq(s, value->ival+1);
  }
  else if ( value->rvtype == RVT_FLOAT )
  {
    append_memory(s, value->fval, sizeof(value->fval));
  }
  else
  {
    const bytevec_t &b = value->bytes();
    append_dd(s, b.size());
    append_memory(s, b.begin(), b.size());
  }
}

//--------------------------------------------------------------------------
static void extract_regval(const uchar **ptr, const uchar *end, regval_t *value)
{
  value->clear();
  value->rvtype = unpack_dd(ptr, end) - 2;
  if ( value->rvtype == RVT_INT )
  {
    value->ival = unpack_dq(ptr, end) - 1;
  }
  else if ( value->rvtype == RVT_FLOAT )
  {
    extract_memory(ptr, end, value->fval, sizeof(value->fval));
  }
  else
  {
    bytevec_t &b = value->_set_bytes();
    int size = unpack_dd(ptr, end);
    b.resize(size);
    extract_memory(ptr, end, b.begin(), size);
  }
}

//--------------------------------------------------------------------------
void extract_regvals(
        const uchar **ptr,
        const uchar *end,
        regval_t *values,
        int n,
        const uchar *regmap)
{
  for ( int i=0; i < n && *ptr < end; i++ )
    if ( regmap == NULL || test_bit(regmap, i) )
      extract_regval(ptr, end, values+i);
}

//--------------------------------------------------------------------------
void append_regvals(bytevec_t &s, const regval_t *values, int n, const uchar *regmap)
{
  for ( int i=0; i < n; i++ )
    if ( regmap == NULL || test_bit(regmap, i) )
      append_regval(s, values+i);
}

//--------------------------------------------------------------------------
void extract_debapp_attrs(
        const uchar **ptr,
        const uchar *end,
        debapp_attrs_t *attrs)
{
  attrs->addrsize = unpack_dd(ptr, end);
  attrs->platform = extract_cstr(ptr, end);
}

//--------------------------------------------------------------------------
void append_debapp_attrs(bytevec_t &s, const debapp_attrs_t *attrs)
{
  append_dd(s, attrs->addrsize);
  append_str(s, attrs->platform.c_str());
}
