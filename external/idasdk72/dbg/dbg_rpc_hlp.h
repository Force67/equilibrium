#ifndef __DBG_RPC_HLP__
#define __DBG_RPC_HLP__

#include <pro.h>
#include <range.hpp>
#include <idd.hpp>
#include <network.hpp>

void extract_regvals(
        const uchar **ptr,
        const uchar *end,
        regval_t *values,
        int n,
        const uchar *regmap);

void append_regvals(bytevec_t &s, const regval_t *values, int n, const uchar *regmap);
void append_debug_event(bytevec_t &s, const debug_event_t *ev);
void extract_debug_event(const uchar **ptr, const uchar *end, debug_event_t *ev);
void extract_exception(const uchar **ptr, const uchar *end, excinfo_t *exc);
void append_exception(bytevec_t &s, const excinfo_t *e);

inline void append_breakpoint(bytevec_t &s, const bptaddr_t *info)
{
  append_ea64(s, info->hea);
  append_ea64(s, info->kea);
}

inline void extract_breakpoint(const uchar **ptr, const uchar *end, bptaddr_t *info)
{
  info->hea = unpack_ea64(ptr, end);
  info->kea = unpack_ea64(ptr, end);
}
void extract_module_info(const uchar **ptr, const uchar *end, modinfo_t *info);
void append_module_info(bytevec_t &s, const modinfo_t *info);
void extract_process_info_vec(const uchar **ptr, const uchar *end, procinfo_vec_t *procs);
void append_process_info_vec(bytevec_t &s, const procinfo_vec_t *procs);

void extract_call_stack(const uchar **ptr, const uchar *end, call_stack_t *trace);
void append_call_stack(bytevec_t &s, const call_stack_t &trace);

void extract_regobjs(const uchar **ptr, const uchar *end, regobjs_t *regargs, bool with_values);
void append_regobjs(bytevec_t &s, const regobjs_t &regargs, bool with_values);

void extract_appcall(
        const uchar **ptr,
        const uchar *end,
        regobjs_t *regargs,
        relobj_t *stkargs,
        regobjs_t *retregs);

void append_appcall(
        bytevec_t &s,
        const regobjs_t &regargs,
        const relobj_t &stkargs,
        const regobjs_t *retregs);

void extract_debapp_attrs(
        const uchar **ptr,
        const uchar *end,
        debapp_attrs_t *attrs);

void append_debapp_attrs(bytevec_t &s, const debapp_attrs_t *attrs);


inline void append_type(bytevec_t &s, const type_t *str)
{
  append_str(s, (char *)str);
}

void append_type(bytevec_t &s, const tinfo_t &tif);
void extract_type(tinfo_t *tif, const uchar **ptr, const uchar *end);

void extract_memory_info(const uchar **ptr, const uchar *end, memory_info_t *info);
void append_memory_info(bytevec_t &s, const memory_info_t *info);

void extract_scattered_segm(const uchar **ptr, const uchar *end, scattered_segm_t *ss);
void append_scattered_segm(bytevec_t &s, const scattered_segm_t *ss);

void append_exception_info(bytevec_t &s, const exception_info_t *table, int qty);
exception_info_t *extract_exception_info(const uchar **ptr, const uchar *end,int qty);


#endif // __DBG_RPC_HLP__
