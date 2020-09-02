# IDA 7.0 SDK: Porting from IDA 4.9-6.x API to IDA 7.0 API

## Introduction

The SDK now only supports the new 7.0 API in x64 mode.
The old SDK 6.95 can be used to develop plugins for IDA 7.0-x86 (which is ABI-compatible with IDA 6.95).

While the API has been revamped somewhat, most basic concepts still hold.

There are still two variants of IDA: one supporting 32-bit (ea_t is 32-bit)
and the other 64-bit address space (ea_t is 64-bit).
IDA database extensions remain correspondingly '.idb' and '.i64'.


Naming of IDA binaries has been unified across all OS variants:

* The IDA GUI binary has been renamed from 'idaq[.exe]' to just 'ida[.exe]'.
* The IDA text-mode UI has been renamed from 'idaw.exe' (on Windows) and 'idal'
  (on Linux/Mac OS X) to 'idat[.exe]' on all platforms.
* Plugins, loaders, processor modules, and the kernel now use standard
  OS-specific suffixes ('.dll', '.so', or '.dylib') instead of custom extensions.


General approaches that were taken when cleaning up the APIs:

* Try to use descriptive names and drop old, cryptic abbreviations.
* Rename functions using camelCase to snake_case (e.g. 'isFlow' -> 'is_flow').
* Move output parameters to the front of the argument list.
* Change input parameters to const references whenever possible.
* Remove obsolete and deprecated functions.
* Rename functioname2/3 to just functioname (e.g. 'validate_name3' -> 'validate_name').
* Rename functions with 64 suffix to the main name (e.g. 'qfseek64' -> 'qfseek').
* File offsets are 64-bit in all functions working with files.
* Get rid of global variables (not complete, but we made good progress).
* Most functions accepting a buffer and size (or limited to MAXSTR) now
  use 'qstring' or 'bytevec_t' instead (depending on the nature of the data).
* Assume UTF-8 in most functions dealing with text.
* Try to get rid of forced struct packing and rearrange fields to avoid unnecessary gaps as needed.


## Porting

Common porting steps for plugins/loaders/modules:

- Add __\_\_X64\_\___ to the list of preprocessor defines.
  You still need to compile with or without __\_\_EA64\_\___ defined to select
  between 32- and 64-bit address space.
- If using custom build system, change output extension to OS-specific
  suffixes ('.dll', '.so', or '.dylib').
- IDA library link path should start with x64 instead of x86.


### Renamed/removed header files

Some headers have been renamed and/or removed:

| original name | new name      |
|---------------|---------------|
| ints.hpp      | <**removed**> |
| sistack.h     | <**removed**> |
| area.hpp      | range.hpp     |
| queue.hpp     | problems.hpp  |
| srarea.hpp    | segregs.hpp   |


### Commonly used renamed structs and fields

| original name | new name   |
|---------------|------------|
| area_t        | range_t    |
| areavec_t     | rangevec_t |
| endEA         | end_ea     |
| startEA       | start_ea   |

area-related methods have been renamed too (e.g. 'prev_area' -> 'prev_range').


### Porting plugins.

The plugin entry prototype has been changed from:

* void idaapi run(int);

to:

* bool idaapi run(size_t);

The input parameter is now of type 'size_t', which allows passing a pointer
as the argument of run() for extra possibilities.

The rest of the plugin interface is unchanged.


### Porting loaders

The prototype for 'accept_file()' has been changed from:

* int idaapi accept_file(linput_t *li, char fileformatname[MAX_FILE_FORMAT_NAME], int n);

to:

* int idaapi accept_file(qstring \*fileformatname, qstring \*processor, linput_t \*li, const char \*filename);

The desired processor may be returned in the 'processor' output parameter.

The return value has been extended with flags 'ACCEPT_ARCHIVE' and
'ACCEPT_CONTINUE'.

Loaders can also process and extract archives now. If you detect an archive,
the return value for 'accept_file' should be ORed with the 'ACCEPT_ARCHIVE'
flag.
After extraction, all loaders are queried again, which means IDA can now
handle multiply nested archives.

Non-archive loaders should extend the return value with the 'ACCEPT_CONTINUE'
flag.


### Porting processor modules

WARNING: The global variables 'cmd' and 'uFlag' are gone.

Most APIs return or accept an 'insn_t' structure with instruction details.

The 'processor_t' structure has had many unused and obsolete fields removed,
such as 'flag2', 'rFiles', 'rFnames', 'rFdescs', and 'CPUregs'.

Most callbacks are now handled centrally via the 'notify()' function:

| original name  | new name         |
|----------------|------------------|
| header         | ev_out_header    |
| footer         | ev_out_footer    |
| segstart       | ev_out_segstart  |
| segend         | ev_out_segend    |
| assumes        | ev_out_assumes   |
| u_ana          | ev_ana_insn      |
| u_emu          | ev_emu_insn      |
| u_out          | ev_out_insn      |
| u_outop        | ev_out_operand   |
| d_out          | ev_out_data      |
| cmp_opnd       | ev_cmp_opnd      |
| can_have_type  | ev_can_have_type |
| is_far_jump    | ev_is_far_jump   |
| getreg         | ev_getreg        |


#### ana.cpp

Change the prototype of 'ana' from:

* int idaapi ana(void);

to:

* int idaapi ana(insn_t *_insn);

You may then declare an 'insn_t' reference variable to simplify your code:

* insn_t &insn = *_insn;

Then replace all uses of 'cmd' by 'insn'.
You will likely need to pass 'insn' to other helper functions that used 'cmd'.


#### emu.cpp

Change the prototype of 'emu' from:

* int idaapi emu(void);

to:

* int idaapi emu(const insn_t &insn);

Then replace all uses of 'cmd' by 'insn'.
You may need to adjust some code if it was relying on cmd being writeable.

#### out.cpp

The output functions now use a context structure ('outctx_t') instead of
operating on a global buffer.

You must declare a class inheriting from 'outctx_t' and override its methods
or add new ones for non-standard functions. For example:

<pre>
class out_myproc_t : public outctx_t
{
  void outreg(int r) { out_register(ph.reg_names[r]); }
  void outmem(const op_t &x, ea_t ea);
  bool outbit(ea_t ea, int bit);

  bool out_operand(const op_t &x);
  void out_insn(void);
  void out_mnem(void);
}
</pre>

Then use one of the two macros from idaidp.hpp:

* DECLARE_OUT_FUNCS_WITHOUT_OUTMNEM(out_myproc_t)

or, if you implement 'out_mnem':

* DECLARE_OUT_FUNCS(out_myproc_t)

Then prefix old function names with your class and rename them to match methods.
For example, from:

<pre>
  void idaapi out(void);
  void out_myproc_t::out(void);
</pre>

to:

<pre>
  bool idaapi outop(op_t &x);
  bool out_myproc_t::out_operand(const op_t &x);
</pre>

Then remove calls to 'init_output_buffer()' and uses of the buffer variable.

Other changes that must be made are:

- Replacing references to 'cmd' with 'insn';
- Replacing term_output_buffer()/MakeLine() sequence with flush_outbuf().

Most of the other code can remain intact or can be replaced by the new helper functions.

For other output-related callbacks, convert them to take an 'outctx_t &ctx' parameter and use its methods.
For example, from:

* void idaapi header(void);

to:

* void idaapi myproc_header(outctx_t &ctx)

See the changes to 'ua.hpp' below for more information on converting the
functions.

Also, see the SDK samples for more ideas.


#### reg.cpp

Remove the old callbacks from the 'processor_t' structure and call them
from the 'notify()' function instead. For example:

<pre>
    case processor_t::ev_ana_insn:
      {
        insn_t *out = va_arg(va, insn_t *);
        return myproc_ana(out);
      }
</pre>

For 'ev_out_insn', call 'out_insn()' generated by the macro in out.cpp:

<pre>
    case processor_t::ev_out_insn:
      {
        outctx_t *ctx = va_arg(va, outctx_t *);
        out_insn(*ctx);
        return 1;
      }
</pre>


### Porting notifications

* When hooking notifications, return 0 for "not handled" instead of 1 as before.
* Many notifications had their arguments types and/or order changed.
  Double-check your handlers against the new headers.
* Instead of calling ph.notify() or similar, prefer helper inline functions for
  additional type safety. For example, use 'ph.get_operand_string()' instead
  of 'ph.notify(processor_t::get_operand_string, ...)'.
* Some IDP events have been moved to the IDB event group (see the table below),
  so they should be handled on the HT_IDB level. You will need to move the
  corresponding code from the IDP notification hooks to the IDB hooks.

| original IDP event    | new IDB notification  |
|-----------------------|-----------------------|
| closebase             | closebase             |
| savebase              | savebase              |
| auto_empty            | auto_empty            |
| auto_empty_finally    | auto_empty_finally    |
| determined_main       | determined_main       |
| load_idasgn           | idasgn_loaded         |
| kernel_config_loaded  | kernel_config_loaded  |
| loader_finished       | loader_finished       |
| preprocess_chart      | flow_chart_created    |
| setsgr                | sgr_changed           |
| set_compiler          | compiler_changed      |
| move_segm             | segm_moved            |
| extlang_changed       | extlang_changed       |
| make_code             | make_code             |
| make_data             | make_data             |
| renamed               | renamed               |
| add_func              | func_added            |
| del_func              | deleting_func         |
| set_func_start        | set_func_start        |
| set_func_end          | set_func_end          |


### UI: Porting choosers

- Make a new class derived from 'chooser_t' or 'chooser_multi_t'.
  Its fields are similar to arguments of 'choose2()' from IDA 6.95.
- You should implement at least 2 methods:

  * 'get_count()', and
  * 'get_row()'.

  The 'get_row()' method combines 3 methods of 6.95's old 'chooser_info_t':

  * 'get_row()'
  * 'get_icon()', and
  * 'get_attrs()'

- If you want to show actions Ins/Delete/Edit/Refresh in a popup-menu
  you should set new bits 'CH_CAN_...' in the 'flags' member.
- The header line is stored in a new 'header' member.
- All indexes are now __0-based__. You can use new constant 'NO_SELECTION'
  for non-existing rows.
- The default value is not stored in the 'chooser_t' structure now and
  it is passed directly to the 'choose()' function.
- You can prepare a specialized version of the 'choose()' method that
  takes a special default value (e.g. an effective address). For this
  you should implement a new 'get_item_index()' method.
- The 'update()' callback has been renamed to 'refresh()' and it
  returns the cursor position after refresh. If the data has not
  changed this callback should return a 'NOTHING_CHANGED' hint.
- The returned value of the 'ins()', 'del()', 'edit()' and
  'exit()' callbacks are the same as for 'refresh()' callback.
  E.g. the 'ins()' callback may return the cursor position of the
  newly inserted item. Or the 'del()' callback may return
  'NOTHING_CHANGED' if it asked the user about the removal and he
  refused.
- The 'initializer()' callback has been renamed to 'init()'. Its use
  allows you to prepare data when it is __really__ needed
  (i.e., "lazy" populating).
- The 'destroyer()' callback has been renamed to 'closed()' and it is
  called when the chooser window is about to close. To clean up the
  chooser data you should use the destructor.
- The 'CH_MULTI' flag has been removed altogether. If you want to create
  a chooser with multiple selection, you should derive your
  class from 'chooser_multi_t'.
- While callbacks for the 'chooser_t' class would receive and return a
  single value specifying the currently-selected row, callbacks of the
  'chooser_multi_t' class will receive a vector of such values instead.
- In a similar fashion, instead of using the 'NO_SELECTION' constant,
  'chooser_multi_t' will use an empty vector.
- In contrast to IDA 6.95, the selected items are now all processed
  at once, in __one__ call to the 'ins()', 'del()', 'edit()' and 'exit()'
  callbacks (this greatly simplified implementing them.)


## Changes per file in the SDK

This section describes in detail the changes to the APIs for each file in the SDK.


### auto.hpp

NOTE: global variables 'auto_state', 'auto_display', and 'autoEnabled' have been removed.

- [1] output argument moved to beginning of argument list

| original name       | new name            |[1]| Notes                                                           |
|---------------------|---------------------|:-:|-----------------------------------------------------------------|
| autoGetName         | <**removed**>       |   |                                                                 |
| autoStep            | <**removed**>       |   |                                                                 |
| <**added**>         | auto_apply_tail     |   |                                                                 |
| <**added**>         | auto_recreate_insn  |   |                                                                 |
| <**added**>         | enable_auto         |   | to be used instead of 'autoEnabled'                             |
| <**added**>         | get_auto_display    |   | to be used instead of 'auto_display'                            |
| <**added**>         | get_auto_state      |   | to be used instead of 'auto_state'                              |
| <**added**>         | is_auto_enabled     |   | to be used instead of 'autoEnabled'                             |
| <**added**>         | set_auto_state      |   | to be used instead of 'auto_state'                              |
| analyze_area        | plan_and_wait       |   | added 'final_pass' argument (true for analyze_area behaviour)   |
| autoCancel          | auto_cancel         |   |                                                                 |
| autoIsOk            | auto_is_ok          |   |                                                                 |
| autoMark            | auto_mark           |   |                                                                 |
| autoUnmark          | auto_unmark         |   |                                                                 |
| autoWait            | auto_wait           |   |                                                                 |
| auto_get            |                     | * |                                                                 |
| noUsed              | plan_ea             |   | (ea_t ea) variant                                               |
| noUsed              | plan_range          |   | (ea_t sEA, ea_t eEA) variant                                    |
| setStat             | set_ida_state       |   |                                                                 |
| showAddr            | show_addr           |   |                                                                 |
| showAuto            | show_auto           |   |                                                                 |


### bitrange.hpp

| original name       | Notes                                    |
|---------------------|------------------------------------------|
| bitrange_t::extract | argument type: 'int' changed to 'size_t' |
| bitrange_t::inject  | argument type: 'int' changed to 'size_t' |


### bytes.hpp

