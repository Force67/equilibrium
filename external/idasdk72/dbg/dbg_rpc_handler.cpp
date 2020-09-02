#include <limits.h>

#include <pro.h>
#include <typeinf.hpp>
#include <diskio.hpp>
#include <network.hpp>      // otherwise can not compile win32_remote.bpr
#include <err.h>

#include "server.h"

//--------------------------------------------------------------------------
// another copy of this function (for local debugging) is defined in common_local_impl.cpp
int send_ioctl(
        rpc_engine_t *srv,
        int fn,
        const void *buf,
        size_t size,
        void **poutbuf,
        ssize_t *poutsize)
{
  return srv->send_ioctl(fn, buf, size, poutbuf, poutsize);
}

//--------------------------------------------------------------------------
AS_PRINTF(3, 0) ssize_t dvmsg(int code, rpc_engine_t *rpc, const char *format, va_list va)
{
  if ( code == 0 )
    code = RPC_MSG;
  else if ( code > 0 )
    code = RPC_WARNING;
  else
    code = RPC_ERROR;

  bytevec_t req = prepare_rpc_packet((uchar)code);

  char buf[MAXSTR];
  qvsnprintf(buf, sizeof(buf), format, va);
  append_str(req, buf);

  qfree(rpc->send_request_and_receive_reply(req));
  if ( code == RPC_ERROR )
    exit(1);
  return strlen(buf);
}

//--------------------------------------------------------------------------
void report_idc_error(rpc_engine_t *rpc, ea_t ea, error_t code, ssize_t errval, const char *errprm)
{
  if ( code == eOS )
  {
    dbg_rpc_handler_t *h = (dbg_rpc_handler_t *)rpc;
    errval = h->get_debugger_instance()->get_system_specific_errno();
  }

  bytevec_t req = prepare_rpc_packet(RPC_REPORT_IDC_ERROR);
  append_ea64(req, ea);
  append_dd(req, code);
  if ( (const char *)errval == errprm )
  {
    append_db(req, 1);
    append_str(req, errprm);
  }
  else
  {
    append_db(req, 0);
    append_ea64(req, errval);
  }
  qfree(rpc->send_request_and_receive_reply(req));
}

//--------------------------------------------------------------------------
debmod_t *dbg_rpc_handler_t::get_debugger_instance()
{
  return dbg_mod;   //lint !e1535 !e1536 exposes lower access member
}

//--------------------------------------------------------------------------
void dbg_rpc_handler_t::prepare_broken_connection(void)
{
  if ( debmod_t::reuse_broken_connections )
  {
    if ( !dbg_mod->dbg_prepare_broken_connection() )
      dmsg("Error preparing debugger server to handle a broken connection\n");
  }
}

//--------------------------------------------------------------------------
//                        dbg_rpc_handler_t
//--------------------------------------------------------------------------
dbg_rpc_handler_t::dbg_rpc_handler_t(
        idarpc_stream_t *_irs,
        dbgsrv_dispatcher_t *_dispatcher)
  : client_handler_t(_irs),
    dbg_rpc_engine_t(/*is_client=*/ false),
    dbg_mod(NULL),
    dispatcher(_dispatcher)
{
  clear_channels(); //lint -esym(1566,dbg_rpc_handler_t::channels) not inited
  struct ida_local lambda_t
  {
    static int ioctl(rpc_engine_t *rpc, int fn, const void *buf, size_t size, void **out, ssize_t *outsz)
    {
      dbg_rpc_handler_t *serv = (dbg_rpc_handler_t *) rpc;
      if ( fn >= MIN_SERVER_IOCTL )
        return serv->handle_server_ioctl(fn, buf, size, out, outsz);
      else
        return serv->get_debugger_instance()->handle_ioctl(fn, buf, size, out, outsz);
    }

    static progress_loop_ctrl_t recv_data_iter(bool, size_t, size_t, void *ud)
    {
      dbg_rpc_handler_t *eng = (dbg_rpc_handler_t *) ud;
      bool performed = false;
      int code = eng->on_recv_packet_progress(&performed);
      if ( performed )
        return code == 0 ? plc_skip_iter : plc_cancel;
      else
        return plc_proceed;
    }
  };

  set_ioctl_handler(lambda_t::ioctl);
  irs_set_progress_cb(irs, 100, lambda_t::recv_data_iter, this);
}

//--------------------------------------------------------------------------
dbg_rpc_handler_t::~dbg_rpc_handler_t()
{
  //lint -e(1506) Call to virtual function 'dbg_rpc_handler_t::get_broken_connection(void)' within a constructor or destructor
  if ( !get_broken_connection() )
    delete dbg_mod; // the connection is not broken, delete the debugger instance

  //lint -esym(1579,dbg_rpc_handler_t::dbg_mod) pointer member might have been freed by a separate function
  clear_channels();

  dispatcher = NULL;
}

//------------------------------------------------------------------------
// Function safe against time slicing attack, comparing time depends only on str length
static bool password_matches(const char *str, const char *pass)
{
  int str_length = strlen(str);
  int pass_length = strlen(pass);
  int res = str_length ^ pass_length;
  if ( pass_length != 0 )
  {
    for ( int i = 0; i < str_length; i++ )
      res |= (pass[i % pass_length] ^ str[i]);
  }
  return res == 0;
}

