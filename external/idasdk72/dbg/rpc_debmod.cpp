
#include <segment.hpp>
#include <err.h>
#include <network.hpp>

#include "rpc_debmod.h"
#include "dbg_rpc_hlp.h"

//-------------------------------------------------------------------------
inline drc_t unpack_drc(const uchar **ptr, const uchar *end)
{
  return drc_t(unpack_dd(ptr, end));
}

//--------------------------------------------------------------------------
rpc_debmod_t::rpc_debmod_t(const char *default_platform)
  : dbg_rpc_client_t(NULL)
{
  nregs = debugger.nregs;
  for ( int i=0; i < nregs; i++ )
  {
    const register_info_t &ri = debugger.regs(i);
    if ( (ri.flags & REGISTER_SP) != 0 )
      sp_idx = i;
    if ( (ri.flags & REGISTER_IP) != 0 )
      pc_idx = i;
  }
  bpt_code.append(debugger.bpt_bytes, debugger.bpt_size);
  rpc = this;

  set_platform(default_platform);
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::handle_ioctl(    //-V524 equivalent to 'send_ioctl'
        int fn,
        const void *buf,
        size_t size,
        void **poutbuf,
        ssize_t *poutsize)
{
  return rpc_engine_t::send_ioctl(fn, buf, size, poutbuf, poutsize);
}

//--------------------------------------------------------------------------
inline int get_expected_addrsize(void)
{
  if ( is_miniidb() )
#ifdef __EA64__
    return 8;
#else
    return 4;
#endif
  return inf.is_64bit() ? 8 : 4;
}

//--------------------------------------------------------------------------
bool idaapi rpc_debmod_t::open_remote(
        const char *hostname,
        int port_number,
        const char *password,
        qstring *errbuf)
{
  if ( hostname[0] == '\0' )
  {
    if ( errbuf != NULL )
      *errbuf = "Please specify the hostname in Debugger, Process options";
    return false;
  }

  rpc_packet_t *rp = NULL;
  network_error = false;
  client_irs = irs_new();
  if ( !irs_init_client(client_irs, hostname, port_number) )
  {
FAILURE:
    if ( rp != NULL )
      qfree(rp);

    if ( errbuf != NULL )
      *errbuf = irs_strerror(client_irs);
    irs_term(&client_irs);

    return false;
  }

  rp = recv_packet();
  if ( rp == NULL || rp->code != RPC_OPEN )  // is this an ida debugger server?
  {
    dbg_rpc_client_t::dwarning("ICON ERROR\nAUTOHIDE NONE\n"
                               "Bogus or irresponsive remote server");
    goto FAILURE;
  }

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;
  int version = unpack_dd(&answer, end);
  int remote_debugger_id = unpack_dd(&answer, end);
  int easize = unpack_dd(&answer, end);
  qstring errstr;
  if ( version != IDD_INTERFACE_VERSION )
    errstr.sprnt("protocol version is %d, expected %d", version, IDD_INTERFACE_VERSION);
  else if ( remote_debugger_id != debugger.id )
    errstr.sprnt("debugger id is %d, expected %d (%s)", remote_debugger_id, debugger.id, debugger.name);
  else if ( easize != get_expected_addrsize() )
    errstr.sprnt("address size is %d bytes, expected %d", easize, inf.is_64bit() ? 8 : 4);
  if ( !errstr.empty() )
  {
    bytevec_t req = prepare_rpc_packet(RPC_OK);
    append_dd(req, false);
    send_data(req);
    warning("ICON ERROR\nAUTOHIDE NONE\n"
            "Incompatible debugging server:\n"
            "%s", errstr.c_str());
    goto FAILURE;
  }
  qfree(rp);

  bytevec_t req = prepare_rpc_packet(RPC_OK);
  append_dd(req, true);
  append_str(req, password);
  send_data(req);

  rp = recv_packet();
  if ( rp == NULL || rp->code != RPC_OK )
    goto FAILURE;

  answer = (uchar *)(rp+1);
  end = answer + rp->length;
  bool password_ok = unpack_dd(&answer, end) != 0;
  if ( !password_ok )  // is this an ida debugger server?
  {
    warning("ICON ERROR\nAUTOHIDE NONE\n"
            "Bad password");
    goto FAILURE;
  }

  qfree(rp);
  return true;
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_add_bpt(bytevec_t *, bpttype_t, ea_t, int)
{
  INTERR(30114);
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_del_bpt(bpttype_t, ea_t, const uchar *, int)
{
  INTERR(30115);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_update_lowcnds(
        int *nupdated,
        const lowcnd_t *lowcnds,
        int nlowcnds,
        qstring *errbuf)
{
  ea_t ea = 0;
  bytevec_t req = prepare_rpc_packet(RPC_UPDATE_LOWCNDS);
  append_dd(req, nlowcnds);
  const lowcnd_t *lc = lowcnds;
  for ( int i=0; i < nlowcnds; i++, lc++ )
  {
    append_ea64(req, lc->ea-ea); ea = lc->ea;
    append_str(req, lc->cndbody);
    if ( !lc->cndbody.empty() )
    {
      append_dd(req, lc->type);
      if ( lc->type != BPT_SOFT )
        append_dd(req, lc->size);
      append_db(req, lc->orgbytes.size());
      append_memory(req, lc->orgbytes.begin(), lc->orgbytes.size());
      append_ea64(req, lc->cmd.ea);
      if ( lc->cmd.ea != BADADDR )
        append_memory(req, &lc->cmd, sizeof(lc->cmd));
    }
  }

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = unpack_drc(&answer, end);
  int ret_nupdated = unpack_dd(&answer, end);
  if ( nupdated != NULL )
    *nupdated = ret_nupdated;

  if ( errbuf != NULL && drc != DRC_NONE )
    *errbuf = extract_cstr(&answer, end);

  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_eval_lowcnd(thid_t tid, ea_t ea, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_EVAL_LOWCND);
  append_dd(req, tid);
  append_ea64(req, ea);
  return send_request_get_drc_result(req, errbuf);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_update_bpts(
        int *nbpts,
        update_bpt_info_t *ubpts,
        int nadd,
        int ndel,
        qstring *errbuf)
{
  int skipped = 0;
  update_bpt_info_t *b;
  update_bpt_info_t *bend = ubpts + nadd;
  for ( b=ubpts; b != bend; b++ )
    if ( b->code != BPT_OK )
      skipped++;
  if ( skipped == nadd && ndel == 0 )
  {
    if ( nbpts != NULL )
      *nbpts = 0;   // no bpts to update
    return DRC_OK;
  }

  bytevec_t req = prepare_rpc_packet(RPC_UPDATE_BPTS);
  append_dd(req, nadd-skipped);
  append_dd(req, ndel);
  ea_t ea = 0;
  for ( b=ubpts; b != bend; b++ )
  {
    if ( b->code == BPT_OK )
    {
      append_ea64(req, b->ea-ea); ea = b->ea;
      append_dd(req, b->size);
      append_dd(req, b->type);
      append_dd(req, b->pid);
      append_dd(req, b->tid);
    }
  }

  ea = 0;
  bend += ndel;
  for ( ; b != bend; b++ )
  {
    append_ea64(req, b->ea-ea); ea = b->ea;
    append_db(req, b->orgbytes.size());
    append_memory(req, b->orgbytes.begin(), b->orgbytes.size());
    append_dd(req, b->type);
    append_dd(req, b->pid);
    append_dd(req, b->tid);
  }

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *ptr = (uchar *)(rp+1);
  const uchar *end = ptr + rp->length;

  drc_t drc = unpack_drc(&ptr, end);
  int ret_nbpts = unpack_dd(&ptr, end);
  if ( nbpts != NULL )
    *nbpts = ret_nbpts;
  bend = ubpts + nadd;
  for ( b=ubpts; b != bend; b++ )
  {
    if ( b->code == BPT_OK )
    {
      b->code = unpack_db(&ptr, end);
      if ( b->code == BPT_OK && b->type == BPT_SOFT )
      {
        uchar len = unpack_db(&ptr, end);
        b->orgbytes.resize(len);
        extract_memory(&ptr, end, b->orgbytes.begin(), len);
      }
    }
  }

  bend += ndel;
  for ( ; b != bend; b++ )
    b->code = unpack_db(&ptr, end);

  if ( errbuf != NULL && drc != DRC_NONE )
    *errbuf = extract_cstr(&ptr, end);
  return drc;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_thread_get_sreg_base(ea_t *ea, thid_t tid, int sreg_value, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_GET_SREG_BASE);
  append_dd(req, tid);
  append_dd(req, sreg_value);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = unpack_drc(&answer, end);
  if ( drc == DRC_OK )
    *ea = unpack_ea64(&answer, end);
  else if ( errbuf != NULL )
    *errbuf = extract_cstr(&answer, end);

  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
void idaapi rpc_debmod_t::dbg_set_exception_info(const exception_info_t *table, int qty)
{
  bytevec_t req = prepare_rpc_packet(RPC_SET_EXCEPTION_INFO);
  append_dd(req, qty);
  append_exception_info(req, table, qty);

  qfree(send_request_and_receive_reply(req));
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_open_file(const char *file, uint64 *fsize, bool readonly)
{
  bytevec_t req = prepare_rpc_packet(RPC_OPEN_FILE);
  append_str(req, file);
  append_dd(req, readonly);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return -1;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int fn = unpack_dd(&answer, end);
  if ( fn != -1 )
  {
    if ( fsize != NULL && readonly )
      *fsize = unpack_dq(&answer, end);
  }
  else
  {
    qerrcode(unpack_dd(&answer, end));
  }
  qfree(rp);
  return fn;
}

//--------------------------------------------------------------------------
void idaapi rpc_debmod_t::dbg_close_file(int fn)
{
  bytevec_t req = prepare_rpc_packet(RPC_CLOSE_FILE);
  append_dd(req, fn);

  qfree(send_request_and_receive_reply(req));
}

//--------------------------------------------------------------------------
ssize_t idaapi rpc_debmod_t::dbg_read_file(int fn, qoff64_t off, void *buf, size_t size)
{
  bytevec_t req = prepare_rpc_packet(RPC_READ_FILE);
  append_dd(req, fn);
  append_dq(req, off);
  append_dd(req, (uint32)size);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return -1;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int32 rsize = unpack_dd(&answer, end);
  if ( size != rsize )
    qerrcode(unpack_dd(&answer, end));

  if ( rsize > 0 )
  {
    QASSERT(1204, rsize <= size);
    extract_memory(&answer, end, buf, rsize);
  }
  qfree(rp);
  return rsize;
}

//--------------------------------------------------------------------------
ssize_t idaapi rpc_debmod_t::dbg_write_file(int fn, qoff64_t off, const void *buf, size_t size)
{
  bytevec_t req = prepare_rpc_packet(RPC_WRITE_FILE);
  append_dd(req, fn);
  append_dq(req, off);
  append_dd(req, (uint32)size);
  append_memory(req, buf, size);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return -1;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int32 rsize = unpack_dd(&answer, end);
  if ( size != rsize )
    qerrcode(unpack_dd(&answer, end));

  qfree(rp);
  return rsize;
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_is_ok_bpt(bpttype_t type, ea_t ea, int len)
{
  bytevec_t req = prepare_rpc_packet(RPC_ISOK_BPT);
  append_dd(req, type);
  append_ea64(req, ea);
  append_dd(req, len+1);

  return send_request_get_long_result(req);
}

//--------------------------------------------------------------------------
void idaapi rpc_debmod_t::dbg_set_debugging(bool _debug_debugger)
{
  debug_debugger = _debug_debugger;
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_init(qstring *errbuf)
{
  has_pending_event = false;
  poll_debug_events = false;

  bytevec_t req = prepare_rpc_packet(RPC_INIT);
  append_dd(req, debugger.flags);
  append_dd(req, debug_debugger);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return -1;
  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int result = unpack_dd(&answer, end);
  if ( result < 0 && errbuf != NULL )
    *errbuf = extract_cstr(&answer, end);

  qfree(rp);
  return result;
}

//--------------------------------------------------------------------------
void idaapi rpc_debmod_t::dbg_term(void)
{
  bytevec_t req = prepare_rpc_packet(RPC_TERM);

  qfree(send_request_and_receive_reply(req));
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_get_processes(procinfo_vec_t *procs, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_GET_PROCESSES);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;
  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  procs->qclear();
  drc_t drc = unpack_drc(&answer, end);
  if ( drc == DRC_OK )
    extract_process_info_vec(&answer, end, procs);
  else if ( errbuf != NULL )
    *errbuf = extract_cstr(&answer, end);

  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_detach_process(void)
{
  return get_drc(RPC_DETACH_PROCESS);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_start_process(
        const char *path,
        const char *args,
        const char *startdir,
        int flags,
        const char *input_path,
        uint32 input_file_crc32,
        qstring *errbuf)
{
  if ( (inf.s_cmtflg & SW_TESTMODE) != 0 )
    flags |= DBG_HIDE_WINDOW;
  bytevec_t req = prepare_rpc_packet(RPC_START_PROCESS);
  append_str(req, path);
  append_str(req, args);
  append_str(req, startdir);
  append_dd(req, flags);
  append_str(req, input_path);
  append_dd(req, input_file_crc32);

  return process_start_or_attach(req, errbuf);
}

//--------------------------------------------------------------------------
gdecode_t idaapi rpc_debmod_t::dbg_get_debug_event(debug_event_t *event, int timeout_ms)
{
  if ( has_pending_event )
  {
    verbev(("get_debug_event => has pending event, returning it\n"));
    *event = pending_event;
    has_pending_event = false;
    poll_debug_events = false;
    return GDE_ONE_EVENT;
  }

  gdecode_t result = GDE_NO_EVENT;
  if ( poll_debug_events )
  {
    // do we have something waiting?
    if ( irs_ready(client_irs, timeout_ms) > 0 )
    {
      verbev(("get_debug_event => remote has a packet for us\n"));
      // get the packet - it can RPC_EVENT or RPC_MSG/RPC_WARNING/RPC_ERROR
      bytevec_t empty;
      rpc_packet_t *rp = send_request_and_receive_reply(empty, PREQ_GET_EVENT);
      verbev(("get_debug_event => processed remote event, has=%d\n", has_pending_event));
      if ( rp != NULL )
      {
        warning("rpc: event protocol error (rp=%p has_event=%d)", rp, has_pending_event);
        return GDE_ERROR;
      }
    }
  }
  else
  {
    verbev(("get_debug_event => first time, send GET_DEBUG_EVENT\n"));
    bytevec_t req = prepare_rpc_packet(RPC_GET_DEBUG_EVENT);
    append_dd(req, timeout_ms);

    rpc_packet_t *rp = send_request_and_receive_reply(req);
    if ( rp == NULL )
      return GDE_ERROR;
    const uchar *answer = (uchar *)(rp+1);
    const uchar *end = answer + rp->length;

    result = gdecode_t(unpack_dd(&answer, end));
    if ( result >= GDE_ONE_EVENT )
      extract_debug_event(&answer, end, event);
    else
      poll_debug_events = true;
    verbev(("get_debug_event => remote said %d, poll=%d now\n", result, poll_debug_events));
    qfree(rp);
  }
  return result;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_attach_process(pid_t _pid, int event_id, int flags, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_ATTACH_PROCESS);
  append_dd(req, _pid);
  append_dd(req, event_id);
  append_dd(req, flags);
  return process_start_or_attach(req, errbuf);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_prepare_to_pause_process(qstring *errbuf)
{
  return get_drc(RPC_PREPARE_TO_PAUSE_PROCESS, errbuf);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_exit_process(qstring *errbuf)
{
  return get_drc(RPC_EXIT_PROCESS, errbuf);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_continue_after_event(const debug_event_t *event)
{
  bytevec_t req = prepare_rpc_packet(RPC_CONTINUE_AFTER_EVENT);
  append_debug_event(req, event);

  return send_request_get_drc_result(req, NULL);
}

//--------------------------------------------------------------------------
void idaapi rpc_debmod_t::dbg_stopped_at_debug_event(import_infos_t *, bool dlls_added, thread_name_vec_t *thr_names)
{
  bytevec_t req = prepare_rpc_packet(RPC_STOPPED_AT_DEBUG_EVENT);
  append_db(req, dlls_added);
  bool ask_thr_names = thr_names != NULL;
  append_db(req, ask_thr_names);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return;

  if ( ask_thr_names )
  {
    const uchar *answer = (uchar *)(rp+1);
    const uchar *end = answer + rp->length;
    const uchar **ptr = &answer;

    uint32 n = unpack_dd(ptr, end);
    thr_names->resize(n);
    for ( int i=0; i < n; ++i )
    {
      thread_name_t &tn = (*thr_names)[i];
      tn.tid = unpack_dd(ptr, end);
      tn.name = extract_cstr(ptr, end);
    }
  }

  qfree(rp);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_thread_suspend(thid_t tid)
{
  return get_drc_int(RPC_TH_SUSPEND, tid);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_thread_continue(thid_t tid)
{
  return get_drc_int(RPC_TH_CONTINUE, tid);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_set_resume_mode(thid_t tid, resume_mode_t resmod)
{
  bytevec_t req = prepare_rpc_packet(RPC_SET_RESUME_MODE);
  append_dd(req, tid);
  append_dd(req, resmod);

  return send_request_get_drc_result(req, NULL);
}

//--------------------------------------------------------------------------
// prepare bitmap of registers belonging to the specified classes
// return size of the bitmap in bits (always the total number of registers)
static int calc_regmap(bytevec_t *regmap, int clsmask)
{
  int nregs = debugger.nregs;
  regmap->resize((nregs+7)/8, 0);
  for ( int i=0; i < nregs; i++ )
    if ( (debugger.regs(i).register_class & clsmask) != 0 )
      regmap->set_bit(i);
  return nregs;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_read_registers(
        thid_t tid,
        int clsmask,
        regval_t *values,
        qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_READ_REGS);
  append_dd(req, tid);
  append_dd(req, clsmask);
  // append additional information about the class structure
  bytevec_t regmap;
  int n_regs = calc_regmap(&regmap, clsmask);
  append_dd(req, n_regs);
  append_memory(req, regmap.begin(), regmap.size());

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = unpack_drc(&answer, end);
  if ( drc == DRC_OK )
    extract_regvals(&answer, end, values, n_regs, regmap.begin());
  else if ( errbuf != NULL )
    *errbuf = extract_cstr(&answer, end);
  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_write_register(
        thid_t tid,
        int reg_idx,
        const regval_t *value,
        qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_WRITE_REG);
  append_dd(req, tid);
  append_dd(req, reg_idx);
  append_regvals(req, value, 1, NULL);

  return send_request_get_drc_result(req, errbuf);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_get_memory_info(meminfo_vec_t &areas, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_GET_MEMORY_INFO);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = drc_t(unpack_dd(&answer, end) + DRC_IDBSEG);
  if ( drc > DRC_NONE )
  {
    int n = unpack_dd(&answer, end);
    areas.resize(n);
    for ( int i=0; i < n; i++ )
      extract_memory_info(&answer, end, &areas[i]);
  }
  else if ( errbuf != NULL )
  {
    *errbuf = extract_cstr(&answer, end);
  }
  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_get_scattered_image(scattered_image_t &si, ea_t base)
{
  bytevec_t req = prepare_rpc_packet(RPC_GET_SCATTERED_IMAGE);
  append_ea64(req, base);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return false;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int result = unpack_dd(&answer, end) - 2;
  if ( result > 0 )
  {
    int n = unpack_dd(&answer, end);
    si.resize(n);
    for ( int i=0; i < n; i++ )
      extract_scattered_segm(&answer, end, &si[i]);
  }
  qfree(rp);
  return result;
}

//--------------------------------------------------------------------------
bool idaapi rpc_debmod_t::dbg_get_image_uuid(bytevec_t *uuid, ea_t base)
{
  bytevec_t req = prepare_rpc_packet(RPC_GET_IMAGE_UUID);
  append_ea64(req, base);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return false;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  bool result = unpack_dd(&answer, end) != 0;
  if ( result )
  {
    int n = unpack_dd(&answer, end);
    uuid->append(answer, n);
  }
  qfree(rp);
  return result;
}

//--------------------------------------------------------------------------
ea_t idaapi rpc_debmod_t::dbg_get_segm_start(ea_t base, const qstring &segname)
{
  bytevec_t req = prepare_rpc_packet(RPC_GET_SEGM_START);
  append_ea64(req, base);
  append_str(req, segname.c_str());

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return false;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  ea_t result = unpack_ea64(&answer, end);
  qfree(rp);
  return result;
}

//--------------------------------------------------------------------------
ssize_t idaapi rpc_debmod_t::dbg_read_memory(ea_t ea, void *buffer, size_t size, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_READ_MEMORY);
  append_ea64(req, ea);
  append_dd(req, (uint32)size);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return -1;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int result = unpack_dd(&answer, end);
  if ( result > 0 )
  {
    QASSERT(1205, result <= size);
    extract_memory(&answer, end, buffer, result);
  }
  else if ( errbuf != NULL )
  {
    *errbuf = extract_cstr(&answer, end);
  }
  qfree(rp);
  return result;
}

//--------------------------------------------------------------------------
ssize_t idaapi rpc_debmod_t::dbg_write_memory(ea_t ea, const void *buffer, size_t size, qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_WRITE_MEMORY);
  append_ea64(req, ea);
  append_dd(req, (uint32)size);
  append_memory(req, buffer, size);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return -1;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  int result = unpack_dd(&answer, end);
  if ( errbuf != NULL && result <= 0 )
    *errbuf = extract_cstr(&answer, end);
  qfree(rp);
  return result;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_update_call_stack(thid_t tid, call_stack_t *trace)
{
  bytevec_t req = prepare_rpc_packet(RPC_UPDATE_CALL_STACK);
  append_dd(req, tid);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = unpack_drc(&answer, end);
  if ( drc == DRC_OK )
    extract_call_stack(&answer, end, trace);
  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
ea_t idaapi rpc_debmod_t::dbg_appcall(
        ea_t func_ea,
        thid_t tid,
        int stkarg_nbytes,
        const struct regobjs_t *regargs,
        struct relobj_t *stkargs,
        struct regobjs_t *retregs,
        qstring *errbuf,
        debug_event_t *event,
        int flags)
{
  bytevec_t req = prepare_rpc_packet(RPC_APPCALL);
  append_ea64(req, func_ea);
  append_dd(req, tid);
  append_dd(req, stkarg_nbytes);
  append_dd(req, flags);
  regobjs_t *rr = (flags & APPCALL_MANUAL) == 0 ? retregs : NULL;
  append_appcall(req, *regargs, *stkargs, rr);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return BADADDR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  ea_t sp = unpack_ea64(&answer, end);
  if ( sp == BADADDR )
  {
    if ( (flags & APPCALL_DEBEV) != 0 )
      extract_debug_event(&answer, end, event);
    if ( errbuf != NULL )
      *errbuf = extract_cstr(&answer, end);
  }
  else if ( (flags & APPCALL_MANUAL) == 0 )
  {
    if ( retregs != NULL )
      extract_regobjs(&answer, end, retregs, true);
  }
  qfree(rp);
  return sp;
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_cleanup_appcall(thid_t tid)
{
  bytevec_t req = prepare_rpc_packet(RPC_CLEANUP_APPCALL);
  append_dd(req, tid);
  return send_request_get_drc_result(req, NULL);
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::dbg_rexec(const char *cmdline)
{
  bytevec_t req = prepare_rpc_packet(RPC_REXEC);
  append_str(req, cmdline);
  return send_request_get_long_result(req);
}

//--------------------------------------------------------------------------
drc_t idaapi rpc_debmod_t::dbg_bin_search(
        ea_t *pea,
        ea_t start_ea,
        ea_t end_ea,
        const compiled_binpat_vec_t &ptns,
        int srch_flags,
        qstring *errbuf)
{
  bytevec_t req = prepare_rpc_packet(RPC_BIN_SEARCH);
  append_ea64(req, start_ea);
  append_ea64(req, end_ea);
  // compiled_binpat_vec_t
  int sz = ptns.size();
  append_dd(req, sz);
  for ( compiled_binpat_vec_t::const_iterator p=ptns.begin();
        p != ptns.end();
        ++p )
  { // compiled_binpat_t
    sz = p->bytes.size();
    append_dd(req, sz);
    append_memory(req, p->bytes.begin(), sz);
    sz = p->mask.size();
    append_dd(req, sz);
    append_memory(req, p->mask.begin(), sz);
    sz = p->strlits.size();
    append_dd(req, sz);
    for ( int i=0; i < sz; ++i )
    {
      append_ea64(req, p->strlits[i].start_ea);
      append_ea64(req, p->strlits[i].end_ea);
    }
    append_dd(req, p->encidx);
  }
  append_dd(req, srch_flags);

  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = drc_t(unpack_dd(&answer, end));
  if ( drc == DRC_OK )
  {
    if ( pea != NULL )
      *pea = unpack_ea64(&answer, end);
  }
  else if ( drc != DRC_FAILED )   // DRC_FAILED means not found
  {
    if ( errbuf != NULL )
      *errbuf = extract_cstr(&answer, end);
  }

  qfree(rp);
  return drc;
}

//--------------------------------------------------------------------------
drc_t rpc_debmod_t::close_remote()
{
  bytevec_t req = prepare_rpc_packet(RPC_OK);
  send_data(req);
  irs_term(&client_irs);
  network_error = false;
  return DRC_OK;
}

//--------------------------------------------------------------------------
int idaapi rpc_debmod_t::get_system_specific_errno(void) const
{
  return irs_get_error(client_irs);
}

//-------------------------------------------------------------------------
drc_t rpc_debmod_t::process_start_or_attach(bytevec_t &req, qstring *errbuf)
{
  rpc_packet_t *rp = send_request_and_receive_reply(req);
  if ( rp == NULL )
    return DRC_NETERR;

  const uchar *answer = (uchar *)(rp+1);
  const uchar *end = answer + rp->length;

  drc_t drc = unpack_drc(&answer, end);
  if ( drc > DRC_NONE )
    extract_debapp_attrs(&answer, end, &debapp_attrs);
  else if ( errbuf != NULL )
    *errbuf = extract_cstr(&answer, end);
  qfree(rp);
  return drc;
}