NOTE: The misleading term "ASCII string" has been replaced by "string literal" (strlit).

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name                 | new name                       |[1]|[2]| Notes                                                                                                                   |
|-------------------------------|--------------------------------|:-:|:-:|-------------------------------------------------------------------------------------------------------------------------|
| clrFlbits                     | <**removed**>                  |   |   |                                                                                                                         |
| do3byte                       | <**removed**>                  |   |   |                                                                                                                         |
| doASCI                        | <**removed**>                  |   |   |                                                                                                                         |
| doVar                         | <**removed**>                  |   |   |                                                                                                                         |
| do_unknown                    | <**removed**>                  |   |   | use 'del_items' instead                                                                                                 |
| do_unknown_range              | <**removed**>                  |   |   | use 'del_items' instead                                                                                                 |
| f_is3byte                     | <**removed**>                  |   |   |                                                                                                                         |
| getRadixEA                    | <**removed**>                  |   |   |                                                                                                                         |
| get_3byte                     | <**removed**>                  |   |   |                                                                                                                         |
| get_many_bytes                | <**removed**>                  |   |   | use 'get_bytes' instead                                                                                                 |
| get_many_bytes_ex             | <**removed**>                  |   |   | use 'get_bytes' instead                                                                                                 |
| is3byte                       | <**removed**>                  |   |   |                                                                                                                         |
| isVar                         | <**removed**>                  |   |   |                                                                                                                         |
| noImmd                        | <**removed**>                  |   |   |                                                                                                                         |
| setFlags                      | <**removed**>                  |   |   |                                                                                                                         |
| setFlbits                     | <**removed**>                  |   |   |                                                                                                                         |
| tribyteflag                   | <**removed**>                  |   |   |                                                                                                                         |
| <**added**>                   | add_mapping                    |   |   |                                                                                                                         |
| <**added**>                   | attach_custom_data_format      |   |   |                                                                                                                         |
| <**added**>                   | del_items                      |   |   |                                                                                                                         |
| <**added**>                   | del_mapping                    |   |   |                                                                                                                         |
| <**added**>                   | detach_custom_data_format      |   |   |                                                                                                                         |
| <**added**>                   | get_bytes                      |   |   |                                                                                                                         |
| <**added**>                   | get_first_hidden_range         |   |   |                                                                                                                         |
| <**added**>                   | get_last_hidden_range          |   |   |                                                                                                                         |
| <**added**>                   | get_mapping                    |   |   |                                                                                                                         |
| <**added**>                   | get_mappings_qty               |   |   |                                                                                                                         |
| <**added**>                   | is_attached_custom_data_format |   |   |                                                                                                                         |
| <**added**>                   | revert_byte                    |   |   |                                                                                                                         |
| <**added**>                   | update_hidden_range            |   |   |                                                                                                                         |
| <**added**>                   | use_mapping                    |   |   |                                                                                                                         |
| add_hidden_area               | add_hidden_range               |   |   |                                                                                                                         |
| alignflag                     | align_flag                     |   |   |                                                                                                                         |
| asciflag                      | strlit_flag                    |   |   |                                                                                                                         |
| binflag                       | bin_flag                       |   |   |                                                                                                                         |
| byteflag                      | byte_flag                      |   |   |                                                                                                                         |
| charflag                      | char_flag                      |   |   |                                                                                                                         |
| chunksize                     | chunk_size                     |   |   |                                                                                                                         |
| chunkstart                    | chunk_start                    |   |   |                                                                                                                         |
| codeflag                      | code_flag                      |   |   |                                                                                                                         |
| custflag                      | cust_flag                      |   |   |                                                                                                                         |
| custfmtflag                   | custfmt_flag                   |   |   |                                                                                                                         |
| decflag                       | dec_flag                       |   |   |                                                                                                                         |
| delValue                      | del_value                      |   |   |                                                                                                                         |
| del_hidden_area               | del_hidden_range               |   |   |                                                                                                                         |
| do16bit                       | create_16bit_data              |   |   |                                                                                                                         |
| do32bit                       | create_32bit_data              |   |   |                                                                                                                         |
| doAlign                       | create_align                   |   |   |                                                                                                                         |
| doByte                        | create_byte                    |   |   |                                                                                                                         |
| doCustomData                  | create_custdata                |   |   |                                                                                                                         |
| doDouble                      | create_double                  |   |   |                                                                                                                         |
| doDwrd                        | create_dword                   |   |   |                                                                                                                         |
| doFloat                       | create_float                   |   |   |                                                                                                                         |
| doImmd                        | set_immd                       |   |   |                                                                                                                         |
| doOwrd                        | create_oword                   |   |   |                                                                                                                         |
| doPackReal                    | create_packed_real             |   |   |                                                                                                                         |
| doQwrd                        | create_qword                   |   |   |                                                                                                                         |
| doStruct                      | create_struct                  |   |   |                                                                                                                         |
| doTbyt                        | create_tbyte                   |   |   |                                                                                                                         |
| doWord                        | create_word                    |   |   |                                                                                                                         |
| doYwrd                        | create_yword                   |   |   |                                                                                                                         |
| doZwrd                        | create_zword                   |   |   |                                                                                                                         |
| do_data_ex                    | create_data                    |   |   |                                                                                                                         |
| doubleflag                    | double_flag                    |   |   |                                                                                                                         |
| dwrdflag                      | dword_flag                     |   |   |                                                                                                                         |
| enumflag                      | enum_flag                      |   |   |                                                                                                                         |
| f_hasRef                      | f_has_xref                     |   |   |                                                                                                                         |
| f_isASCII                     | f_is_strlit                    |   |   |                                                                                                                         |
| f_isAlign                     | f_is_align                     |   |   |                                                                                                                         |
| f_isByte                      | f_is_byte                      |   |   |                                                                                                                         |
| f_isCode                      | f_is_code                      |   |   |                                                                                                                         |
| f_isCustom                    | f_is_custom                    |   |   |                                                                                                                         |
| f_isData                      | f_is_data                      |   |   |                                                                                                                         |
| f_isDouble                    | f_is_double                    |   |   |                                                                                                                         |
| f_isDwrd                      | f_is_dword                     |   |   |                                                                                                                         |
| f_isFloat                     | f_is_float                     |   |   |                                                                                                                         |
| f_isHead                      | f_is_head                      |   |   |                                                                                                                         |
| f_isNotTail                   | f_is_not_tail                  |   |   |                                                                                                                         |
| f_isOwrd                      | f_is_oword                     |   |   |                                                                                                                         |
| f_isPackReal                  | f_is_pack_real                 |   |   |                                                                                                                         |
| f_isQwrd                      | f_is_qword                     |   |   |                                                                                                                         |
| f_isStruct                    | f_is_struct                    |   |   |                                                                                                                         |
| f_isTail                      | f_is_tail                      |   |   |                                                                                                                         |
| f_isTbyt                      | f_is_tbyte                     |   |   |                                                                                                                         |
| f_isWord                      | f_is_word                      |   |   |                                                                                                                         |
| f_isYwrd                      | f_is_yword                     |   |   |                                                                                                                         |
| floatflag                     | float_flag                     |   |   |                                                                                                                         |
| fltflag                       | flt_flag                       |   |   |                                                                                                                         |
| freechunk                     | free_chunk                     |   |   |                                                                                                                         |
| getDefaultRadix               | get_default_radix              |   |   |                                                                                                                         |
| getFlags                      | get_full_flags                 |   |   | WARNING: 'getFlags' has not been renamed to 'get_flags'                                                                 |
| get_long                      | get_dword                      |   |   |                                                                                                                         |
| get_full_byte                 | get_wide_byte                  |   |   |                                                                                                                         |
| get_full_word                 | get_wide_word                  |   |   |                                                                                                                         |
| get_full_long                 | get_wide_dword                 |   |   |                                                                                                                         |
| get_original_long             | get_original_dword             |   |   |                                                                                                                         |
| put_long                      | put_dword                      |   |   |                                                                                                                         |
| patch_long                    | patch_dword                    |   |   |                                                                                                                         |
| add_long                      | add_dword                      |   |   |                                                                                                                         |
| getRadix                      | get_radix                      |   |   |                                                                                                                         |
| get_ascii_contents2           | get_strlit_contents            | q | * | return type changed from 'bool' to 'ssize_t'; output argument 'usedsize' (in bytes) changed to 'maxcps' (in codepoints) |
| get_cmt                       |                                | q | * |                                                                                                                         |
| get_custom_data_format        |                                |   |   | removed 'dtid' argument                                                                                                 |
| get_data_value                |                                | * |   |                                                                                                                         |
| get_enum_id                   |                                | * |   |                                                                                                                         |
| get_flags_novalue             | get_flags                      |   |   | WARNING: 'getFlags' has not been renamed to 'get_flags'                                                                 |
| get_forced_operand            |                                | q | * |                                                                                                                         |
| get_hidden_area               | get_hidden_range               |   |   | return type: 'hidden_area_t *' has been renamed to 'hidden_range_t \*'                                                  |
| get_hidden_area_num           | get_hidden_range_num           |   |   |                                                                                                                         |
| get_hidden_area_qty           | get_hidden_range_qty           |   |   |                                                                                                                         |
| get_manual_insn               |                                | q | * | return type changed from 'char *' to 'ssize_t';                                                                         |
| get_max_ascii_length          | get_max_strlit_length          |   |   |                                                                                                                         |
| get_next_hidden_range         | get_next_hidden_area           |   |   | return type: 'hidden_area_t *' has been renamed to 'hidden_range_t \*'                                                  |
| get_opinfo                    |                                | * |   |                                                                                                                         |
| get_predef_insn_cmt           |                                | q | * | moved from ints.hpp                                                                                                     |
| get_prev_hidden_range         | get_prev_hidden_area           |   |   | return type: 'hidden_area_t *' has been renamed to 'hidden_range_t \*'                                                  |
| get_stroff_path               |                                | * |   |                                                                                                                         |
| get_zero_areas                | get_zero_ranges                |   |   | argument type: 'areaset_t' has been renamed to 'rangeset_t'                                                             |
| getn_hidden_area              | getn_hidden_range              |   |   | return type: 'hidden_area_t *' has been renamed to 'hidden_range_t \*'                                                  |
| hasExtra                      | has_extra_cmts                 |   |   |                                                                                                                         |
| hasRef                        | has_xref                       |   |   |                                                                                                                         |
| hasValue                      | has_value                      |   |   |                                                                                                                         |
| hexflag                       | hex_flag                       |   |   |                                                                                                                         |
| isASCII                       | is_strlit                      |   |   |                                                                                                                         |
| isAlign                       | is_align                       |   |   |                                                                                                                         |
| isByte                        | is_byte                        |   |   |                                                                                                                         |
| isChar                        | is_char                        |   |   |                                                                                                                         |
| isChar0                       | is_char0                       |   |   |                                                                                                                         |
| isChar1                       | is_char1                       |   |   |                                                                                                                         |
| isCode                        | is_code                        |   |   |                                                                                                                         |
| isCustFmt                     | is_custfmt                     |   |   |                                                                                                                         |
| isCustFmt0                    | is_custfmt0                    |   |   |                                                                                                                         |
| isCustFmt1                    | is_custfmt1                    |   |   |                                                                                                                         |
| isCustom                      | is_custom                      |   |   |                                                                                                                         |
| isData                        | is_data                        |   |   |                                                                                                                         |
| isDefArg                      | is_defarg                      |   |   |                                                                                                                         |
| isDefArg0                     | is_defarg0                     |   |   |                                                                                                                         |
| isDefArg1                     | is_defarg1                     |   |   |                                                                                                                         |
| isDouble                      | is_double                      |   |   |                                                                                                                         |
| isDwrd                        | is_dword                       |   |   |                                                                                                                         |
| isEnabled                     | is_mapped                      |   |   |                                                                                                                         |
| isEnum                        | is_enum                        |   |   |                                                                                                                         |
| isEnum0                       | is_enum0                       |   |   |                                                                                                                         |
| isEnum1                       | is_enum1                       |   |   |                                                                                                                         |
| isFloat                       | is_float                       |   |   |                                                                                                                         |
| isFloat0                      | is_float0                      |   |   |                                                                                                                         |
| isFloat1                      | is_float1                      |   |   |                                                                                                                         |
| isFlow                        | is_flow                        |   |   |                                                                                                                         |
| isFltnum                      | is_fltnum                      |   |   |                                                                                                                         |
| isFop                         | is_manual                      |   |   |                                                                                                                         |
| isFunc                        | is_func                        |   |   |                                                                                                                         |
| isHead                        | is_head                        |   |   |                                                                                                                         |
| isImmd                        | has_immd                       |   |   |                                                                                                                         |
| isLoaded                      | is_loaded                      |   |   |                                                                                                                         |
| isNotTail                     | is_not_tail                    |   |   |                                                                                                                         |
| isNum                         | is_numop                       |   |   |                                                                                                                         |
| isNum0                        | is_numop0                      |   |   |                                                                                                                         |
| isNum1                        | is_numop1                      |   |   |                                                                                                                         |
| isOff                         | is_off                         |   |   |                                                                                                                         |
| isOff0                        | is_off0                        |   |   |                                                                                                                         |
| isOff1                        | is_off1                        |   |   |                                                                                                                         |
| isOwrd                        | is_oword                       |   |   |                                                                                                                         |
| isPackReal                    | is_pack_real                   |   |   |                                                                                                                         |
| isQwrd                        | is_qword                       |   |   |                                                                                                                         |
| isSeg                         | is_seg                         |   |   |                                                                                                                         |
| isSeg0                        | is_seg0                        |   |   |                                                                                                                         |
| isSeg1                        | is_seg1                        |   |   |                                                                                                                         |
| isStkvar                      | is_stkvar                      |   |   |                                                                                                                         |
| isStkvar0                     | is_stkvar0                     |   |   |                                                                                                                         |
| isStkvar1                     | is_stkvar1                     |   |   |                                                                                                                         |
| isStroff                      | is_stroff                      |   |   |                                                                                                                         |
| isStroff0                     | is_stroff0                     |   |   |                                                                                                                         |
| isStroff1                     | is_stroff1                     |   |   |                                                                                                                         |
| isStruct                      | is_struct                      |   |   |                                                                                                                         |
| isTail                        | is_tail                        |   |   |                                                                                                                         |
| isTbyt                        | is_tbyte                       |   |   |                                                                                                                         |
| isUnknown                     | is_unknown                     |   |   |                                                                                                                         |
| isVoid                        | is_suspop                      |   |   |                                                                                                                         |
| isWord                        | is_word                        |   |   |                                                                                                                         |
| isYwrd                        | is_yword                       |   |   |                                                                                                                         |
| isZwrd                        | is_zword                       |   |   |                                                                                                                         |
| make_ascii_string             | create_strlit                  |   |   |                                                                                                                         |
| nextaddr                      | next_addr                      |   |   |                                                                                                                         |
| nextchunk                     | next_chunk                     |   |   |                                                                                                                         |
| nextthat                      | next_that                      |   |   |                                                                                                                         |
| noType                        | clr_op_type                    |   |   |                                                                                                                         |
| numflag                       | num_flag                       |   |   |                                                                                                                         |
| octflag                       | oct_flag                       |   |   |                                                                                                                         |
| offflag                       | off_flag                       |   |   |                                                                                                                         |
| op_stroff                     |                                |   |   | converted input 'ea_t' argument to 'const insn_t &'                                                                     |
| owrdflag                      | oword_flag                     |   |   |                                                                                                                         |
| packrealflag                  | packreal_flag                  |   |   |                                                                                                                         |
| patch_many_bytes              | patch_bytes                    |   |   |                                                                                                                         |
| prevaddr                      | prev_addr                      |   |   |                                                                                                                         |
| prevchunk                     | prev_chunk                     |   |   |                                                                                                                         |
| prevthat                      | prev_that                      |   |   |                                                                                                                         |
| print_ascii_string_type       | print_strlit_type              | q | * | return type changed from 'char *' to 'bool'; added 'out_tooltip' and 'flags' arguments                                  |
| put_many_bytes                | put_bytes                      |   |   |                                                                                                                         |
| qwrdflag                      | qword_flag                     |   |   |                                                                                                                         |
| register_custom_data_format   |                                |   |   | removed 'dtid' argument                                                                                                 |
| segflag                       | seg_flag                       |   |   |                                                                                                                         |
| set_opinfo                    |                                |   |   | added 'suppress_events' argument                                                                                        |
| stkvarflag                    | stkvar_flag                    |   |   |                                                                                                                         |
| stroffflag                    | stroff_flag                    |   |   |                                                                                                                         |
| struflag                      | stru_flag                      |   |   |                                                                                                                         |
| tbytflag                      | tbyte_flag                     |   |   |                                                                                                                         |
| unregister_custom_data_format |                                |   |   | removed 'dtid' argument                                                                                                 |
| wordflag                      | word_flag                      |   |   |                                                                                                                         |
| ywrdflag                      | yword_flag                     |   |   |                                                                                                                         |
| zwrdflag                      | zword_flag                     |   |   |                                                                                                                         |