//-------------------------------------------------------------------------
bool dbg_rpc_handler_t::handle()
{
  bytevec_t req = prepare_rpc_packet(RPC_OPEN);
  append_dd(req, IDD_INTERFACE_VERSION);
  append_dd(req, DEBUGGER_ID);
  append_dd(req, sizeof(ea_t));

  bool send_response = false;
  rpc_packet_t *rp = send_request_and_receive_reply(req, PREQ_MUST_LOGIN);
  bool ok = rp != NULL;
  if ( ok )
  {
    send_response = true;

    // Answer is beyond the rpc_packet_t buffer
    const uchar *answer = (uchar *)(rp+1);
    const uchar *end = answer + rp->length;

    ok = unpack_dd(&answer, end) != 0;
    if ( !ok )
    {
      lprintf("[%d] Incompatible IDA version\n", session_id);
      send_response = false;
    }
    else if ( !dispatcher->server_password.empty() )
    {
      char *pass = extract_cstr(&answer, end);
      if ( !password_matches(pass, dispatcher->server_password.c_str()) )
      {
        lprintf("[%d] Bad password\n", session_id);
        ok = false;
      }
    }

    qfree(rp);
  }
  else
  {
    lprintf("[%d] Could not establish the connection\n", session_id);
  }

  if ( send_response )
  {
    req = prepare_rpc_packet(RPC_OK);
    append_dd(req, ok);
    send_data(req);

    // remove reception timeout on the server side
    recv_timeout = -1;
    logged_in = true;

    if ( ok )
    {
      // the main loop: handle client requests until it drops the connection
      // or sends us RPC_OK (see rpc_debmod_t::close_remote)
      bytevec_t empty;
      rpc_packet_t *packet = send_request_and_receive_reply(empty);
      if ( packet != NULL )
        qfree(packet);
    }
  }
  network_error = false;

  bool preserve_server = false;
  if ( get_broken_connection() )
  {
    if ( dispatcher->on_broken_conn == BCH_KEEP_DEBMOD )
    {
      term_irs();
      lprintf("[%d] Debugged session entered into sleeping mode\n", session_id);
      prepare_broken_connection();
      preserve_server = true;
    }
    else
    {
      if ( dispatcher->on_broken_conn == BCH_KILL_PROCESS )
      {
        int pid = get_debugger_instance()->pid;
        if ( pid > 0 )
        {
          lprintf("[%d] Killing debugged process %d\n",
                  session_id, get_debugger_instance()->pid);
          int code = kill_process();
          if ( code != 0 )
            lprintf("[%d] Failed to kill process after %d seconds. Giving up\n",
                    session_id, code);
        }
      }
      goto TERM_DEBMOD;
    }
  }
  else
  {
TERM_DEBMOD:
    get_debugger_instance()->dbg_term();
    term_irs();
  }

  return !preserve_server;
}

//--------------------------------------------------------------------------
void dbg_rpc_handler_t::set_debugger_instance(debmod_t *instance)
{
  dbg_mod = instance;
  dbg_mod->rpc = this;
}

//--------------------------------------------------------------------------
#ifdef VERBOSE_ENABLED
static const char *bptcode2str(uint code)
{
  static const char *const strs[] =
  {
    "BPT_OK",
    "BPT_INTERNAL_ERR",
    "BPT_BAD_TYPE",
    "BPT_BAD_ALIGN",
    "BPT_BAD_ADDR",
    "BPT_BAD_LEN",
    "BPT_TOO_MANY",
    "BPT_READ_ERROR",
    "BPT_WRITE_ERROR",
    "BPT_SKIP",
    "BPT_PAGE_OK",
  };
  if ( code >= qnumber(strs) )
    return "?";
  return strs[code];
}
#endif

//--------------------------------------------------------------------------
int dbg_rpc_handler_t::rpc_update_bpts(
        const uchar *ptr,
        const uchar *end,
        bytevec_t &req)
{
  update_bpt_vec_t bpts;
  int nadd = unpack_dd(&ptr, end);
  int ndel = unpack_dd(&ptr, end);

  if ( nadd < 0 || ndel < 0 || INT_MAX - ndel < nadd )
  {
    append_dd(req, 0);
    verb(("update_bpts(nadd=%d, ndel=%d) => 0 (incorrect values)\n", nadd, ndel));
    return 0;
  }

  bpts.resize(nadd+ndel);
  ea_t ea = 0;
  update_bpt_vec_t::iterator b;
  update_bpt_vec_t::iterator bend = bpts.begin() + nadd;
  for ( b=bpts.begin(); b != bend; ++b )
  {
    b->code = BPT_OK;
    b->ea = ea + unpack_ea64(&ptr, end); ea = b->ea;
    b->size = unpack_dd(&ptr, end);
    b->type = unpack_dd(&ptr, end);
    b->pid  = unpack_dd(&ptr, end);
    b->tid  = unpack_dd(&ptr, end);
  }

  ea = 0;
  bend += ndel;
  for ( ; b != bend; ++b )
  {
    b->ea = ea + unpack_ea64(&ptr, end); ea = b->ea;
    uchar len = unpack_db(&ptr, end);
    if ( len > 0 )
    {
      b->orgbytes.resize(len);
      extract_memory(&ptr, end, b->orgbytes.begin(), len);
    }
    b->type = unpack_dd(&ptr, end);
    b->pid  = unpack_dd(&ptr, end);
    b->tid  = unpack_dd(&ptr, end);
  }

#ifdef VERBOSE_ENABLED
  for ( b=bpts.begin()+nadd; b != bend; ++b )
    verb(("del_bpt(ea=%a, type=%d orgbytes.size=%" FMT_Z " size=%d)\n",
          b->ea, b->type, b->orgbytes.size(), b->type != BPT_SOFT ? b->size : 0));
#endif

  int nbpts;
  qstring errbuf;
  drc_t drc = dbg_mod->dbg_update_bpts(&nbpts, bpts.begin(), nadd, ndel, &errbuf);

  bend = bpts.begin() + nadd;
#ifdef VERBOSE_ENABLED
  for ( b=bpts.begin(); b != bend; ++b )
    verb(("add_bpt(ea=%a type=%d len=%d) => %s\n", b->ea, b->type, b->size, bptcode2str(b->code)));
#endif

  append_dd(req, drc);
  append_dd(req, nbpts);
  for ( b=bpts.begin(); b != bend; ++b )
  {
    append_db(req, b->code);
    if ( b->code == BPT_OK && b->type == BPT_SOFT )
    {
      append_db(req, b->orgbytes.size());
      append_memory(req, b->orgbytes.begin(), b->orgbytes.size());
    }
  }

  bend += ndel;
  for ( ; b != bend; ++b )
  {
    append_db(req, b->code);
    verb(("del_bpt(ea=%a) => %s\n", b->ea, bptcode2str(b->code)));
  }

  if ( drc != DRC_OK )
    append_str(req, errbuf);
  return drc;
}

//--------------------------------------------------------------------------
void dbg_rpc_handler_t::rpc_update_lowcnds(
        const uchar *ptr,
        const uchar *end,
        bytevec_t &req)
{
  ea_t ea = 0;
  lowcnd_vec_t lowcnds;
  int nlowcnds = unpack_dd(&ptr, end);
  lowcnds.resize(nlowcnds);
  lowcnd_t *lc = lowcnds.begin();
  for ( int i=0; i < nlowcnds; i++, lc++ )
  {
    lc->compiled = false;
    lc->ea = ea + unpack_ea64(&ptr, end); ea = lc->ea;
    lc->cndbody = extract_cstr(&ptr, end);
    if ( !lc->cndbody.empty() )
    {
      lc->size = 0;
      lc->type = unpack_dd(&ptr, end);
      if ( lc->type != BPT_SOFT )
        lc->size = unpack_dd(&ptr, end);
      int norg = unpack_db(&ptr, end);
      if ( norg > 0 )
      {
        lc->orgbytes.resize(norg);
        extract_memory(&ptr, end, lc->orgbytes.begin(), norg);
      }
      lc->cmd.ea = unpack_ea64(&ptr, end);
      if ( lc->cmd.ea != BADADDR )
        extract_memory(&ptr, end, &lc->cmd, sizeof(lc->cmd));
    }
    verb(("update_lowcnd(ea=%a cnd=%s)\n", ea, lc->cndbody.c_str()));
  }
  int nupdated;
  qstring errbuf;
  drc_t drc = dbg_mod->dbg_update_lowcnds(&nupdated, lowcnds.begin(), nlowcnds, &errbuf);
  verb(("  update_lowcnds => %d\n", drc));

  append_dd(req, drc);
  append_dd(req, nupdated);
  if ( drc != DRC_OK )
    append_str(req, errbuf);
}

//--------------------------------------------------------------------------
bool dbg_rpc_handler_t::check_broken_connection(pid_t pid)
{
  bool result = false;
  dispatcher->clients_list->lock();
  client_handlers_list_t::storage_t::iterator p;
  for ( p = dispatcher->clients_list->storage.begin();
        p != dispatcher->clients_list->storage.end();
        ++p )
  {
    dbg_rpc_handler_t *h = (dbg_rpc_handler_t *) p->first;
    if ( h == this )
      continue;

    debmod_t *d = h->get_debugger_instance();
    if ( d->broken_connection && d->pid == pid && d->dbg_continue_broken_connection(pid) )
    {
      dbg_mod->dbg_term();
      delete dbg_mod;
      dbg_mod = d;
      result = true;
      verb(("reusing previously broken debugging session\n"));

#ifndef __SINGLE_THREADED_SERVER__
      qthread_t thr = p->second;

      // free thread
      if ( thr != NULL )
        qthread_free(thr);
#endif

      h->term_irs();
      dispatcher->clients_list->storage.erase(p);
      delete h;

      d->broken_connection = false;
      break;
    }
  }
  dispatcher->clients_list->unlock();
  return result;
}

//-------------------------------------------------------------------------
int dbg_rpc_handler_t::handle_server_ioctl(
        int fn,
        const void *buf,
        size_t size,
        void **out,
        ssize_t *outsz)
{
  int code = -1;
  verb(("handle_server_ioctl(fn=%d, bufsize=%" FMT_Z ").\n", fn, size));
  return code;
}

//-------------------------------------------------------------------------
int dbg_rpc_handler_t::on_recv_packet_progress(bool *performed)
{
  *performed = poll_debug_events;
  return poll_debug_events ? poll_events(TIMEOUT) : 0;
}

//--------------------------------------------------------------------------
drc_t dbg_rpc_handler_t::rpc_attach_process(
        const uchar *ptr,
        const uchar *end,
        qstring *errbuf)
{
  pid_t pid = unpack_dd(&ptr, end);
  int event_id = unpack_dd(&ptr, end);
  int flags = unpack_dd(&ptr, end);
  drc_t drc = check_broken_connection(pid)
            ? DRC_OK
            : dbg_mod->dbg_attach_process(pid, event_id, flags, errbuf);
  verb(("attach_process(pid=%d, evid=%d) => %d\n", pid, event_id, drc));
  return drc;
}

//-------------------------------------------------------------------------
void dbg_rpc_handler_t::append_start_or_attach(bytevec_t &req, drc_t drc, const qstring &errbuf) const
{
  append_dd(req, drc);
  if ( drc > DRC_NONE )
  {
    debapp_attrs_t attrs;
    dbg_mod->dbg_get_debapp_attrs(&attrs);
    append_debapp_attrs(req, &attrs);
  }
  else
  {
    append_str(req, errbuf);
  }
}

//-------------------------------------------------------------------------
void dbg_rpc_handler_t::shutdown_gracefully(int /*signum*/)
{
  debmod_t *d = get_debugger_instance();
  if ( d != NULL )
    d->dbg_exit_process(NULL); // kill the process instead of letting it run in wild
}