### compress.hpp

| original name            | new name              |
|--------------------------|-----------------------|
| process_zipfile64        | process_zipfile       |
| process_zipfile_entry64  | process_zipfile_entry |


### config.hpp (**NEW** file)

| original name                | Notes              |
|------------------------------|--------------------|
| cfg_get_cc_header_path       | moved from idp.hpp |
| cfg_get_cc_parm              | moved from idp.hpp |
| cfg_get_cc_predefined_macros | moved from idp.hpp |
| cfgopt_t__apply              | moved from idp.hpp |
| parse_config_value           | moved from idp.hpp |
| read_config                  | moved from idp.hpp |
| read_config_file             | moved from idp.hpp |
| read_config_string           | moved from idp.hpp |


### dbg.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name               | new name                     |[1]|[2]| Notes                                                                                            |
|-----------------------------|------------------------------|:-:|:-:|--------------------------------------------------------------------------------------------------|
| get_process_info            | <**removed**>                |   |   | use 'get_processes' instead                                                                      |
| get_process_qty             | <**removed**>                |   |   | use 'get_processes' instead                                                                      |
| getn_process                | <**removed**>                |   |   | use 'get_processes' instead                                                                      |
| <**added**>                 | bpt_t::get_cnd_elang_idx     |   |   |                                                                                                  |
| <**added**>                 | get_ip_val                   |   |   |                                                                                                  |
| <**added**>                 | get_sp_val                   |   |   |                                                                                                  |
| bpt_location_t::print       |                              |   | * |                                                                                                  |
| choose_trace_file           |                              |   | * |                                                                                                  |
| create_source_viewer        |                              |   |   | argument type: 'TWinControl' and 'TCustomControl' changed to 'TWidget'; added 'out_ccv' argument |
| get_dbg_byte                |                              | * |   |                                                                                                  |
| get_trace_file_desc         |                              | q | * |                                                                                                  |
| internal_get_sreg_base      |                              | * |   |                                                                                                  |
| load_trace_file             |                              | q | * |                                                                                                  |
| source_file_t::open_srcview |                              |   |   | argument type: 'TCustomControl' changed to 'TWidget'                                             |
| source_item_t::get_hint     |                              | q |   |                                                                                                  |
| source_item_t::get_kind     | source_item_t::get_item_kind |   |   |                                                                                                  |


### diskio.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] return type changed from '[u]int32' to '[u]int64'/'qoff64_t'
- [4] input argument changed from '[u]int32' to '[u]int64'/'qoff64_t'

| original name         | new name            |[1]|[2]|[3]|[4]| Notes                                                                                     |
|-----------------------|---------------------|:-:|:-:|:-:|:-:|-------------------------------------------------------------------------------------------|
| create_generic_linput | <**removed**>       |   |   |   |   |                                                                                           |
| echsize64             | <**removed**>       |   |   |   |   |                                                                                           |
| ecreateT              | <**removed**>       |   |   |   |   |                                                                                           |
| eseek64               | <**removed**>       |   |   |   |   |                                                                                           |
| free_ioports          | <**removed**>       |   |   |   |   |                                                                                           |
| qfsize64              | <**removed**>       |   |   |   |   |                                                                                           |
| qlgetz64              | <**removed**>       |   |   |   |   |                                                                                           |
| qlseek64              | <**removed**>       |   |   |   |   |                                                                                           |
| qlsize64              | <**removed**>       |   |   |   |   |                                                                                           |
| qltell64              | <**removed**>       |   |   |   |   |                                                                                           |
| choose_ioport_device  |                     | q | * |   |   |                                                                                           |
| echsize               |                     |   |   |   | * |                                                                                           |
| eseek                 |                     |   |   |   | * |                                                                                           |
| find_ioport           |                     |   |   |   |   | input argument converted to 'const ioports_t &'                                           |
| find_ioport_bit       |                     |   |   |   |   | input argument converted to 'const ioports_t &'                                           |
| get_special_folder    |                     | * |   |   |   |                                                                                           |
| getdspace             | get_free_disk_space |   |   |   |   |                                                                                           |
| qfsize                |                     |   |   | * |   |                                                                                           |
| qlgetz                |                     |   |   |   | * |                                                                                           |
| qlseek                |                     |   |   | * | * |                                                                                           |
| qlsize                |                     |   |   | * |   |                                                                                           |
| qltell                |                     |   |   | * |   |                                                                                           |
| read_ioports          |                     |   |   |   |   | return type changed from 'ioport_t *' to 'ssize_t'; output argument converted 'ioports_t' |


### entry.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] added 'flags' argument

| original name       |[1]|[2]|[3]|
|---------------------|:-:|:-:|:-:|
| add_entry           |   |   | * |
| get_entry_forwarder | q | * |   |
| get_entry_name      | q | * |   |
| rename_entry        |   |   | * |
| set_entry_forwarder |   |   | * |


### enum.hpp

NOTE: global variable 'enums' has been removed.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name                | new name      |[1]|[2]|
|------------------------------|---------------|:-:|:-:|
| add_selected_enum            | <**removed**> |   |   |
| get_bmask_node               | <**removed**> |   |   |
| get_selected_enum            | <**removed**> |   |   |
| init_enums                   | <**removed**> |   |   |
| save_enums                   | <**removed**> |   |   |
| set_enum_flag                | <**removed**> |   |   |
| term_enums                   | <**removed**> |   |   |
| unmark_selected_enums        | <**removed**> |   |   |
| get_bmask_cmt                |               | q | * |
| get_enum_cmt                 |               | q | * |
| get_enum_member_cmt          |               | q | * |
| get_first_serial_enum_member |               | * |   |
| get_last_serial_enum_member  |               | * |   |
| get_next_serial_enum_member  |               | * |   |
| get_prev_serial_enum_member  |               | * |   |


### err.h

| original name | Notes                                                               |
|---------------|---------------------------------------------------------------------|
| qstrerror     | buf argument removed; returns string in static buffer (thread-safe) |


### expr.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] input argument changed from pointer to reference

| original name                 | new name                      |[1]|[2]|[3]| Notes                                                                                             |
|-------------------------------|-------------------------------|:-:|:-:|:-:|---------------------------------------------------------------------------------------------------|
| ExecuteFile                   | <**removed**>                 |   |   |   | use 'exec_idc_script' instead                                                                     |
| ExecuteLine, execute          | <**removed**>                 |   |   |   | use 'eval_idc_snippet' instead                                                                    |
| call_idc_method               | <**removed**>                 |   |   |   |                                                                                                   |
| call_script_method            | <**removed**>                 |   |   |   | use 'extlang_t::call_method' instead                                                              |
| compile_script_file           | <**removed**>                 |   |   |   | use 'extlang_t::compile_file' instead                                                             |
| compile_script_func           | <**removed**>                 |   |   |   | use 'extlang_t::compile_expr' instead                                                             |
| create_idc_object             | <**removed**>                 |   |   |   |                                                                                                   |
| create_script_object          | <**removed**>                 |   |   |   | use 'extlang_t::create_object' instead                                                            |
| extlang_call_method_exists    | <**removed**>                 |   |   |   | 'extlang_t::call_method' should always exist                                                      |
| extlang_compile_file          | <**removed**>                 |   |   |   | use 'extlang_t::compile_file' instead                                                             |
| extlang_compile_file_exists   | <**removed**>                 |   |   |   | 'extlang_t::compile_file' should always exist                                                     |
| extlang_create_object_exists  | <**removed**>                 |   |   |   | 'extlang_t::create_object' should always exist                                                    |
| extlang_get_attr_exists       | <**removed**>                 |   |   |   | 'extlang_t::get_attr' should always exist                                                         |
| extlang_run_statements_exists | <**removed**>                 |   |   |   | replaced by 'extlang_t::eval_statements', which should always exist                               |
| extlang_set_attr_exists       | <**removed**>                 |   |   |   | 'extlang_t::set_attr' should always exist                                                         |
| extlang_unload_procmod        | <**removed**>                 |   |   |   | use 'extlang_t::unload_procmod' instead                                                           |
| get_extlang_fileext           | <**removed**>                 |   |   |   | use 'extlang_t::fileext' instead                                                                  |
| get_extlangs                  | <**removed**>                 |   |   |   | use 'for_all_extlangs' instead                                                                    |
| get_idc_func_body             | <**removed**>                 |   |   |   |                                                                                                   |
| get_script_attr               | <**removed**>                 |   |   |   | use 'extlang_t::get_attr' instead                                                                 |
| run_script_func               | <**removed**>                 |   |   |   | use 'extlang_t::call_func' instead                                                                |
| run_statements                | <**removed**>                 |   |   |   | use 'extlang_t::eval_statements' instead                                                          |
| set_idc_func_body             | <**removed**>                 |   |   |   |                                                                                                   |
| set_idc_func_ex               | <**removed**>                 |   |   |   | use 'add_idc_func'/'del_idc_func' instead                                                         |
| set_script_attr               | <**removed**>                 |   |   |   | use 'extlang_t::set_attr' instead                                                                 |
| <**added**>                   | add_idc_func                  |   |   |   | to be used instead of 'set_idc_func_ex'                                                           |
| <**added**>                   | compile_idc_snippet           |   |   |   |                                                                                                   |
| <**added**>                   | del_idc_func                  |   |   |   | to be used instead of 'set_idc_func_ex'                                                           |
| <**added**>                   | eval_idc_snippet              |   |   |   |                                                                                                   |
| <**added**>                   | find_extlang_by_index         |   |   |   |                                                                                                   |
| <**added**>                   | find_idc_func                 |   |   |   |                                                                                                   |
| <**added**>                   | for_all_extlangs              |   |   |   |                                                                                                   |
| <**added**>                   | get_extlang                   |   |   |   | always returns non-NULL                                                                           |
| Compile, CompileEx            | compile_idc_file              | q | * |   |                                                                                                   |
| CompileLine, CompileLineEx    | compile_idc_text              |   | * |   | added 'resolver' argument                                                                         |
| Run                           | call_idc_func                 | * | * |   | swapped 'argsnum' and 'args'; argument type: 'int' changed to 'size_t'; added 'resolver' argument |
| VarAssign                     | copy_idcv                     |   |   | * |                                                                                                   |
| VarCopy                       | deep_copy_idcv                |   |   | * |                                                                                                   |
| VarDelAttr                    | del_idcv_attr                 |   |   |   |                                                                                                   |
| VarDeref                      | deref_idcv                    |   |   |   |                                                                                                   |
| VarFirstAttr                  | first_idcv_attr               |   |   |   |                                                                                                   |
| VarFloat                      | idcv_float                    |   |   |   |                                                                                                   |
| VarFree                       | free_idcv                     |   |   |   |                                                                                                   |
| VarGetAttr                    | get_idcv_attr                 | * |   |   |                                                                                                   |
| VarGetClassName               | get_idcv_class_name           | q |   |   |                                                                                                   |
| VarGetSlice                   | get_idcv_slice                | * |   |   |                                                                                                   |
| VarInt64                      | idcv_int64                    |   |   |   |                                                                                                   |
| VarLastAttr                   | last_idcv_attr                |   |   |   |                                                                                                   |
| VarLong                       | idcv_long                     |   |   |   |                                                                                                   |
| VarMove                       | move_idcv                     |   |   |   |                                                                                                   |
| VarNextAttr                   | next_idcv_attr                |   |   |   |                                                                                                   |
| VarNum                        | idcv_num                      |   |   |   |                                                                                                   |
| VarObject                     | idcv_object                   |   |   |   |                                                                                                   |
| VarPrevAttr                   | prev_idcv_attr                |   |   |   |                                                                                                   |
| VarPrint                      | print_idcv                    |   |   | * |                                                                                                   |
| VarRef                        | create_idcv_ref               |   |   |   |                                                                                                   |
| VarSetAttr                    | set_idcv_attr                 |   |   | * |                                                                                                   |
| VarSetSlice                   | set_idcv_slice                |   |   | * |                                                                                                   |
| VarString2                    | idcv_string                   |   |   |   |                                                                                                   |
| VarSwap                       | swap_idcvs                    |   |   |   |                                                                                                   |
| calc_idc_expr                 | eval_idc_expr                 | * | * |   |                                                                                                   |
| calcexpr                      | eval_expr                     | * | * |   |                                                                                                   |
| calcexpr_long                 | eval_expr_long                | * | * |   |                                                                                                   |
| dosysfile                     | exec_system_script            |   |   |   | argument order has swapped                                                                        |
| find_extlang_by_ext           |                               |   |   |   | return type changed from 'const extlang_t *' to 'extlang_object_t'                                |
| find_extlang_by_name          |                               |   |   |   | return type changed from 'const extlang_t *' to 'extlang_object_t'                                |
| install_extlang               |                               |   |   |   | removed const from 'el' argument; return type changed from 'bool' to 'ssize_t'                    |
| remove_extlang                |                               |   |   |   | removed const from 'el' argument                                                                  |
| select_extlang                |                               |   |   |   | removed const from 'el' argument                                                                  |