//--------------------------------------------------------------------------
// performs requests on behalf of a remote client
// client -> server
#ifdef __UNIX__
#  define PERM_0755 (S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#  define IS_SUBPATH strneq
#else
#  define PERM_0755 0755
#  define IS_SUBPATH strnieq
#endif
bytevec_t dbg_rpc_handler_t::on_send_request_interrupt(const rpc_packet_t *rp)
{
  // While the server is performing a request, it should not poll
  // for debugger events
  bool saved_poll_mode = poll_debug_events;
  poll_debug_events = false;

  const uchar *ptr = (const uchar *)(rp + 1);
  const uchar *end = ptr + rp->length;
  bytevec_t req = prepare_rpc_packet(RPC_OK);
#if defined(__EXCEPTIONS) || defined(__NT__)
  try
#endif
  {
    switch ( rp->code )
    {
      case RPC_INIT:
        {
          dbg_mod->debugger_flags = unpack_dd(&ptr, end);
          bool debug_debugger = unpack_dd(&ptr, end) != 0;
          if ( debug_debugger )
            verbose = true;

          dbg_mod->dbg_set_debugging(debug_debugger);
          qstring errbuf;
          int result = dbg_mod->dbg_init(&errbuf);
          verb(("init(debug_debugger=%d) => %d\n", debug_debugger, result));
          append_dd(req, result);
          if ( result < 0 )
            append_str(req, errbuf);
        }
        break;

      case RPC_TERM:
        // Do not dbg_term() here, as it will be called
        // at the end of server.cpp's handle_single_session(),
        // right after this.
        // dbg_mod->dbg_term();
        // verb(("term()\n"));
        break;

      case RPC_GET_PROCESSES:
        {
          procinfo_vec_t procs;
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_get_processes(&procs, &errbuf);
          append_dd(req, drc);
          if ( drc == DRC_OK )
            append_process_info_vec(req, &procs);
          else
            append_str(req, errbuf);
          verb(("get_processes() => %d\n", drc));
        }
        break;

      case RPC_DETACH_PROCESS:
        {
          drc_t drc = dbg_mod->dbg_detach_process();
          append_dd(req, drc);
          verb(("detach_process() => %d\n", drc));
        }
        break;

      case RPC_START_PROCESS:
        {
          char *path = extract_cstr(&ptr, end);
          char *args = extract_cstr(&ptr, end);
          char *sdir = extract_cstr(&ptr, end);
          int flags  = unpack_dd(&ptr, end);
          char *input= extract_cstr(&ptr, end);
          uint32 crc32= unpack_dd(&ptr, end);
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_start_process(path, args, sdir, flags, input, crc32, &errbuf);
          verb(("start_process(path=%s args=%s flags=%s%s%s\n"
            "              sdir=%s\n"
            "              input=%s crc32=%x) => %d\n",
            path, args,
            flags & DBG_PROC_IS_DLL ? " is_dll" : "",
            flags & DBG_PROC_IS_GUI ? " under_gui" : "",
            flags & DBG_HIDE_WINDOW ? " hide_window" : "",
            sdir,
            input, crc32,
            drc));
          append_start_or_attach(req, drc, errbuf);
        }
        break;

      case RPC_GET_DEBUG_EVENT:
        {
          int timeout_ms = unpack_dd(&ptr, end);
          gdecode_t result = GDE_NO_EVENT;
          if ( !has_pending_event )
            result = dbg_mod->dbg_get_debug_event(&ev, timeout_ms);
          append_dd(req, result);
          if ( result >= GDE_ONE_EVENT )
          {
            append_debug_event(req, &ev);
            verb(("got event: %s\n", debug_event_str(&ev)));
          }
          else if ( !has_pending_event )
          {
            saved_poll_mode = true;
          }
          verbev(("get_debug_event(timeout=%d) => %d (has_pending=%d, willpoll=%d)\n", timeout_ms, result, has_pending_event, saved_poll_mode));
        }
        break;

      case RPC_ATTACH_PROCESS:
        {
          qstring errbuf;
          append_start_or_attach(req, rpc_attach_process(ptr, end, &errbuf), errbuf);
        }
        break;

      case RPC_PREPARE_TO_PAUSE_PROCESS:
        {
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_prepare_to_pause_process(&errbuf);
          verb(("prepare_to_pause_process() => %d\n", drc));
          append_dd(req, drc);
          if ( drc < DRC_NONE )
            append_str(req, errbuf);
        }
        break;

      case RPC_EXIT_PROCESS:
        {
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_exit_process(&errbuf);
          verb(("exit_process() => %d\n", drc));
          append_dd(req, drc);
          if ( drc < DRC_NONE )
            append_str(req, errbuf);
        }
        break;

      case RPC_CONTINUE_AFTER_EVENT:
        {
          extract_debug_event(&ptr, end, &ev);
          drc_t drc = dbg_mod->dbg_continue_after_event(&ev);
          verb(("continue_after_event(...) => %d\n", drc));
          append_dd(req, drc);
        }
        break;

      case RPC_STOPPED_AT_DEBUG_EVENT:
        {
          bool dlls_added = unpack_db(&ptr, end) != 0;
          bool ask_thr_names = unpack_db(&ptr, end) != 0;
          import_infos_t infos;
          thread_name_vec_t thr_names;
          dbg_mod->dbg_stopped_at_debug_event(&infos, dlls_added, ask_thr_names ? &thr_names : NULL);
          process_import_requests(infos);
          name_info_t *ni = dbg_mod->get_debug_names();
          int err = RPC_OK;
          if ( ni != NULL )
          {
            err = send_debug_names_to_ida(ni->addrs.begin(), ni->names.begin(), (int)ni->addrs.size());
            dbg_mod->clear_debug_names();
          }
          if ( ask_thr_names )
          {
            uint32 n = thr_names.size();
            append_dd(req, n);
            for ( int i=0; i < n; ++i )
            {
              thread_name_t &tn = thr_names[i];
              append_dd(req, tn.tid);
              append_str(req, tn.name);
            }
          }
        }
        break;

      case RPC_TH_SUSPEND:
        {
          thid_t tid = unpack_dd(&ptr, end);
          drc_t drc = dbg_mod->dbg_thread_suspend(tid);
          verb(("thread_suspend(tid=%d) => %d\n", tid, drc));
          append_dd(req, drc);
        }
        break;

      case RPC_TH_CONTINUE:
        {
          thid_t tid = unpack_dd(&ptr, end);
          drc_t drc = dbg_mod->dbg_thread_continue(tid);
          verb(("thread_continue(tid=%d) => %d\n", tid, drc));
          append_dd(req, drc);
        }
        break;

      case RPC_SET_RESUME_MODE:
        {
          thid_t tid = unpack_dd(&ptr, end);
          resume_mode_t resmod = resume_mode_t(unpack_dd(&ptr, end));
          drc_t drc = dbg_mod->dbg_set_resume_mode(tid, resmod);
          verb(("set_resume_mode(tid=%d, resmod=%d) => %d\n", tid, resmod, drc));
          append_dd(req, drc);
        }
        break;

      case RPC_READ_REGS:
        {
          drc_t drc = DRC_NONE;
          qstring errbuf;
          bytevec_t regmap;
          regval_t *values = NULL;
          thid_t tid  = unpack_dd(&ptr, end);
          int clsmask = unpack_dd(&ptr, end);
          int nregs   = unpack_dd(&ptr, end);
          if ( nregs <= 0 || nregs > dbg_mod->nregs )
          {
            errbuf.sprnt("read_regs(tid=%d, mask=%x, nregs=%d) => 0 "
                         "(incorrect nregs, should be in range 0..%d)\n",
                         tid, clsmask, nregs, dbg_mod->nregs);
          }
          else
          {
            regmap.resize((nregs+7)/8);
            extract_memory(&ptr, end, regmap.begin(), regmap.size());
            values = OPERATOR_NEW(regval_t, dbg_mod->nregs);
            drc = dbg_mod->dbg_read_registers(tid, clsmask, values, &errbuf);
            verb(("read_regs(tid=%d, mask=%x) => %d\n", tid, clsmask, drc));
          }
          append_dd(req, drc);
          if ( drc == DRC_OK )
            append_regvals(req, values, nregs, regmap.begin());
          else
            append_str(req, errbuf);
          delete[] values;
        }
        break;

      case RPC_WRITE_REG:
        {
          thid_t tid = unpack_dd(&ptr, end);
          int reg_idx = unpack_dd(&ptr, end);
          regval_t value;
          extract_regvals(&ptr, end, &value, 1, NULL);
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_write_register(tid, reg_idx, &value, &errbuf);
          verb(("write_reg(tid=%d) => %d\n", tid, drc));
          append_dd(req, drc);
          if ( drc < DRC_NONE )
            append_str(req, errbuf);
        }
        break;

      case RPC_GET_SREG_BASE:
        {
          thid_t tid = unpack_dd(&ptr, end);
          int sreg_value = unpack_dd(&ptr, end);
          ea_t ea;
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_thread_get_sreg_base(&ea, tid, sreg_value, &errbuf);
          verb(("get_thread_sreg_base(tid=%d, %d) => %a\n", tid, sreg_value, drc == DRC_OK ? ea : BADADDR));
          append_dd(req, drc);
          if ( drc == DRC_OK )
            append_ea64(req, ea);
          else
            append_str(req, errbuf);
        }
        break;

      case RPC_SET_EXCEPTION_INFO:
        {
          int qty = unpack_dd(&ptr, end);
          exception_info_t *extable = extract_exception_info(&ptr, end, qty);
          dbg_mod->dbg_set_exception_info(extable, qty);
          delete [] extable;
          verb(("set_exception_info(qty=%d)\n", qty));
        }
        break;

      case RPC_GET_MEMORY_INFO:
        {
          meminfo_vec_t areas;
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_get_memory_info(areas, &errbuf);
          int qty = areas.size();
          verb(("get_memory_info() => %d (qty=%d)\n", drc, qty));
          append_dd(req, drc + (-DRC_IDBSEG));
          if ( drc == DRC_OK )
          {
            append_dd(req, qty);
            for ( int i=0; i < qty; i++ )
              append_memory_info(req, &areas[i]);
          }
          else
          {
            append_str(req, errbuf);
          }
        }
        break;

      case RPC_GET_SCATTERED_IMAGE:
        {
          ea_t base = unpack_ea64(&ptr, end);
          scattered_image_t si;
          int result = dbg_mod->dbg_get_scattered_image(si, base);
          int qty = si.size();
          verb(("get_scattered_image(base=%a) => %d (qty=%d)\n", base, result, qty));
          append_dd(req, result+2);
          if ( result > 0 )
          {
            append_dd(req, qty);
            for ( int i=0; i < qty; i++ )
              append_scattered_segm(req, &si[i]);
          }
        }
        break;

      case RPC_GET_IMAGE_UUID:
        {
          ea_t base = unpack_ea64(&ptr, end);
          bytevec_t uuid;
          bool result = dbg_mod->dbg_get_image_uuid(&uuid, base);
          int qty = uuid.size();
          verb(("get_image_uuid(base=%a) => %d (qty=%d)\n", base, result, qty));
          append_dd(req, result);
          if ( result )
            append_buf(req, uuid.begin(), qty);
        }
        break;

      case RPC_GET_SEGM_START:
        {
          ea_t base = unpack_ea64(&ptr, end);
          const char *segname = extract_cstr(&ptr, end);
          ea_t result = dbg_mod->dbg_get_segm_start(base, segname);
          verb(("get_segm_start(base=%a, segname=%s) => %a\n", base, segname, result));
          append_ea64(req, result);
        }
        break;

      case RPC_READ_MEMORY:
        {
          ea_t ea = unpack_ea64(&ptr, end);
          size_t size = unpack_dd(&ptr, end);
          uchar *buf = new uchar[size];
          qstring errbuf;
          ssize_t result = dbg_mod->dbg_read_memory(ea, buf, size, &errbuf);
          verb(("read_memory(ea=%a size=%" FMT_Z ") => %" FMT_ZS, ea, size, result));
          if ( result > 0 && size == 1 )
            verb((" (0x%02X)\n", *buf));
          else
            verb(("\n"));
          append_dd(req, uint32(result));
          if ( result > 0 )
            append_memory(req, buf, result);
          else
            append_str(req, errbuf);
          delete[] buf;
        }
        break;

      case RPC_WRITE_MEMORY:
        {
          ea_t ea = unpack_ea64(&ptr, end);
          size_t size = unpack_dd(&ptr, end);
          uchar *buf = new uchar[size];
          extract_memory(&ptr, end, buf, size);
          qstring errbuf;
          ssize_t result = dbg_mod->dbg_write_memory(ea, buf, size, &errbuf);
          verb(("write_memory(ea=%a size=%" FMT_Z ") => %" FMT_ZS, ea, size, result));
          if ( result && size == 1 )
            verb((" (0x%02X)\n", *buf));
          else
            verb(("\n"));
          append_dd(req, uint32(result));
          if ( result <= 0 )
            append_str(req, errbuf);
          delete[] buf;
        }
        break;

      case RPC_ISOK_BPT:
        {
          bpttype_t type = unpack_dd(&ptr, end);
          ea_t ea        = unpack_ea64(&ptr, end);
          int len        = unpack_dd(&ptr, end) - 1;
          int result = dbg_mod->dbg_is_ok_bpt(type, ea, len);
          verb(("isok_bpt(type=%d ea=%a len=%d) => %d\n", type, ea, len, result));
          append_dd(req, result);
        }
        break;

      case RPC_UPDATE_BPTS:
        {
          int ret = rpc_update_bpts(ptr, end, req);
          if ( ret == 0 )
            verb(("rpc_update_bpts failed!\n"));
        }
        break;

      case RPC_UPDATE_LOWCNDS:
        rpc_update_lowcnds(ptr, end, req);
        break;

      case RPC_EVAL_LOWCND:
        {
          thid_t tid = unpack_dd(&ptr, end);
          ea_t ea    = unpack_ea64(&ptr, end);
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_eval_lowcnd(tid, ea, &errbuf);
          append_dd(req, drc);
          if ( drc != DRC_OK )
            append_str(req, errbuf);
          verb(("eval_lowcnd(tid=%d, ea=%a) => %d\n", tid, ea, drc));
        }
        break;

      case RPC_OPEN_FILE:
        {
          char *path = extract_cstr(&ptr, end);
          bool readonly = unpack_dd(&ptr, end) != 0;
          int64 fsize = 0;
          int fn = find_free_channel();
          if ( fn != -1 )
          {
            if ( readonly )
            {
              channels[fn] = fopenRB(path);
            }
            else
            {
              char dir[QMAXPATH];
              if ( qdirname(dir, sizeof(dir), path) && !qisdir(dir) )
              {
                char absdir[QMAXPATH];
                qmake_full_path(absdir, sizeof(absdir), dir);
                char cwd[QMAXPATH];
                qgetcwd(cwd, sizeof(cwd));
                if ( IS_SUBPATH(absdir, cwd, qstrlen(cwd)) )
                {
                  qstrvec_t subpaths;
                  while ( !qisdir(absdir) )
                  {
                    subpaths.insert(subpaths.begin(), absdir);
                    if ( !qdirname(absdir, sizeof(absdir), absdir) )
                      break;
                  }
                  for ( size_t i = 0, n = subpaths.size(); i < n; ++i )
                  {
                    const char *subdir = subpaths[i].c_str();
                    verb(("open_file() creating directory %s\n", subdir));
                    if ( qmkdir(subdir, PERM_0755) != 0 )
                      break;
                  }
                }
              }
              channels[fn] = fopenWB(path);
            }
            if ( channels[fn] == NULL )
              fn = -1;
            else if ( readonly )
              fsize = qfsize(channels[fn]);
          }
          verb(("open_file('%s', %d) => %d %" FMT_64 "d\n", path, readonly, fn, fsize));
          append_dd(req, fn);
          if ( fn != -1 )
            append_dq(req, fsize);
          else
            append_dd(req, qerrcode());
        }
        break;

      case RPC_CLOSE_FILE:
        {
          int fn = unpack_dd(&ptr, end);
          if ( fn >= 0 && fn < qnumber(channels) )
          {
#ifdef __UNIX__
            // set mode 0755 for unix applications
            fchmod(fileno(channels[fn]), PERM_0755);
#endif
            qfclose(channels[fn]);
            channels[fn] = NULL;
          }
          verb(("close_file(%d)\n", fn));
        }
        break;

      case RPC_READ_FILE:
        {
          char *buf  = NULL;
          int fn     = unpack_dd(&ptr, end);
          int64 off  = unpack_dq(&ptr, end);
          int32 size = unpack_dd(&ptr, end);
          int32 s2 = 0;
          if ( size > 0 )
          {
            buf = new char[size];
            qfseek(channels[fn], off, SEEK_SET);
            s2 = qfread(channels[fn], buf, size);
          }
          append_dd(req, s2);
          if ( size != s2 )
            append_dd(req, qerrcode());
          if ( s2 > 0 )
            append_memory(req, buf, s2);
          delete[] buf;
          verb(("read_file(%d, 0x%" FMT_64 "X, %d) => %d\n", fn, off, size, s2));
        }
        break;

      case RPC_WRITE_FILE:
        {
          char *buf = NULL;
          int fn = unpack_dd(&ptr, end);
          uint64 off = unpack_dq(&ptr, end);
          uint32 size = unpack_dd(&ptr, end);
          if ( size > 0 )
          {
            buf = new char[size];
            extract_memory(&ptr, end, buf, size);
          }
          qfseek(channels[fn], off, SEEK_SET);
          uint32 s2 = buf == NULL ? 0 : qfwrite(channels[fn], buf, size);
          append_dd(req, size);
          if ( size != s2 )
            append_dd(req, qerrcode());
          delete [] buf;
          verb(("write_file(%d, 0x%" FMT_64 "X, %u) => %u\n", fn, off, size, s2));
        }
        break;

      case RPC_EVOK:
        req.clear();
        verbev(("got evok!\n"));
        break;

      case RPC_IOCTL:
        {
          int code = handle_ioctl_packet(req, ptr, end);
          if ( code != RPC_OK )
            req = prepare_rpc_packet((uchar)code);
        }
        break;

      case RPC_UPDATE_CALL_STACK:
        {
          call_stack_t trace;
          thid_t tid = unpack_dd(&ptr, end);
          drc_t drc = dbg_mod->dbg_update_call_stack(tid, &trace);
          append_dd(req, drc);
          if ( drc == DRC_OK )
            append_call_stack(req, trace);
        }
        break;

      case RPC_APPCALL:
        {
          ea_t func_ea      = unpack_ea64(&ptr, end);
          thid_t tid        = unpack_dd(&ptr, end);
          int stkarg_nbytes = unpack_dd(&ptr, end);
          int flags         = unpack_dd(&ptr, end);

          regobjs_t regargs, retregs;
          relobj_t stkargs;
          regobjs_t *rr = (flags & APPCALL_MANUAL) == 0 ? &retregs : NULL;
          extract_appcall(&ptr, end, &regargs, &stkargs, rr);

          qstring errbuf;
          debug_event_t event;
          ea_t sp = dbg_mod->dbg_appcall(func_ea, tid, stkarg_nbytes, &regargs, &stkargs,
                                          &retregs, &errbuf, &event, flags);
          append_ea64(req, sp);
          if ( sp == BADADDR )
          {
            if ( (flags & APPCALL_DEBEV) != 0 )
              append_debug_event(req, &event);
            append_str(req, errbuf);
          }
          else if ( (flags & APPCALL_MANUAL) == 0 )
          {
            append_regobjs(req, retregs, true);
          }
        }
        break;

      case RPC_CLEANUP_APPCALL:
        {
          thid_t tid = unpack_dd(&ptr, end);
          drc_t drc = dbg_mod->dbg_cleanup_appcall(tid);
          append_dd(req, drc);
        }
        break;

      case RPC_REXEC:
        {
          const char *cmdline = extract_cstr(&ptr, end);
          int code = dbg_mod->dbg_rexec(cmdline);
          append_dd(req, code);
        }
        break;

      case RPC_BIN_SEARCH:
        {
          ea_t start_ea = unpack_ea64(&ptr, end);
          ea_t end_ea = unpack_ea64(&ptr, end);
          int cnt = unpack_dd(&ptr, end);
          compiled_binpat_vec_t ptns;
          ptns.resize(cnt);
          for ( int i=0; i < cnt; ++i )
          {
            compiled_binpat_t &p = ptns[i];
            // bytes
            int sz = unpack_dd(&ptr, end);
            if ( sz != 0 )
            {
              p.bytes.resize(sz);
              extract_memory(&ptr, end, p.bytes.begin(), sz);
            }
            // mask
            sz = unpack_dd(&ptr, end);
            if ( sz != 0 )
            {
              p.mask.resize(sz);
              extract_memory(&ptr, end, p.mask.begin(), sz);
            }
            // strlits
            sz = unpack_dd(&ptr, end);
            p.strlits.resize(sz);
            for ( int j=0; j < sz; ++j )
            {
              p.strlits[j].start_ea = unpack_ea64(&ptr, end);
              p.strlits[j].end_ea = unpack_ea64(&ptr, end);
            }
            // encidx
            p.encidx = unpack_dd(&ptr, end);
          }
          int srch_flags = unpack_dd(&ptr, end);
          ea_t srch_ea;
          qstring errbuf;
          drc_t drc = dbg_mod->dbg_bin_search(&srch_ea, start_ea, end_ea, ptns, srch_flags, &errbuf);
          append_dd(req, drc);
          if ( drc == DRC_OK )
            append_ea64(req, srch_ea);
          else if ( drc != DRC_FAILED )   // DRC_FAILED means not found
            append_str(req, errbuf);
        }
        break;

      default:
        req = prepare_rpc_packet(RPC_UNK);
        break;
    }
  }
#if defined(__EXCEPTIONS) || defined(__NT__)
  catch ( const std::bad_alloc & )
  {
    req = prepare_rpc_packet(RPC_MEM);
  }
#endif

  if ( saved_poll_mode )
    poll_debug_events = true;
  return req;
}