### fixup.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] input argument changed from pointer to reference

| original name                 | new name              |[1]|[2]|[3]| Notes                                                                                                                                        |
|-------------------------------|-----------------------|:-:|:-:|:-:|----------------------------------------------------------------------------------------------------------------------------------------------|
| get_fixup_base                | <**removed**>         |   |   |   | use 'fd.get_base()' instead                                                                                                                  |
| get_fixup_extdef_ea           | <**removed**>         |   |   |   | use 'fd.get_base() + fd.off' instead                                                                                                         |
| get_fixup_segdef_sel          | <**removed**>         |   |   |   | use 'fd.sel' instead                                                                                                                         |
| set_custom_fixup              | <**removed**>         |   |   |   | use 'set_fixup' instead                                                                                                                      |
| set_custom_fixup_ex           | <**removed**>         |   |   |   | use 'set_fixup' instead                                                                                                                      |
| set_fixup_ex                  | <**removed**>         |   |   |   |                                                                                                                                              |
| <**added**>                   | calc_fixup_size       |   |   |   |                                                                                                                                              |
| <**added**>                   | exists_fixup          |   |   |   |                                                                                                                                              |
| <**added**>                   | find_custom_fixup     |   |   |   | to be used instead of 'create_custom_fixup' (from idp.hpp)                                                                                   |
| <**added**>                   | get_fixup_handler     |   |   |   |                                                                                                                                              |
| <**added**>                   | get_fixup_value       |   |   |   |                                                                                                                                              |
| <**added**>                   | get_fixups            |   |   |   |                                                                                                                                              |
| <**added**>                   | is_fixup_custom       |   |   |   |                                                                                                                                              |
| <**added**>                   | patch_fixup_value     |   |   |   |                                                                                                                                              |
| get_fixup                     |                       | * |   |   |                                                                                                                                              |
| get_fixup_desc                |                       | q | * | * | return type changed from 'char *' to 'const char \*'                                                                                         |
| register_custom_fixup         |                       |   |   |   | input argument changed from 'const fixup_handler_t *' to 'const custom_fixup_handler_t \*'; return type changed from 'int' to 'fixup_type_t' |
| set_fixup                     |                       |   |   | * |                                                                                                                                              |
| unregister_custom_fixup       |                       |   |   |   | input argument changed from 'int' to 'fixup_type_t'                                                                                          |


### fpro.h

- [1] input argument changed from 'int32' to 'qoff64_t'
- [2] return type changed from 'int32' to 'qoff64_t'

| original name | new name  |   |   |
|---------------|-----------|---|---|
| <**added**>   | qaccess   |   |   |
| <**added**>   | qgetline  |   |   |
| qcopyfile64   | qcopyfile |   |   |
| qfseek64      | qfseek    | * |   |
| qftell64      | qftell    |   | * |


### frame.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] input argument 'func_t *pfn' made const

| original name            | new name        |[1]|[2]|[3]| Notes                                               |
|--------------------------|-----------------|:-:|:-:|:-:|-----------------------------------------------------|
| add_auto_stkpnt2         | add_auto_stkpnt |   |   |   |                                                     |
| add_stkvar2              | define_stkvar   |   |   |   |                                                     |
| add_stkvar3              | add_stkvar      |   |   |   | added 'const insn_t &' input argument               |
| build_stkvar_name        |                 | q | * | * | return type changed from 'char *' to 'ssize_t'      |
| calc_stkvar_struc_offset |                 |   |   |   | converted input 'ea_t' argument to 'const insn_t &' |
| frame_off_args           |                 |   |   | * |                                                     |
| frame_off_lvars          |                 |   |   | * |                                                     |
| frame_off_retaddr        |                 |   |   | * |                                                     |
| frame_off_savregs        |                 |   |   | * |                                                     |
| get_frame_part           |                 | * |   | * | argument type: 'area_t' changed to 'range_t'        |
| get_frame_retsize        |                 |   |   | * |                                                     |
| get_frame_size           |                 |   |   | * |                                                     |
| get_stkvar               |                 | * |   |   | added 'const insn_t &' input argument               |
| is_funcarg_off           |                 |   |   | * |                                                     |
| lvar_off                 |                 |   |   | * |                                                     |


### funcs.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name                               | new name             |[1]|[2]| Notes                                                                                                   |
|---------------------------------------------|----------------------|:-:|:-:|---------------------------------------------------------------------------------------------------------|
| a2funcoff                                   | <**removed**>        |   |   |                                                                                                         |
| apply_idasgn                                | <**removed**>        |   |   |                                                                                                         |
| clear_func_struct                           | <**removed**>        |   |   |                                                                                                         |
| del_func_cmt                                | <**removed**>        |   |   | use 'set_func_cmt("")' instead                                                                          |
| std_gen_func_header                         | <**removed**>        |   |   | use 'outctx_base_t::gen_func_header' instead                                                            |
| <**added**>                                 | is_same_func         |   |   |                                                                                                         |
| <**added**>                                 | lock_func_range      |   |   |                                                                                                         |
| <**added**>                                 | reanalyze_noret_flag |   |   |                                                                                                         |
| add_func                                    |                      |   |   | second 'ea_t' argument made optional                                                                    |
| add_regarg2                                 | add_regarg           |   |   |                                                                                                         |
| find_func_bounds                            |                      | * |   | removed 'ea' argument                                                                                   |
| func_item_iterator_t::decode_preceding_insn |                      |   |   | added 'insn_t *' output argument                                                                        |
| func_item_iterator_t::decode_prev_insn      |                      |   |   | added 'insn_t *' output argument                                                                        |
| func_setend                                 | set_func_end         |   |   |                                                                                                         |
| func_setstart                               | set_func_start       |   |   |                                                                                                         |
| get_func_bits                               |                      |   |   | input argument 'func_t *' made const                                                                    |
| get_func_bytes                              |                      |   |   | input argument 'func_t *' made const                                                                    |
| get_func_cmt                                |                      | q | * | return type changed from 'char *' to 'ssize_t'                                                          |
| get_func_limits                             | get_func_ranges      | * |   | output argument converted from 'area_t *' to 'rangeset_t \*'; return type changed from 'bool' to 'ea_t' |
| get_func_name2                              | get_func_name        |   |   |                                                                                                         |
| get_idasgn_desc                             |                      | q | * |                                                                                                         |
| get_idasgn_title                            |                      | q | * | return type changed from 'char *' to 'ssize_t'                                                          |
| set_func_cmt                                |                      |   |   | input argument 'func_t *' made const                                                                    |


### gdl.hpp

| original name                 | Notes                                                       |
|-------------------------------|-------------------------------------------------------------|
| create_multirange_qflow_chart | argument type: 'areavec_t' has been renamed to 'rangevec_t' |


### graph.hpp

- [1] input argument changed from 'TCustomControl *' to 'graph_viewer_t \*'

| original name                | new name       |[1]| Notes                                                                     |
|------------------------------|----------------|:-:|---------------------------------------------------------------------------|
| set_graph_dispatcher         | <**removed**>  |   | use 'hook_to_notification_point(HT_GRAPH, [...])' instead                 |
| viewer_add_menu_item         | <**removed**>  |   | use 'viewer_attach_menu_item' instead                                     |
| viewer_del_menu_item         | <**removed**>  |   |                                                                           |
| <**added**>                  | viewer_get_gli |   |                                                                           |
| clr_node_info2               | clr_node_info  |   |                                                                           |
| create_disasm_graph          |                |   | argument type: 'areavec_t' has been renamed to 'rangevec_t'               |
| create_graph_viewer          |                |   | added 'title' argument; 'parent' argument made optional and reordered     |
| del_node_info2               | del_node_info  |   |                                                                           |
| get_graph_viewer             |                |   | input argument changed from 'TForm *' to 'TWidget \*'                     |
| get_node_info2               | get_node_info  |   |                                                                           |
| get_viewer_graph             |                | * |                                                                           |
| grentry                      |                |   | 'grentry' has been converted from a global variable to an inline function |
| refresh_viewer               |                | * |                                                                           |
| set_node_info2               | set_node_info  |   |                                                                           |
| viewer_center_on             |                | * |                                                                           |
| viewer_create_groups         |                | * |                                                                           |
| viewer_del_node_info         |                | * |                                                                           |
| viewer_delete_groups         |                | * |                                                                           |
| viewer_fit_window            |                | * |                                                                           |
| viewer_get_curnode           |                | * |                                                                           |
| viewer_get_node_info         |                | * |                                                                           |
| viewer_set_gli               |                | * | added 'flags' argument                                                    |
| viewer_set_groups_visibility |                | * |                                                                           |
| viewer_set_node_info         |                | * |                                                                           |


### help.h

| original name | new name   |
|---------------|------------|
| askyn         | ask_yn     |
| askyn_v       | vask_yn    |


### ida.hpp

| original name     | new name          |
|-------------------|-------------------|
| ansi2idb          | <**removed**>     |
| dto_copy_from_inf | <**removed**>     |
| dto_copy_to_inf   | <**removed**>     |
| dto_init          | <**removed**>     |
| idb2scr           | <**removed**>     |
| scr2idb           | <**removed**>     |
| showAllComments   | show_all_comments |
| showComments      | show_comments     |
| showRepeatables   | show_repeatables  |
| toEA              | to_ea             |


### idd.hpp

- [1] output argument moved to beginning of argument list

| original name |[1]| Notes                                                                 |
|---------------|:-:|-----------------------------------------------------------------------|
| dbg_appcall   | * | swapped 'argnum' and 'argv'; argument type: 'int' changed to 'size_t' |


### idp.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name                  | new name         |[1]|[2]| Notes                                                                                        |
|--------------------------------|------------------|:-:|:-:|----------------------------------------------------------------------------------------------|
| get_reg_info2                  | get_reg_info     |   |   |                                                                                              |
| get_reg_name                   |                  | q | * |                                                                                              |
| invoke_callbacks               |                  |   |   | moved from loader.hpp                                                                        |
| hook_to_notification_point     |                  |   |   | moved from loader.hpp                                                                        |
| unhook_from_notification_point |                  |   |   | moved from loader.hpp                                                                        |
| set_processor_type             |                  |   |   | return type changed from 'char' to 'bool'; argument type: 'int' changed to 'setproc_level_t' |
| parse_reg_name                 |                  | * |   |                                                                                              |
| cfg_get_cc_header_path         |                  |   |   | moved to config.hpp                                                                          |
| cfg_get_cc_parm                |                  |   |   | moved to config.hpp                                                                          |
| cfg_get_cc_predefined_macros   |                  |   |   | moved to config.hpp                                                                          |
| cfgopt_t__apply                |                  |   |   | moved to config.hpp                                                                          |
| parse_config_value             |                  |   |   | moved to config.hpp                                                                          |
| read_config                    |                  |   |   | moved to config.hpp                                                                          |
| read_config_file               |                  |   |   | moved to config.hpp                                                                          |
| read_config_string             |                  |   |   | moved to config.hpp                                                                          |
| InstrIsSet                     | has_insn_feature |   |   |                                                                                              |
| str2regf                       | <**removed**>    |   |   |                                                                                              |
| create_custom_fixup            | <**removed**>    |   |   |                                                                                              |
| gen_spcdef                     | <**removed**>    |   |   | use 'outctx_t::out_specea' instead                                                           |
| gen_abssym                     | <**removed**>    |   |   | use 'outctx_t::out_specea' instead                                                           |
| gen_comvar                     | <**removed**>    |   |   | use 'outctx_t::out_specea' instead                                                           |
| gen_extern                     | <**removed**>    |   |   | use 'outctx_t::out_specea' instead                                                           |
| intel_data                     | <**removed**>    |   |   | use 'outctx_t::out_data' instead                                                             |
| is_basic_block_end             |                  |   |   | added 'const insn_t &' input argument                                                        |
| is_call_insn                   |                  |   |   | converted input 'ea_t' argument to 'const insn_t &'                                          |
| is_indirect_jump_insn          |                  |   |   | converted input 'ea_t' argument to 'const insn_t &'                                          |
| is_ret_insn                    |                  |   |   | converted input 'ea_t' argument to 'const insn_t &'                                          |


### ieee.h

- [1] output argument moved to beginning of argument list

| original name |[1]|
|---------------|:-:|
| eetol         | * |
| eetol64       | * |
| eetol64u      | * |
| realtoasc     | * |


### ints.hpp (**REMOVED**)

| original name       | new name      | Notes              |
|---------------------|---------------|--------------------|
| get_predef_cmt      | <**removed**> |                    |
| get_vxd_func_name   | <**removed**> |                    |
| get_predef_insn_cmt |               | moved to bytes.hpp |


### kernwin.hpp

NOTE: Please note that in IDA version 6.7 we introduced the *Actions API*, which deprecated many functions related to augmenting functionality in IDA.

Those previously deprecated functions have been removed. For more details about the Actions API, please visit our old blog post from 2014:

<http://www.hexblog.com/?p=886>

NOTE: 'TForm', 'TCustomControl', and 'TWinControl' have been replaced by 'TWidget'

NOTE: global variable 'dirty_infos' has been removed.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] input argument changed from pointer to reference
- [4] return type changed from 'TForm *' to 'TWidget \*'
- [5] input argument changed from 'TCustomControl *' to 'TWidget \*'