//--------------------------------------------------------------------------
// poll for events from the debugger module
int dbg_rpc_handler_t::poll_events(int timeout_ms)
{
  int code = 0;
  if ( !has_pending_event )
  {
    // immediately set poll_debug_events to false to avoid recursive calls.
    poll_debug_events = false;
    has_pending_event = dbg_mod->dbg_get_debug_event(&pending_event, timeout_ms) >= GDE_ONE_EVENT;
    if ( has_pending_event )
    {
      verbev(("got event, sending it, poll will be 0 now\n"));
      bytevec_t req = prepare_rpc_packet(RPC_EVENT);
      append_debug_event(req, &pending_event);
      code = send_data(req);
      has_pending_event = false;
    }
    else
    { // no event, continue to poll
      poll_debug_events = true;
    }
  }
  return code;
}

//--------------------------------------------------------------------------
// this function runs on the server side
// a dbg_rpc_client sends an RPC_SYNC request and the server must give the stub to the client
bool dbg_rpc_handler_t::rpc_sync_stub(const char *server_stub_name, const char *ida_stub_name)
{
  bool ok = false;
  int32 crc32 = -1;
  linput_t *li = open_linput(server_stub_name, false);
  if ( li != NULL )
  {
    crc32 = calc_file_crc32(li);
    close_linput(li);
  }

  bytevec_t stub = prepare_rpc_packet(RPC_SYNC_STUB);
  append_str(stub, ida_stub_name);
  append_dd(stub, crc32);
  rpc_packet_t *rp = send_request_and_receive_reply(stub);

  if ( rp == NULL )
    return ok;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;
  size_t size = unpack_dd(&answer, end);
  if ( size == 1 )
  {
    ok = true;
  }
  else if ( size != 0 )
  {
    FILE *fp = fopenWB(server_stub_name);
    if ( fp != NULL )
    {
      ok = qfwrite(fp, answer, size) == size;
      dmsg("Updated kernel debugger stub: %s\n", ok ? "success" : "failed");
      qfclose(fp);
    }
    else
    {
      dwarning("Could not update the kernel debugger stub.\n%s", qerrstr());
    }
  }
  qfree(rp);

  return ok;
}