| original name                     | new name             |[1]|[2]|[3]|[4]|[5]| Notes                                                                                                      |
|-----------------------------------|----------------------|:-:|:-:|:-:|:-:|:-:|------------------------------------------------------------------------------------------------------------|
| askfile_c                         | <**removed**>        |   |   |   |   |   |                                                                                                            |
| askfile_cv                        | <**removed**>        |   |   |   |   |   |                                                                                                            |
| askstr                            | <**removed**>        |   |   |   |   |   |                                                                                                            |
| close_form                        | <**removed**>        |   |   |   |   |   | use 'form_actions_t::close' instead                                                                        |
| close_tform                       | <**removed**>        |   |   |   |   |   | use 'close_widget' instead                                                                                 |
| create_tform                      | <**removed**>        |   |   |   |   |   | use 'find_widget' or 'create_empty_widget' instead                                                         |
| enable_menu_item                  | <**removed**>        |   |   |   |   |   | superseded by the Actions API (see blog post above)                                                        |
| entab                             | <**removed**>        |   |   |   |   |   |                                                                                                            |
| find_tform                        | <**removed**>        |   |   |   |   |   | use 'find_widget' instead                                                                                  |
| get_current_tform                 | <**removed**>        |   |   |   |   |   | use 'get_current_widget' instead                                                                           |
| get_highlighted_identifier        | <**removed**>        |   |   |   |   |   | use 'get_current_viewer' and 'get_highlight' instead                                                       |
| get_tform_idaview                 | <**removed**>        |   |   |   |   |   | use the 'TWidget *' directly instead of obtaining the IDAView                                              |
| get_tform_title                   | <**removed**>        |   |   |   |   |   | use 'get_widget_title' instead                                                                             |
| get_tform_type                    | <**removed**>        |   |   |   |   |   | use 'get_widget_type' instead                                                                              |
| get_viewer_name                   | <**removed**>        |   |   |   |   |   | use 'get_widget_title' instead                                                                             |
| init_kernel                       | <**removed**>        |   |   |   |   |   |                                                                                                            |
| is_chooser_tform                  | <**removed**>        |   |   |   |   |   | use 'is_chooser_widget' instead                                                                            |
| print_disp                        | <**removed**>        |   |   |   |   |   | use 'append_disp' instead                                                                                  |
| set_menu_item_icon                | <**removed**>        |   |   |   |   |   | superseded by the Actions API (see blog post above)                                                        |
| switchto_tform                    | <**removed**>        |   |   |   |   |   | use 'activate_widget' instead                                                                              |
| term_kernel                       | <**removed**>        |   |   |   |   |   |                                                                                                            |
| umsg                              | <**removed**>        |   |   |   |   |   |                                                                                                            |
| vaskstr                           | <**removed**>        |   |   |   |   |   |                                                                                                            |
| vumsg                             | <**removed**>        |   |   |   |   |   |                                                                                                            |
| <**added**>                       | activate_widget      |   |   |   |   |   |                                                                                                            |
| <**added**>                       | append_disp          |   |   |   |   |   |                                                                                                            |
| <**added**>                       | close_widget         |   |   |   |   |   |                                                                                                            |
| <**added**>                       | create_empty_widget  |   |   |   |   |   |                                                                                                            |
| <**added**>                       | find_widget          |   |   |   |   |   |                                                                                                            |
| <**added**>                       | get_current_widget   |   |   |   |   |   |                                                                                                            |
| <**added**>                       | get_highlight        |   |   |   |   |   |                                                                                                            |
| <**added**>                       | get_widget_title     |   |   |   |   |   |                                                                                                            |
| <**added**>                       | get_widget_type      |   |   |   |   |   |                                                                                                            |
| <**added**>                       | is_buttoncb_t_type   |   |   |   |   |   |                                                                                                            |
| <**added**>                       | is_chooser_widget    |   |   |   |   |   |                                                                                                            |
| <**added**>                       | is_formchgcb_t_type  |   |   |   |   |   |                                                                                                            |
| <**added**>                       | qcleanline           |   |   |   |   |   |                                                                                                            |
| <**added**>                       | set_highlight        |   |   |   |   |   |                                                                                                            |
| <**added**>                       | unpack_ds_to_buf     |   |   |   |   |   |                                                                                                            |
| AskUsingForm_c                    | ask_form             |   |   |   |   |   |                                                                                                            |
| AskUsingForm_cv                   | vask_form            |   |   |   |   |   |                                                                                                            |
| OpenForm_c                        | open_form            |   |   |   | * |   |                                                                                                            |
| OpenForm_cv                       | vopen_form           |   |   |   | * |   |                                                                                                            |
| askaddr                           | ask_addr             |   |   |   |   |   |                                                                                                            |
| askbuttons_c                      | ask_buttons          |   |   |   |   |   |                                                                                                            |
| askbuttons_cv                     | vask_buttons         |   |   |   |   |   |                                                                                                            |
| askfile2_c                        | ask_file             |   |   |   |   |   | 'filters' argument merged into 'format'                                                                    |
| askfile2_cv                       | vask_file            |   |   |   |   |   | 'filters' argument merged into 'format'                                                                    |
| askident                          | ask_ident            |   | * |   |   |   | return type changed from 'char *' to 'bool'                                                                |
| asklong                           | ask_long             |   |   |   |   |   |                                                                                                            |
| askqstr                           | ask_str              |   |   |   |   |   | added 'hist' argument                                                                                      |
| askseg                            | ask_seg              |   |   |   |   |   |                                                                                                            |
| asktext                           | ask_text             | q | * |   |   |   | return type changed from 'char *' to 'bool'                                                                |
| askyn_c                           | ask_yn               |   |   |   |   |   |                                                                                                            |
| askyn_cv                          | vask_yn              |   |   |   |   |   |                                                                                                            |
| atob32                            |                      | * |   |   |   |   |                                                                                                            |
| atob64                            |                      | * |   |   |   |   |                                                                                                            |
| atoea                             |                      | * |   |   |   |   |                                                                                                            |
| atos                              |                      | * |   |   |   |   |                                                                                                            |
| attach_action_to_popup            |                      |   |   |   |   |   | input argument changed from 'TForm *' to 'TWidget \*'                                                      |
| attach_dynamic_action_to_popup    |                      |   | * |   |   |   | input argument changed from 'TForm *' to 'TWidget \*'                                                      |
| b2a32                             |                      | * |   |   |   |   |                                                                                                            |
| b2a64                             |                      | * |   |   |   |   |                                                                                                            |
| back_char                         |                      |   |   |   |   |   | moved to pro.h                                                                                             |
| choose, choose2, choose3          | choose               |   |   |   |   |   | choosers should use the new 'chooser_base_t' interface                                                     |
| choose_srcp                       |                      |   |   |   |   |   | return type changed from 'segreg_area_t *' to 'sreg_range_t \*'                                            |
| choose_til                        |                      |   | * |   |   |   |                                                                                                            |
| clearBreak                        | clr_cancelled        |   |   |   |   |   |                                                                                                            |
| clear_refresh_request             |                      |   |   |   |   |   | to be used instead of 'dirty_infos'                                                                        |
| create_code_viewer                |                      |   |   |   |   |   | return type changed from 'TCustomControl *' to 'TWidget \*'; 'parent' argument made optional and reordered |
| create_custom_viewer              |                      |   |   |   |   |   | return type changed from 'TCustomControl *' to 'TWidget \*'; 'parent' argument made optional and reordered |
| custom_viewer_jump                |                      |   |   |   |   | * |                                                                                                            |
| destroy_custom_viewer             |                      |   |   |   |   | * |                                                                                                            |
| detach_action_from_popup          |                      |   |   |   |   |   | input argument changed from 'TForm *' to 'TWidget \*'                                                      |
| ea2str                            |                      | * |   |   |   |   |                                                                                                            |
| ea_viewer_history_push_and_jump   |                      |   |   |   |   | * |                                                                                                            |
| gen_disasm_text                   |                      | * |   |   |   |   |                                                                                                            |
| get_8bit                          |                      |   |   | * |   |   |                                                                                                            |
| get_action_label                  |                      | * |   |   |   |   |                                                                                                            |
| get_action_shortcut               |                      | * |   |   |   |   |                                                                                                            |
| get_action_tooltip                |                      | * |   |   |   |   |                                                                                                            |
| get_chooser_data                  |                      |   |   |   |   |   | argument type: 'uint32' changed to 'int'                                                                   |
| get_current_viewer                |                      |   |   |   |   |   | return type changed from 'TCustomControl *' to 'TWidget \*'                                                |
| get_custom_viewer_curline         |                      |   |   |   |   | * |                                                                                                            |
| get_custom_viewer_place           |                      |   |   |   |   | * |                                                                                                            |
| get_ea_viewer_history_info        |                      |   |   |   |   | * |                                                                                                            |
| get_kernel_version                |                      |   |   |   |   |   | return type changed from 'bool' to 'ssize_t'                                                               |
| get_output_curline                |                      |   | * |   |   |   |                                                                                                            |
| get_output_selected_text          |                      |   | * |   |   |   |                                                                                                            |
| get_view_renderer_type            |                      |   |   |   |   | * |                                                                                                            |
| get_viewer_place_type             |                      |   |   |   |   | * |                                                                                                            |
| get_viewer_user_data              |                      |   |   |   |   | * |                                                                                                            |
| is_idaview                        |                      |   |   |   |   | * |                                                                                                            |
| is_refresh_requested              |                      |   |   |   |   |   | to be used instead of 'dirty_infos'                                                                        |
| jumpto                            |                      |   |   |   |   | * |                                                                                                            |
| linearray_t::down                 |                      |   |   |   |   |   | return type changed from 'char *' to 'const qstring \*'                                                    |
| linearray_t::up                   |                      |   |   |   |   |   | return type changed from 'char *' to 'const qstring \*'                                                    |
| open_bpts_window                  |                      |   |   |   |   | * |                                                                                                            |
| open_bpts_window                  |                      |   |   |   | * |   |                                                                                                            |
| open_calls_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_disasm_window                |                      |   |   |   | * |   | input argument changed from 'const areavec_t *' to 'const rangevec_t \*'                                   |
| open_enums_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_exports_window               |                      |   |   |   | * |   |                                                                                                            |
| open_frame_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_funcs_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_hexdump_window               |                      |   |   |   | * |   |                                                                                                            |
| open_imports_window               |                      |   |   |   | * |   |                                                                                                            |
| open_loctypes_window              |                      |   |   |   | * |   |                                                                                                            |
| open_modules_window               |                      |   |   |   | * |   |                                                                                                            |
| open_names_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_navband_window               |                      |   |   |   | * |   |                                                                                                            |
| open_notepad_window               |                      |   |   |   | * |   |                                                                                                            |
| open_problems_window              |                      |   |   |   | * |   |                                                                                                            |
| open_segments_window              |                      |   |   |   | * |   |                                                                                                            |
| open_segregs_window               |                      |   |   |   | * |   |                                                                                                            |
| open_selectors_window             |                      |   |   |   | * |   |                                                                                                            |
| open_signatures_window            |                      |   |   |   | * |   |                                                                                                            |
| open_stack_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_strings_window               |                      |   |   |   | * |   |                                                                                                            |
| open_structs_window               |                      |   |   |   | * |   |                                                                                                            |
| open_tform                        | display_widget       |   |   |   |   |   | input argument changed from 'TForm *' to 'TWidget \*'                                                      |
| open_threads_window               |                      |   |   |   | * |   |                                                                                                            |
| open_tils_window                  |                      |   |   |   | * |   |                                                                                                            |
| open_trace_window                 |                      |   |   |   | * |   |                                                                                                            |
| open_xrefs_window                 |                      |   |   |   | * |   |                                                                                                            |
| qstr2user                         |                      |   |   |   |   |   | moved to pro.h                                                                                             |
| r50_to_asc                        |                      | * |   |   |   |   |                                                                                                            |
| read_range_selection              | read_selection       |   |   |   |   | * | WARNING: 'read_selection' has changed meaning                                                              |
| read_selection                    | read_range_selection |   |   |   |   |   | WARNING: 'read_selection' has changed meaning; added 'TWidget *' argument                                  |
| refresh_custom_viewer             |                      |   |   |   |   | * |                                                                                                            |
| repaint_custom_viewer             |                      |   |   |   |   | * |                                                                                                            |
| request_refresh                   |                      |   |   |   |   |   | added 'cnd' argument                                                                                       |
| setBreak                          | set_cancelled        |   |   |   |   |   |                                                                                                            |
| set_code_viewer_handler           |                      |   |   |   |   | * |                                                                                                            |
| set_code_viewer_is_source         |                      |   |   |   |   | * |                                                                                                            |
| set_code_viewer_line_handlers     |                      |   |   |   |   | * |                                                                                                            |
| set_code_viewer_lines_alignment   |                      |   |   |   |   | * |                                                                                                            |
| set_code_viewer_lines_icon_margin |                      |   |   |   |   | * |                                                                                                            |
| set_code_viewer_lines_radix       |                      |   |   |   |   | * |                                                                                                            |
| set_code_viewer_user_data         |                      |   |   |   |   | * |                                                                                                            |
| set_custom_viewer_handler         |                      |   |   |   |   | * |                                                                                                            |
| set_custom_viewer_handlers        |                      |   |   |   |   | * |                                                                                                            |
| set_custom_viewer_qt_aware        |                      |   |   |   |   | * |                                                                                                            |
| set_custom_viewer_range           |                      |   |   |   |   | * |                                                                                                            |
| set_view_renderer_type            |                      |   |   |   |   | * |                                                                                                            |
| show_hex_file                     |                      |   |   |   |   |   | argument type: 'int32' changed to 'int64'                                                                  |
| skipSpaces                        | skip_spaces          |   |   |   |   |   |                                                                                                            |
| stoa                              |                      | q | * |   |   |   |                                                                                                            |
| str2ea                            |                      | * |   |   |   |   |                                                                                                            |
| str2ea_ex                         |                      | * |   |   |   |   |                                                                                                            |
| str2user                          |                      |   |   |   |   |   | moved to pro.h                                                                                             |
| ui_load_new_file                  |                      |   |   |   |   |   | added 'temp_file' and 'ploaders'; input argument 'filename' changed from 'const char *' to 'qstring \*'    |
| user2qstr                         |                      |   |   |   |   |   | moved to pro.h                                                                                             |
| user2str                          |                      |   |   |   |   |   | moved to pro.h                                                                                             |
| vaskqstr                          | vask_str             |   |   |   |   |   | added 'hist' argument                                                                                      |
| vasktext                          | vask_text            | q | * |   |   |   | return type changed from 'char *' to 'bool'                                                                |
| vshow_hex_file                    |                      |   |   |   |   |   | argument type: 'int32' changed to 'int64'                                                                  |
| wasBreak                          | user_cancelled       |   |   |   |   |   |                                                                                                            |


### lex.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name | new name           |[1]|[2]|
|---------------|--------------------|:-:|:-:|
| lex_define    | lex_define_macro   |   |   |
| lex_undef     | lex_undefine_macro |   |   |
| lxascii       | lex_print_token    | q | * |
| lxget         | lex_get_token      |   |   |
| lxgetserr     | lex_get_file_line  |   |   |
| lxgetsini     | lex_init_file      |   |   |
| lxgetstrm     | lex_term_file      |   |   |
| lxini         | lex_init_string    |   |   |


### lines.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] return type changed from 'void' to 'bool'