//--------------------------------------------------------------------------
//lint -e{818} 'addrs' could be declared as pointing to const
int dbg_rpc_handler_t::send_debug_names_to_ida(ea_t *addrs, const char *const *names, int qty)
{
  if ( qty == 0 )
    return RPC_OK;

  bytevec_t buf;

  const size_t SZPACKET = 1300; // try not to go over the usual network MSS
                                // (this number is slightly less that 1460 because
                                //  we stop the loop going over this number)
  while ( qty > 0 )
  {
    buf.qclear();

    ea_t old = 0;
    const char *optr = "";

    // Start appending names and EAs
    int i = 0;
    while ( i < qty )
    {
      adiff_t diff = *addrs - old;
      bool neg = diff < 0;
      if ( neg )
        diff = -diff;

      append_ea64(buf, diff); // send address deltas
      append_dd(buf, neg);

      old = *addrs;
      const char *nptr = *names;
      int len = 0;

      // do not send repeating prefixes of names
      while ( nptr[len] != '\0' && nptr[len] == optr[len] ) //lint !e690 wrong access
        len++;

      append_dd(buf, len);
      append_str(buf, nptr+len);
      optr = nptr;
      addrs++;
      names++;
      i++;

      if ( buf.size() > SZPACKET )
        break;
    }
    qty -= i;

    bytevec_t req = prepare_rpc_packet(RPC_SET_DEBUG_NAMES);
    append_dd(req, i);
    req.append(buf.begin(), buf.size());

    // should return a qty as much as sent...if not probably network error!
    if ( i != send_request_get_long_result(req) )
      return RPC_UNK;
  }

  return RPC_OK;
}

//--------------------------------------------------------------------------
int dbg_rpc_handler_t::send_debug_event_to_ida(const debug_event_t *debev, int rqflags)
{
  bytevec_t req = prepare_rpc_packet(RPC_HANDLE_DEBUG_EVENT);
  append_debug_event(req, debev);
  append_dd(req, rqflags);
  return send_request_get_long_result(req);
}

//--------------------------------------------------------------------------
void dbg_rpc_handler_t::process_import_requests(const import_infos_t &infos)
{
  // in an effort to avoid sending large amounts of symbol data over the network,
  // we attempt to import symbols for each dll on the client side.
  // if the client does not support such behavior, then we simply parse the symbols
  // on the server side and append to the list of debug names to send to IDA, as normal.
  for ( import_infos_t::const_iterator i = infos.begin(); i != infos.end(); ++i )
  {
    ea_t base = i->base;
    const char *path = i->path.c_str();
    const bytevec_t &uuid = i->uuid;

    bytevec_t req = prepare_rpc_packet(RPC_IMPORT_DLL);
    append_ea64(req, base);
    append_str(req, path);
    append_buf(req, uuid.begin(), uuid.size());

    int code = send_request_get_long_result(req);
    if ( code < 0 )  // cancelled or network error
      return;
    if ( code != 0 ) // request failed, fall back to parsing symbols server-side
      dbg_mod->import_dll(*i);
  }
}