| original name          | new name        |[1]|[2]|[3]| Notes                                                             |
|------------------------|-----------------|:-:|:-:|:-:|-------------------------------------------------------------------|
| MakeBorder             | <**removed**>   |   |   |   | use 'outctx_base_t::gen_border_line(false)' instead               |
| MakeLine               | <**removed**>   |   |   |   | use 'outctx_base_t::flush_buf' instead                            |
| MakeNull               | <**removed**>   |   |   |   | use 'outctx_base_t::gen_empty_line' instead                       |
| MakeSolidBorder        | <**removed**>   |   |   |   | use 'outctx_base_t::gen_border_line(true)' instead                |
| add_long_cmt_v         | <**removed**>   |   |   |   | use 'vadd_extra_line' instead                                     |
| close_comment          | <**removed**>   |   |   |   | use 'outctx_base_t::close_comment' instead                        |
| describex              | <**removed**>   |   |   |   | use 'vadd_extra_line' instead                                     |
| finish_makeline        | <**removed**>   |   |   |   | use 'outctx_base_t::term_outctx' instead                          |
| gen_cmt_line           | <**removed**>   |   |   |   | use 'outctx_base_t::gen_cmt_line' instead                         |
| gen_cmt_line_v         | <**removed**>   |   |   |   | use 'outctx_base_t::gen_cmt_line_v' instead                       |
| gen_collapsed_line     | <**removed**>   |   |   |   | use 'outctx_base_t::gen_collapsed_line' instead                   |
| gen_colored_cmt_line_v | <**removed**>   |   |   |   | use 'outctx_base_t::gen_colored_cmt_line_v' instead               |
| generate_big_comment   | <**removed**>   |   |   |   | use 'outctx_base_t::gen_block_cmt' instead                        |
| generate_many_lines    | <**removed**>   |   |   |   | use 'outctx_base_t::gen_many_lines(-1, NULL, [...])' instead      |
| init_lines             | <**removed**>   |   |   |   |                                                                   |
| init_lines_array       | <**removed**>   |   |   |   | use 'outctx_base_t::init_lines_array' instead                     |
| printf_line            | <**removed**>   |   |   |   | use 'outctx_base_t::gen_printf' instead                           |
| printf_line_v          | <**removed**>   |   |   |   | use 'outctx_base_t::gen_vprintf' instead                          |
| save_line_in_array     | <**removed**>   |   |   |   | use 'outctx_base_t::save_buf' instead                             |
| save_lines             | <**removed**>   |   |   |   |                                                                   |
| save_sourcefiles       | <**removed**>   |   |   |   |                                                                   |
| setup_makeline         | <**removed**>   |   |   |   | use 'outctx_base_t::setup_outctx' instead                         |
| tag_addchr             | <**removed**>   |   |   |   |                                                                   |
| tag_addstr             | <**removed**>   |   |   |   |                                                                   |
| tag_off                | <**removed**>   |   |   |   |                                                                   |
| tag_on                 | <**removed**>   |   |   |   |                                                                   |
| <**added**>            | get_last_pfxlen |   |   |   |                                                                   |
| <**added**>            | vadd_extra_line |   |   |   |                                                                   |
| add_long_cmt           | add_extra_cmt   |   |   |   |                                                                   |
| add_pgm_cmt            |                 |   |   | * |                                                                   |
| describe               | add_extra_line  |   |   | * |                                                                   |
| generate_disasm_line   |                 | q | * |   |                                                                   |
| generate_disassembly   |                 | q | * |   | output argument is 'qstrvec_t'                                    |
| get_extra_cmt          |                 | q | * |   |                                                                   |
| get_sourcefile         |                 |   |   |   | argument type: 'area_t *' changed to 'range_t \*'                 |
| tag_addr               |                 | q | * |   | return type changed from 'char *' to 'void'; added 'ins' argument |
| tag_remove             |                 | q | * |   | added 'init_level' argument                                       |


### llong.hpp

- [1] output argument moved to beginning of argument list
- [2] output argument changed from reference to pointer

| original name | new name      |[1]|[2]|
|---------------|---------------|:-:|:-:|
| print         | <**removed**> |   |   |
| llong_div     |               | * | * |
| llong_udiv    |               | * | * |


### loader.hpp

NOTE: global variables 'database_flags', 'command_line_file', 'idb_path', and 'id0_path' have been removed.

NOTE: class 'loader_jump' has been renamed to 'loader_failure_t'

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] input argument changed from 'int32' to 'qoff64_t'
- [3] return type changed from 'int32' to 'qoff64_t'

| original name                  | new name         |[1]|[2]|[3]| Notes                                                           |
|--------------------------------|------------------|:-:|:-:|:-:|-----------------------------------------------------------------|
| enum_plugins                   | <**removed**>    |   |   |   |                                                                 |
| init_loader_options            | <**removed**>    |   |   |   |                                                                 |
| <**added**>                    | find_plugin      |   |   |   |                                                                 |
| <**added**>                    | process_archive  |   |   |   |                                                                 |
| accept_file                    |                  | q | * |   | added 'processor' output argument (optional)                    |
| base2file                      |                  |   | * |   |                                                                 |
| build_loaders_list             |                  |   |   |   | added 'filename' argument (name of the input file for archives) |
| clr_database_flag              |                  |   |   |   | to be used instead of 'database_flags'                          |
| extract_module_from_archive    |                  | * |   |   |                                                                 |
| file2base                      |                  |   | * |   |                                                                 |
| get_fileregion_ea              |                  |   | * |   |                                                                 |
| get_fileregion_offset          |                  |   |   | * |                                                                 |
| get_path                       |                  |   |   |   | to be used instead of 'idb_path'                                |
| hook_to_notification_point     |                  |   |   |   | moved to idp.hpp                                                |
| invoke_callbacks               |                  |   |   |   | moved to idp.hpp                                                |
| is_database_flag               |                  |   |   |   | to be used instead of 'database_flags'                          |
| load_and_run_plugin            |                  |   |   |   | argument type: 'int' changed to 'size_t'                        |
| load_binary_file               |                  |   | * |   | argument type: 'uint32' changed to 'uint64'                     |
| load_dll_or_say                | load_core_module | * |   |   | added 'entry' argument (name of plugin 'entrypoint' symbol)     |
| mem2base                       |                  |   | * |   |                                                                 |
| run_plugin                     |                  |   |   |   | argument type: 'int' changed to 'size_t'                        |
| save_database_ex               | save_database    |   |   |   |                                                                 |
| set_database_flag              |                  |   |   |   | to be used instead of 'database_flags'                          |
| set_path                       |                  |   |   |   | to be used instead of 'idb_path'                                |
| unhook_from_notification_point |                  |   |   |   | moved to idp.hpp                                                |


### moves.hpp

NOTE: 'curloc_t' and 'location_t' have been replaced by 'lochist_t'.

| original name              | new name                           |
|----------------------------|------------------------------------|
| curloc_get                 | <**removed**>                      |
| curloc_get_entry           | <**removed**>                      |
| curloc_hide_if_necessary   | <**removed**>                      |
| curloc_jump                | <**removed**>                      |
| curloc_jump_push           | <**removed**>                      |
| curloc_linkTo              | <**removed**>                      |
| curloc_mark                | <**removed**>                      |
| curloc_markdesc            | <**removed**>                      |
| curloc_markedpos           | <**removed**>                      |
| curloc_pop                 | <**removed**>                      |
| curloc_unhide_if_necessary | <**removed**>                      |
| location_get               | <**removed**>                      |
| location_get_entry         | <**removed**>                      |
| location_jump              | <**removed**>                      |
| location_linkTo            | <**removed**>                      |
| location_mark              | <**removed**>                      |
| location_pop               | <**removed**>                      |
| location_push_and_jump     | <**removed**>                      |
| <**added**>                | graph_location_info_t::deserialize |
| <**added**>                | graph_location_info_t::serialize   |
| <**added**>                | renderer_info_pos_t::deserialize   |
| <**added**>                | renderer_info_pos_t::serialize     |


### nalt.hpp

NOTE: global variable 'import_node' has been removed.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name            | new name                   |[1]|[2]| Notes                                                                                  |
|--------------------------|----------------------------|:-:|:-:|----------------------------------------------------------------------------------------|
| _del_item_color          | <**removed**>              |   |   |                                                                                        |
| _del_strid               | <**removed**>              |   |   |                                                                                        |
| _set_item_color          | <**removed**>              |   |   |                                                                                        |
| _set_item_color          | <**removed**>              |   |   |                                                                                        |
| _set_strid               | <**removed**>              |   |   |                                                                                        |
| del__segtrans            | <**removed**>              |   |   |                                                                                        |
| del_enum_id0             | <**removed**>              |   |   |                                                                                        |
| del_enum_id1             | <**removed**>              |   |   |                                                                                        |
| del_fop1                 | <**removed**>              |   |   |                                                                                        |
| del_fop2                 | <**removed**>              |   |   |                                                                                        |
| del_fop3                 | <**removed**>              |   |   |                                                                                        |
| del_fop4                 | <**removed**>              |   |   |                                                                                        |
| del_fop5                 | <**removed**>              |   |   |                                                                                        |
| del_fop6                 | <**removed**>              |   |   |                                                                                        |
| del_graph_groups0        | <**removed**>              |   |   |                                                                                        |
| del_jumptable_info       | <**removed**>              |   |   |                                                                                        |
| del_linnum0              | <**removed**>              |   |   |                                                                                        |
| del_manual_insn0         | <**removed**>              |   |   |                                                                                        |
| del_nalt_cmt             | <**removed**>              |   |   |                                                                                        |
| del_nalt_rptcmt          | <**removed**>              |   |   |                                                                                        |
| del_stroff0              | <**removed**>              |   |   |                                                                                        |
| del_stroff1              | <**removed**>              |   |   |                                                                                        |
| del_wide_value           | <**removed**>              |   |   |                                                                                        |
| get__segtrans            | <**removed**>              |   |   |                                                                                        |
| get_auto_plugins         | <**removed**>              |   |   |                                                                                        |
| get_custom_refinfos      | <**removed**>              |   |   | use 'get_refinfo_descs' instead                                                        |
| get_enum_id0             | <**removed**>              |   |   |                                                                                        |
| get_enum_id1             | <**removed**>              |   |   |                                                                                        |
| get_fop1                 | <**removed**>              |   |   |                                                                                        |
| get_fop2                 | <**removed**>              |   |   |                                                                                        |
| get_fop3                 | <**removed**>              |   |   |                                                                                        |
| get_fop4                 | <**removed**>              |   |   |                                                                                        |
| get_fop5                 | <**removed**>              |   |   |                                                                                        |
| get_fop6                 | <**removed**>              |   |   |                                                                                        |
| get_graph_groups0        | <**removed**>              |   |   |                                                                                        |
| get_jumptable_info       | <**removed**>              |   |   |                                                                                        |
| get_linnum0              | <**removed**>              |   |   |                                                                                        |
| get_manual_insn0         | <**removed**>              |   |   |                                                                                        |
| get_nalt_cmt             | <**removed**>              |   |   |                                                                                        |
| get_nalt_rptcmt          | <**removed**>              |   |   |                                                                                        |
| get_stroff0              | <**removed**>              |   |   |                                                                                        |
| get_stroff1              | <**removed**>              |   |   |                                                                                        |
| get_wide_value           | <**removed**>              |   |   |                                                                                        |
| is_unicode               | <**removed**>              |   |   | use 'get_strtype_bpu' instead                                                          |
| set__segtrans            | <**removed**>              |   |   |                                                                                        |
| set_auto_plugins         | <**removed**>              |   |   |                                                                                        |
| set_enum_id0             | <**removed**>              |   |   |                                                                                        |
| set_enum_id1             | <**removed**>              |   |   |                                                                                        |
| set_fop1                 | <**removed**>              |   |   |                                                                                        |
| set_fop2                 | <**removed**>              |   |   |                                                                                        |
| set_fop3                 | <**removed**>              |   |   |                                                                                        |
| set_fop4                 | <**removed**>              |   |   |                                                                                        |
| set_fop5                 | <**removed**>              |   |   |                                                                                        |
| set_fop6                 | <**removed**>              |   |   |                                                                                        |
| set_graph_groups0        | <**removed**>              |   |   |                                                                                        |
| set_jumptable_info       | <**removed**>              |   |   |                                                                                        |
| set_linnum0              | <**removed**>              |   |   |                                                                                        |
| set_manual_insn0         | <**removed**>              |   |   |                                                                                        |
| set_nalt_cmt             | <**removed**>              |   |   |                                                                                        |
| set_nalt_rptcmt          | <**removed**>              |   |   |                                                                                        |
| set_stroff0              | <**removed**>              |   |   |                                                                                        |
| set_stroff1              | <**removed**>              |   |   |                                                                                        |
| set_wide_value           | <**removed**>              |   |   |                                                                                        |
| <**added**>              | clr_notproc                |   |   |                                                                                        |
| <**added**>              | delete_imports             |   |   | to be used instead of 'auto_display'                                                   |
| <**added**>              | ea2node                    |   |   |                                                                                        |
| <**added**>              | find_custom_refinfo        |   |   |                                                                                        |
| <**added**>              | get_abi_name               |   |   |                                                                                        |
| <**added**>              | get_archive_path           |   |   |                                                                                        |
| <**added**>              | get_custom_refinfo         |   |   |                                                                                        |
| <**added**>              | get_custom_refinfo_handler |   |   |                                                                                        |
| <**added**>              | get_encoding_bpu           |   |   |                                                                                        |
| <**added**>              | get_gotea                  |   |   |                                                                                        |
| <**added**>              | get_refinfo_descs          |   |   |                                                                                        |
| <**added**>              | get_strtype_bpu            |   |   |                                                                                        |
| <**added**>              | getnode                    |   |   |                                                                                        |
| <**added**>              | is_notproc                 |   |   |                                                                                        |
| <**added**>              | is_reftype_target_optional |   |   |                                                                                        |
| <**added**>              | node2ea                    |   |   |                                                                                        |
| <**added**>              | set_archive_path           |   |   |                                                                                        |
| <**added**>              | set_gotea                  |   |   |                                                                                        |
| <**added**>              | set_notproc                |   |   |                                                                                        |
| change_encoding_name     | rename_encoding            |   |   |                                                                                        |
| del_switch_info_ex       | del_switch_info            |   |   |                                                                                        |
| del_tinfo2               | del_tinfo                  |   |   |                                                                                        |
| del_tinfo2(,n)           | del_op_tinfo               |   |   |                                                                                        |
| get_array_parameters     |                            | * |   | removed 'bufsize' argument                                                             |
| get_asm_inc_file         |                            |   | * |                                                                                        |
| get_custom_data_type_ids |                            | * |   | removed 'bufsize' argument                                                             |
| get_default_encoding_idx |                            |   |   | argument type: 'int32' changed to 'int'                                                |
| get_encodings_count      | get_encoding_qty           |   |   |                                                                                        |
| get_import_module_name   |                            | q | * |                                                                                        |
| get_op_tinfo2            | get_op_tinfo               | * |   |                                                                                        |
| get_refinfo              |                            | * |   |                                                                                        |
| get_str_type_code        |                            |   |   | return type changed from 'char' to 'uchar'; argument type: 'uval_t' changed to 'int32' |
| get_strid                |                            |   |   | return type changed from 'ea_t' to 'tid_t'                                             |
| get_switch_info_ex       | get_switch_info            | * |   | removed 'bufsize' argument                                                             |
| get_tinfo2               | get_tinfo                  | * |   |                                                                                        |
| get_xrefpos              |                            | * |   | removed 'bufsize' argument                                                             |
| read_struc_path          |                            | * |   | argument type: 'netnode' changed to 'ea_t'                                             |
| set_default_encoding_idx |                            |   |   | argument type: 'int32' changed to 'int'                                                |
| set_op_tinfo2            | set_op_tinfo               |   |   |                                                                                        |
| set_switch_info_ex       | set_switch_info            |   |   | input argument changed from 'const switch_info_ex_t *' to 'const switch_info_t &'      |
| set_tinfo2               | set_tinfo                  |   |   |                                                                                        |
| write_struc_path         |                            |   |   | argument type: 'netnode' changed to 'ea_t'                                             |


### name.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] output argument changed from reference to pointer

| original name         | new name             |[1]|[2]|[3]| Notes                                      |
|-----------------------|----------------------|:-:|:-:|:-:|--------------------------------------------|
| gen_name_decl         | <**removed**>        |   |   |   | use 'outctx_base_t::gen_name_decl' instead |
| <**added**>           | is_strlit_cp         |   |   |   |                                            |
| <**added**>           | is_valid_cp          |   |   |   |                                            |
| <**added**>           | set_cp_validity      |   |   |   |                                            |
| append_struct_fields2 | append_struct_fields | * |   |   |                                            |
| demangle_name2        | demangle_name        |   |   |   |                                            |
| do_name_anyway        | force_name           |   |   |   | removed 'maxlen' argument                  |
| extract_name2         | extract_name         |   |   |   |                                            |
| get_debug_name2       | get_debug_name       |   |   |   |                                            |
| get_debug_names       |                      | * |   | * |                                            |
| get_ea_name           |                      |   |   |   | removed const from 'gtni' argument         |
| get_name_expr         |                      | q | * |   |                                            |
| get_name_value        |                      | * |   |   |                                            |
| get_nice_colored_name |                      | q | * |   |                                            |
| get_struct_operand    |                      | * |   |   |                                            |
| get_true_name         | get_name             |   |   |   |                                            |
| is_ident_char         | is_ident_cp          |   |   |   |                                            |
| is_visible_char       | is_visible_cp        |   |   |   |                                            |
| isident               | is_ident             |   |   |   |                                            |
| validate_name3        | validate_name        |   |   |   | added 'type' and 'flags' arguments         |


### netnode.hpp

| original name         | new name                | Notes                                                          |
|-----------------------|-------------------------|----------------------------------------------------------------|
| <**added**>           | netnode::altdel_ea      | to be used instead of 'netnode::altdel' for addresses (ea_t)   |
| <**added**>           | netnode::altset_ea      | to be used instead of 'netnode::altset' for addresses (ea_t)   |
| <**added**>           | netnode::altval_ea      | to be used instead of 'netnode::altval' for addresses (ea_t)   |
| <**added**>           | netnode::blobsize_ea    | to be used instead of 'netnode::blobsize' for addresses (ea_t) |
| <**added**>           | netnode::chardel_ea     | to be used instead of 'netnode::chardel' for addresses (ea_t)  |
| <**added**>           | netnode::charset_ea     | to be used instead of 'netnode::charset' for addresses (ea_t)  |
| <**added**>           | netnode::charval_ea     | to be used instead of 'netnode::charval' for addresses (ea_t)  |
| <**added**>           | netnode::delblob_ea     | to be used instead of 'netnode::delblob' for addresses (ea_t)  |
| <**added**>           | netnode::eadel          |                                                                |
| <**added**>           | netnode::eadel_idx8     |                                                                |
| <**added**>           | netnode::eaget          |                                                                |
| <**added**>           | netnode::eaget_idx8     |                                                                |
| <**added**>           | netnode::easet          |                                                                |
| <**added**>           | netnode::easet_idx8     |                                                                |
| <**added**>           | netnode::getblob_ea     | to be used instead of 'netnode::getblob' for addresses (ea_t)  |
| <**added**>           | netnode::setblob_ea     | to be used instead of 'netnode::setblob' for addresses (ea_t)  |
| <**added**>           | netnode::supdel_ea      | to be used instead of 'netnode::supdel' for addresses (ea_t)   |
| <**added**>           | netnode::supset_ea      | to be used instead of 'netnode::supset' for addresses (ea_t)   |
| <**added**>           | netnode::supstr_ea      | to be used instead of 'netnode::supstr' for addresses (ea_t)   |
| <**added**>           | netnode::supval_ea      | to be used instead of 'netnode::supval' for addresses (ea_t)   |
| netnode::alt1st       | netnode::altfirst       |                                                                |
| netnode::alt1st_idx8  | netnode::altfirst_idx8  |                                                                |
| netnode::altnxt       | netnode::altnext        |                                                                |
| netnode::char1st      | netnode::charfirst      |                                                                |
| netnode::char1st_idx8 | netnode::charfirst_idx8 |                                                                |
| netnode::charnxt      | netnode::charnext       |                                                                |
| netnode::getblob      |                         | added variants that work with 'qvector<T> *' and 'qstring \*'  |
| netnode::hash1st      | netnode::hashfirst      | added variant that works with 'qstring *'                      |
| netnode::hashlast     |                         | added variant that works with 'qstring *'                      |
| netnode::hashnxt      | netnode::hashnext       | added variant that works with 'qstring *'                      |
| netnode::hashprev     |                         | added variant that works with 'qstring *'                      |
| netnode::hashstr      |                         | added variant that works with 'qstring *'                      |
| netnode::sup1st       | netnode::supfirst       |                                                                |
| netnode::sup1st_idx8  | netnode::supfirst_idx8  |                                                                |
| netnode::supnxt       | netnode::supnext        |                                                                |
| netnode::supstr       |                         | added variant that works with 'qstring *'                      |
| netnode::valstr       |                         | added variant that works with 'qstring *'                      |


### offset.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] input argument 'refinfo_t &' made const

| original name            | new name            |[1]|[2]|[3]| Notes                                                |
|--------------------------|---------------------|:-:|:-:|:-:|------------------------------------------------------|
| calc_reference_basevalue | <**removed**>       |   |   |   | use 'calc_reference_data' instead                    |
| calc_reference_target    | <**removed**>       |   |   |   | use 'calc_reference_data' instead                    |
| set_offset               | <**removed**>       |   |   |   | use 'calc_offset_base' and 'op_plain_offset' instead |
| <**added**>              | add_refinfo_dref    |   |   |   |                                                      |
| <**added**>              | calc_basevalue      |   |   |   |                                                      |
| <**added**>              | calc_offset_base    |   |   |   |                                                      |
| <**added**>              | calc_reference_data |   |   |   |                                                      |
| <**added**>              | op_plain_offset     |   |   |   |                                                      |
| get_offset_expr          |                     | q | * | * |                                                      |
| get_offset_expression    |                     | q | * |   |                                                      |


### problems.h (**RENAMED** from queue.hpp)

NOTE: 'qtype_t' has been changed to 'problist_id_t'.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name        | new name           |[1]|[2]| Notes                                        |
|----------------------|--------------------|:-:|:-:|----------------------------------------------|
| QueueGet             | <**removed**>      |   |   |                                              |
| get_long_queue_name  | <**removed**>      |   |   | Use 'get_problem_name(type, true);' instead  |
| get_short_queue_name | <**removed**>      |   |   | Use 'get_problem_name(type, false);' instead |
| mark_ida_decision    | <**removed**>      |   |   |                                              |
| unmark_ida_decision  | <**removed**>      |   |   |                                              |
| <**added**>          | get_problem_name   |   |   |                                              |
| QueueDel             | forget_problem     |   |   | return type changed from 'void' to 'bool'    |
| QueueGetMessage      | get_problem_desc   | q | * |                                              |
| QueueGetType         | get_problem        |   |   |                                              |
| QueueIsPresent       | is_problem_present |   |   |                                              |
| QueueSet             | remember_problem   |   |   |                                              |


### prodir.h

| original name | new name   |
|---------------|------------|
| qfindclose64  | qfindclose |
| qfindfirst64  | qfindfirst |
| qfindnext64   | qfindnext  |


### pro.h

NOTE: global variables 'codepage' and 'oemcodepage' have been removed.

- [1] output argument moved to beginning of argument list

| original name         | new name              |[1]| Notes                                                                |
|-----------------------|-----------------------|:-:|----------------------------------------------------------------------|
| c2ustr                | <**removed**>         |   | use 'utf8_utf16' instead                                             |
| char2oem              | <**removed**>         |   |                                                                      |
| convert_codepage      | <**removed**>         |   |                                                                      |
| create_hit_counter    | <**removed**>         |   |                                                                      |
| expand_argv           | <**removed**>         |   |                                                                      |
| get_codepages         | <**removed**>         |   |                                                                      |
| hit_counter_timer     | <**removed**>         |   |                                                                      |
| oem2char              | <**removed**>         |   |                                                                      |
| reg_hit_counter       | <**removed**>         |   |                                                                      |
| u2cstr                | <**removed**>         |   | use 'utf16_utf8' instead                                             |
| win_utf2idb           | <**removed**>         |   |                                                                      |
| <**added**>           | acp_utf8              |   |                                                                      |
| <**added**>           | change_codepage       |   |                                                                      |
| <**added**>           | idb_utf8              |   |                                                                      |
| <**added**>           | is_valid_utf8         |   |                                                                      |
| <**added**>           | put_utf8_char         |   |                                                                      |
| <**added**>           | qchdir                |   |                                                                      |
| <**added**>           | qustrlen              |   |                                                                      |
| <**added**>           | scr_utf8              |   |                                                                      |
| <**added**>           | skip_utf8             |   |                                                                      |
| <**added**>           | utf8_scr              |   |                                                                      |
| <**added**>           | utf8_wchar16          |   |                                                                      |
| <**added**>           | utf8_wchar32          |   |                                                                      |
| back_char             |                       |   | moved from kernwin.hpp                                               |
| convert_encoding      |                       | * | return type changed from 'int' to 'ssize_t'                          |
| get_nsec_stamp        |                       |   | output argument changed from 'uint64 *' to the 'uint64' return value |
| parse_command_line3   | parse_command_line    | * |                                                                      |
| qchsize64             | qchsize               |   |                                                                      |
| qfileexist64          | qfileexist            |   |                                                                      |
| qfilesize64           | qfilesize             |   |                                                                      |
| qfstat64              | qfstat                |   |                                                                      |
| qseek64               | qseek                 |   |                                                                      |
| qstat64               | qstat                 |   |                                                                      |
| qstr2user             |                       |   | moved from kernwin.hpp; added 'nsyms' argument                       |
| qtell64               | qtell                 |   |                                                                      |
| qwait                 |                       | * |                                                                      |
| qwait_for_handles     |                       | * |                                                                      |
| qwait_timed           |                       | * |                                                                      |
| search_path           |                       | * |                                                                      |
| str2user              |                       |   | moved from kernwin.hpp                                               |
| unicode_utf8          | utf16_utf8            |   |                                                                      |
| user2qstr             |                       |   | moved from kernwin.hpp                                               |
| user2str              |                       |   | moved from kernwin.hpp                                               |
| utf8_unicode          | utf8_utf16            |   |                                                                      |


### pronet.h

| original name | new name   |
|---------------|------------|
| <**added**>   | qhost2addr |


### range.h (**RENAMED** from area.hpp)

NOTE: some classes have been renamed:
- 'area_t' has been renamed to 'range_t'
- 'areavec_t' has been renamed to 'rangevec_t'
- 'areaset_t' has been renamed to 'rangeset_t'

NOTE: the classes 'rangecb_t', 'ranges_cache_t', and 'lock_range' have been removed


### registry.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name    |[1]|[2]| Notes                              |
|------------------|:-:|:-:|------------------------------------|
| reg_read_strlist | * |   |                                    |
| reg_read_string  | q | * | removed variant with default value |


### search.hpp

- [1] output argument moved to beginning of argument list

| original name | new name    |[1]| Notes                                       |
|---------------|-------------|:-:|---------------------------------------------|
| user2bin      |             | * |                                             |
| find_imm      |             |   | argument type: 'sval_t' changed to 'uval_t' |
| find_void     | find_suspop |   |                                             |


### segment.hpp

NOTE: global variables 'hidden_ranges', 'funcs', and 'segs' have been removed.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] added 'flags' argument

| original name            |                       |[1]|[2]|[3]| Notes                                                                               |
|--------------------------|-----------------------|:-:|:-:|:-:|-------------------------------------------------------------------------------------|
| del_segment_cmt          | <**removed**>         |   |   |   | use 'set_range_cmt("")' instead                                                     |
| vset_segm_name           | <**removed**>         |   |   |   |                                                                                     |
| <**added**>              | get_segm_num          |   |   |   | to be used instead of 'segs.get_range_num()'                                        |
| <**added**>              | lock_segm             |   |   |   | to be used instead of 'rangecb_t_unlock_range(&segs)'                               |
| add_segm                 |                       |   |   | * |                                                                                     |
| ask_selector             | sel2para              |   |   |   |                                                                                     |
| correct_address          |                       |   |   |   | added 'skip_check' argument                                                         |
| del_segment_translations |                       |   |   |   | return type changed from 'bool' to 'void'                                           |
| get_segm_class           |                       | q | * |   |                                                                                     |
| get_segm_name            | get_visible_segm_name |   |   |   | removed variant with 'ea_t' argument                                                |
| get_segment_cmt          |                       | q | * |   | return type changed from 'char *' to 'ssize_t'; added 'repeatable' argument         |
| get_segment_translations |                       |   |   |   | return type changed from 'ea_t *' to 'ssize_t'; output argument converted 'eavec_t' |
| get_true_segm_name       | get_segm_name         | q | * | * |                                                                                     |
| getn_selector            |                       | * |   |   |                                                                                     |
| set_segm_class           |                       |   |   | * |                                                                                     |
| set_segm_name            |                       |   |   | * | arguments converted from printf-style to simple 'const char *'                      |
| set_segment_cmt          |                       |   |   |   | input argument 'segment_t *' made const                                             |
| set_segment_translations |                       |   |   |   | input argument converted to 'const eavec_t &'                                       |
| std_gen_segm_footer      | std_out_segm_footer   |   |   |   | converted to outctx_t; input argument changed to segment_t*                         |