//--------------------------------------------------------------------------
bool dbg_rpc_handler_t::get_broken_connection(void)
{
  return get_debugger_instance()->broken_connection;
}

//--------------------------------------------------------------------------
void dbg_rpc_handler_t::set_broken_connection(void)
{
  get_debugger_instance()->broken_connection = true;
}

//-------------------------------------------------------------------------
int dbg_rpc_handler_t::kill_process(void)
{
  const int NSEC = 5;
  dbg_mod->dbg_exit_process(NULL);

  // now, wait up to NSEC seconds until the process is gone
  qtime64_t wait_start = qtime64();
  qtime64_t wait_threshold = make_qtime64(
          get_secs(wait_start) + NSEC,
          get_usecs(wait_start));
  while ( qtime64() < wait_threshold )
  {
    gdecode_t result = dbg_mod->dbg_get_debug_event(&ev, 100);
    if ( result >= GDE_ONE_EVENT )
    {
      dbg_mod->dbg_continue_after_event(&ev);
      if ( ev.eid() == PROCESS_EXITED )
        return 0;
    }
  }
  return NSEC;
}

//--------------------------------------------------------------------------
int debmod_t::send_debug_names_to_ida(ea_t *addrs, const char *const *names, int qty)
{
  dbg_rpc_handler_t *s = (dbg_rpc_handler_t *)rpc;
  return s->send_debug_names_to_ida(addrs, names, qty);
}

//--------------------------------------------------------------------------
int debmod_t::send_debug_event_to_ida(const debug_event_t *ev, int rqflags)
{
  dbg_rpc_handler_t *s = (dbg_rpc_handler_t *)rpc;
  return s->send_debug_event_to_ida(ev, rqflags);
}