### segregs.hpp (**RENAMED** from srarea.hpp)

NOTE: type 'segreg_area_t' has been renamed to 'sreg_range_t'

| original name            | new name               | Notes                                                    |
|--------------------------|------------------------|----------------------------------------------------------|
| copy_srareas             | copy_sreg_ranges       |                                                          |
| del_srarea               | del_sreg_range         | WARNING: argument order has swapped                      |
| get_prev_srarea          | get_prev_sreg_range    | argument type: 'segreg_area_t' changed to 'sreg_range_t' |
| get_segreg               | get_sreg               |                                                          |
| get_srarea2              | get_sreg_range         | argument type: 'segreg_area_t' changed to 'sreg_range_t' |
| get_srarea_num           | get_sreg_range_num     | WARNING: argument order has swapped                      |
| get_srareas_qty2         | get_sreg_ranges_qty    |                                                          |
| getn_srarea2             | getn_sreg_range        | argument type: 'segreg_area_t' changed to 'sreg_range_t' |
| set_default_segreg_value | set_default_sreg_value |                                                          |
| split_srarea             | split_sreg_range       |                                                          |


### sistack.h (**REMOVED**)

| original name   | new name      |
|-----------------|---------------|
| sistack_t_size  | <**removed**> |
| sistack_t_flush | <**removed**> |


### strlist.hpp

- [1] output argument moved to beginning of argument list

| original name       | new name            |[1]| Notes                                    |
|---------------------|---------------------|:-:|------------------------------------------|
| refresh_strlist     | <**removed**>       |   |                                          |
| set_strlist_options | <**removed**>       |   |                                          |
| <**added**>         | build_strlist       |   |                                          |
| <**added**>         | clear_strlist       |   |                                          |
| <**added**>         | get_strlist_options |   |                                          |
| get_strlist_item    |                     | * | argument type: 'int' changed to 'size_t' |


### struct.hpp

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name              | new name                  |[1]|[2]|
|----------------------------|---------------------------|:-:|:-:|
| get_member_by_fullname     |                           | * |   |
| get_member_cmt             |                           | q | * |
| get_member_name2           | get_member_name           |   |   |
| get_member_tinfo2          | get_member_tinfo          | * |   |
| get_or_guess_member_tinfo2 | get_or_guess_member_tinfo | * |   |
| get_struc_cmt              |                           | q | * |
| retrieve_member_info       |                           | * |   |
| save_struc2                | save_struc                |   |   |
| set_member_tinfo2          | set_member_tinfo          |   |   |


### tryblks.hpp (**NEW** file)

| original name | new name    |
|---------------|-------------|
| <**added**>   | add_tryblk  |
| <**added**>   | del_tryblks |
| <**added**>   | get_tryblks |


### typeinf.hpp

NOTE: global variable 'idati' has been removed.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring

| original name                  | new name             |[1]|[2]| Notes                                                                    |
|--------------------------------|----------------------|:-:|:-:|--------------------------------------------------------------------------|
| based_ptr_name_and_size        | <**removed**>        |   |   |                                                                          |
| callregs_init_regs             | <**removed**>        |   |   |                                                                          |
| choose_local_type              | <**removed**>        |   |   |                                                                          |
| create_numbered_type_reference | <**removed**>        |   |   |                                                                          |
| equal_types                    | <**removed**>        |   |   |                                                                          |
| get_de                         | <**removed**>        |   |   |                                                                          |
| get_default_enum_size          | <**removed**>        |   |   |                                                                          |
| get_func_cvtarg_map            | <**removed**>        |   |   |                                                                          |
| get_named_type_size            | <**removed**>        |   |   |                                                                          |
| get_referred_ordinal           | <**removed**>        |   |   |                                                                          |
| get_stkarg_offset              | <**removed**>        |   |   |                                                                          |
| get_unk_type_bit               | <**removed**>        |   |   |                                                                          |
| is_restype_array               | <**removed**>        |   |   |                                                                          |
| is_restype_bitfld              | <**removed**>        |   |   |                                                                          |
| is_restype_complex             | <**removed**>        |   |   |                                                                          |
| is_restype_const               | <**removed**>        |   |   |                                                                          |
| is_restype_floating            | <**removed**>        |   |   |                                                                          |
| is_restype_func                | <**removed**>        |   |   |                                                                          |
| is_restype_ptr                 | <**removed**>        |   |   |                                                                          |
| is_restype_union               | <**removed**>        |   |   |                                                                          |
| max_ptr_size                   | <**removed**>        |   |   |                                                                          |
| rename_named_type              | <**removed**>        |   |   |                                                                          |
| set_named_type                 | <**removed**>        |   |   | use 'tinfo_t::set_named_type' instead                                    |
| set_named_type64               | <**removed**>        |   |   | use 'tinfo_t::set_named_type' instead                                    |
| <**added**>                    | append_abi_opts      |   |   |                                                                          |
| <**added**>                    | gcc_layout           |   |   |                                                                          |
| <**added**>                    | get_arg_addrs        |   |   |                                                                          |
| <**added**>                    | get_idati            |   |   | to be used instead of 'idati'                                            |
| <**added**>                    | remove_abi_opts      |   |   |                                                                          |
| <**added**>                    | resolve_typedef      |   |   |                                                                          |
| <**added**>                    | set_compiler_string  |   |   |                                                                          |
| add_til2                       | add_til              |   |   |                                                                          |
| append_tinfo_covered           |                      |   |   | argument type: 'areaset_t' has been renamed to 'rangeset_t'              |
| apply_callee_tinfo             |                      |   |   | return type changed from 'void' to 'bool'                                |
| apply_cdecl2                   | apply_cdecl          |   |   |                                                                          |
| apply_tinfo2                   | apply_tinfo          |   |   |                                                                          |
| apply_tinfo_to_stkarg          |                      |   |   | added 'insn' argument                                                    |
| build_anon_type_name           |                      |   | * |                                                                          |
| calc_c_cpp_name4               | calc_c_cpp_name      |   |   |                                                                          |
| calc_tinfo_gaps                |                      |   |   | argument type: 'areaset_t' has been renamed to 'rangeset_t'              |
| choose_local_tinfo             |                      |   |   | added 'def_ord' argument                                                 |
| choose_named_type2             | choose_named_type    | * |   | the original 'choose_named_type' has been removed                        |
| create_numbered_type_name      |                      | q | * | return type changed from 'size_t' to 'ssize_t'                           |
| decorate_name3                 | decorate_name        |   |   | added 'type' argument                                                    |
| del_tinfo_attr                 |                      |   |   | added 'make_copy' argument                                               |
| deref_ptr2                     | deref_ptr            | * |   |                                                                          |
| extract_argloc                 |                      | * |   |                                                                          |
| find_tinfo_udt_member          |                      | * |   |                                                                          |
| format_cdata2                  | format_cdata         |   |   |                                                                          |
| gen_decorate_name3             | gen_decorate_name    |   |   | the original 'gen_decorate_name' has been removed; added 'type' argument |
| get_c_header_path              |                      |   | * |                                                                          |
| get_c_macros                   |                      |   | * |                                                                          |
| get_enum_member_expr2          | get_enum_member_expr |   | * |                                                                          |
| get_idainfo_by_type3           | get_idainfo_by_type  | * |   |                                                                          |
| get_int_type_bit               | get_scalar_bt        |   |   |                                                                          |
| get_tinfo_pdata                |                      | * |   |                                                                          |
| get_tinfo_size                 |                      | * |   |                                                                          |
| guess_tinfo2                   | guess_tinfo          | * |   |                                                                          |
| load_til2                      | load_til             |   | * | the original 'load_til' has been removed; added 'tildir' argument        |
| load_til_header                |                      |   | * |                                                                          |
| lower_type2                    | lower_type           |   |   |                                                                          |
| optimize_argloc                |                      |   |   | argument type: 'areaset_t' has been renamed to 'rangeset_t'              |
| parse_decl2                    | parse_decl           | q |   |                                                                          |
| print_type3                    | print_type           |   |   |                                                                          |
| remove_tinfo_pointer           |                      | * |   |                                                                          |
| save_tinfo                     |                      | * |   |                                                                          |
| set_abi_name                   |                      |   |   | added 'user_level' argument                                              |
| set_compiler2                  | set_compiler         |   |   |                                                                          |
| set_numbered_type              |                      |   |   | return type changed from 'bool' to 'tinfo_code_t'                        |
| verify_argloc                  |                      |   |   | argument type: 'areaset_t' has been renamed to 'rangeset_t'              |


### ua.hpp

WARNING: The global variables 'cmd' and 'uFlag' are gone.

All functions previously operating on 'cmd' now accept an 'insn_t' pointer or reference.
Use get_flags() (or, if you really need it, get_full_flags()) to read the current flags.

NOTE: The maximum number of instruction operands (UA_MAXOP) has increased to 8.

NOTE: class 'outctx_base_t' has been added to replace functions that generate the disassembly text

NOTE: global variable 'lookback' has been removed.

- [1] output argument moved to beginning of argument list
    - q: argument is a qstring
- [2] output buffer converted to qstring
- [3] added input/output 'insn_t &insn' argument
- [4] added input 'const insn_t &insn' argument
- [5] added output 'insn_t *out' argument

| original name             | new name          |[1]|[2]|[3]|[4]|[5]| Notes                                                                        |
|---------------------------|-------------------|:-:|:-:|:-:|:-:|:-:|------------------------------------------------------------------------------|
| OutBadInstruction         | <**removed**>     |   |   |   |   |   |                                                                              |
| OutChar                   | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_char' instead                                        |
| OutImmChar                | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_immchar_cmts' instead                                |
| OutLine                   | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_line' instead                                        |
| OutLong                   | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_btoa' instead                                        |
| OutMnem                   | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_mnem' instead                                        |
| OutValue                  | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_value' instead                                       |
| get_output_ptr            | <**removed**>     |   |   |   |   |   |                                                                              |
| init_output_buffer        | <**removed**>     |   |   |   |   |   |                                                                              |
| out_addr_tag              | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_addr_tag' instead                                    |
| out_colored_register_line | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_colored_register_line' instead                       |
| out_insert                | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::outbuf' directly instead                                 |
| out_line                  | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_line' instead                                        |
| out_long                  | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_long' instead                                        |
| out_name_expr             | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_name_expr' instead                                   |
| out_one_operand           | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_one_operand' instead                                 |
| out_snprintf              | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_printf' instead                                      |
| out_symbol                | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_symbol' instead                                      |
| out_tagoff                | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_tagoff' instead                                      |
| out_tagon                 | <**removed**>     |   |   |   |   |   | use 'outctx_base_t::out_tagon' instead                                       |
| set_output_ptr            | <**removed**>     |   |   |   |   |   |                                                                              |
| term_output_buffer        | <**removed**>     |   |   |   |   |   |                                                                              |
| ua_dodata2                | <**removed**>     |   |   |   |   |   | use 'insn_t::create_op_data' instead                                         |
| ua_next_byte              | <**removed**>     |   |   |   |   |   | use 'insn_t::get_next_byte' instead                                          |
| ua_next_long              | <**removed**>     |   |   |   |   |   | use 'insn_t::get_next_dword' instead                                         |
| ua_next_qword             | <**removed**>     |   |   |   |   |   | use 'insn_t::get_next_qword' instead                                         |
| ua_next_word              | <**removed**>     |   |   |   |   |   | use 'insn_t::get_next_word' instead                                          |
| <**added**>               | can_decode        |   |   |   |   |   |                                                                              |
| <**added**>               | create_outctx     |   |   |   |   |   |                                                                              |
| <**added**>               | get_lookback      |   |   |   |   |   | to be used instead of 'lookback'                                             |
| <**added**>               | map_ea            |   |   |   |   |   |                                                                              |
| codeSeg                   | map_code_ea       |   |   |   | * |   | input arguments changed to either 'const op_t &op' or 'ea_t addr, int opnum' |
| construct_macro           |                   |   |   | * |   |   |                                                                              |
| create_insn               |                   |   |   |   |   | * |                                                                              |
| dataSeg, dataSeg_op       | map_data_ea       |   |   |   | * |   | input arguments changed to either 'const op_t &op' or 'ea_t addr, int opnum' |
| dataSeg_opreg             | calc_dataseg      |   |   |   | * |   |                                                                              |
| decode_insn               |                   |   |   |   |   | * |                                                                              |
| decode_preceding_insn     |                   |   |   |   |   | * |                                                                              |
| decode_prev_insn          |                   |   |   |   |   | * |                                                                              |
| get_dtyp_by_size          | get_dtype_by_size |   |   |   |   |   | return type changed from 'char' to 'op_dtype_t'                              |
| get_dtyp_flag             | get_dtype_flag    |   |   |   |   |   | argument type: 'char' changed to 'op_dtype_t'                                |
| get_dtyp_size             | get_dtype_size    |   |   |   |   |   | argument type: 'char' changed to 'op_dtype_t'                                |
| get_operand_immvals       | get_immvals       | * |   |   |   |   | added 'flags_t' and 'cache' arguments                                        |
| get_spoiled_reg           |                   |   |   |   | * |   |                                                                              |
| guess_table_address       |                   |   |   |   | * |   |                                                                              |
| guess_table_size          |                   |   |   |   | * |   |                                                                              |
| out_real                  | print_fpval       | * |   |   |   |   |                                                                              |
| showAsChar                | print_charlit     | * |   |   |   |   |                                                                              |
| ua_add_cref               | <**removed**>     |   |   |   |   |   | use 'insn_t::add_cref' instead                                               |
| ua_add_dref               | <**removed**>     |   |   |   |   |   | use 'insn_t::add_dref' instead                                               |
| ua_add_off_drefs2         | <**removed**>     |   |   |   |   |   | use 'insn_t::add_off_drefs' instead                                          |
| ua_mnem                   | print_insn_mnem   | q | * |   |   |   |                                                                              |
| ua_outop2                 | print_operand     | q | * |   |   |   | added 'printop_t' argument                                                   |
| ua_stkvar2                | <**removed**>     |   |   |   |   |   | use 'insn_t::create_stkvar' instead                                          |


### xref.hpp

- [1] output argument moved to beginning of argument list
- [2] input argument changed from pointer to reference

| original name       |[1]|[2]|
|---------------------|:-:|:-:|
| calc_switch_cases   | * | * |
| create_switch_table |   | * |
| create_switch_xrefs |   | * |

