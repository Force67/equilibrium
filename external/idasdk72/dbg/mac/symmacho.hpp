#ifndef SYMMACHO_H
#define SYMMACHO_H

// manage the mach-o images in a darwin process

#include <pro.h>
#include <map>

class debmod_t;
class linput_t;

typedef std::map<ea_t, qstring> strings_cache_t;

//--------------------------------------------------------------------------
struct dyld_all_image_infos_t
{
  uint32 version;
  uint32 num_info;
  ea_t info_array;
  ea_t dyld_notify;
  ea_t dyld_image_load_address;
  ea_t dyld_image_infos_address;
  ea_t shared_cache_slide;
  ea_t shared_cache_base_address;

  dyld_all_image_infos_t() { clear(); }

  void clear();
};

//--------------------------------------------------------------------------
enum dyld_image_mode_t
{
  DYLD_IMAGE_ERROR = -1,
  DYLD_IMAGE_ADDING = 0,
  DYLD_IMAGE_REMOVING = 1,
  DYLD_IMAGE_INFO_CHANGE = 2,
};

//--------------------------------------------------------------------------
struct dll_visitor_t
{
  virtual void visit_dll(
          ea_t base,
          asize_t size,
          const char *name,
          const bytevec_t &uuid) = 0;

  DEFINE_VIRTUAL_DTOR(dll_visitor_t)
};

//--------------------------------------------------------------------------
struct macho_visitor_t
{
  int flags;
#define MV_UUID             0x0001 // visit uuid
#define MV_FUNCTION_STARTS  0x0002 // visit function start eas
#define MV_SYMBOLS          0x0004 // visit symbols
#define MV_SEGMENTS         0x0008 // visit segments
#define MV_SECTIONS         0x0010 // visit sections

  macho_visitor_t(int _flags) : flags(_flags) {}

  virtual void visit_uuid(const bytevec_t & /*uuid*/) {}
  virtual void visit_function_start(ea_t /*ea*/) {}
  virtual void visit_symbol(ea_t /*ea*/, const char * /*name*/) {}
  virtual void visit_segment(ea_t /*start_ea*/, ea_t /*end_ea*/, const qstring & /*name*/, bool /*is_code*/) {}
  virtual void visit_section(ea_t /*start_ea*/, ea_t /*end_ea*/, const qstring & /*name*/, bool /*is_code*/) {}

  // called when function start info could not be found/loaded
  virtual void handle_function_start_error() {}
  // called just before a symbol is visited when cpu is CPU_TYPE_ARM
  virtual void handle_thumb(ea_t /*ea*/, const char * /*name*/, bool /*is_thumb*/) {}
};

//--------------------------------------------------------------------------
struct dyld_cache_visitor_t
{
  int flags;
#define DCV_MAPPINGS 0x1 // visit shared region mappings

  dyld_cache_visitor_t(int _flags) : flags(_flags) {}

  virtual void visit_mapping(ea_t /*start_ea*/, ea_t /*end_ea*/) {}
};

//--------------------------------------------------------------------------
class dyld_utils_t
{
  debmod_t *dbgmod;

  int arch;     // PLFM_386 or PLFM_ARM
  int addrsize; // size of an address in the target process
  bool is64;    // is target process 64-bit?
  bool warned;  // warned the user about using SYMBOL_PATH when remote debugging

  strings_cache_t strcache;

  const char *get_cfgname() const;
  int get_cputype() const;

  template<typename H> bool is_dyld_header(ea_t base, char *filename, size_t namesize, uint32 magic);

  bool is_dyld_header_64(ea_t base, char *filename, size_t namesize);
  bool is_dyld_header_32(ea_t base, char *filename, size_t namesize);

public:
  ea_t base_ea;   // base address of dyld ifself
  ea_t entry_ea;  // dyld's entry point
  ea_t infos_ea;  // address of _dyld_all_image_infos
  ea_t ranges_ea; // address of _dyld_shared_cache_ranges

  dyld_all_image_infos_t infos;

  rangeset_t shared_cache_ranges;

  qstring symbol_path;

  dyld_utils_t(debmod_t *_dbgmod, int _arch);

  void clear();

  size_t read_mem(ea_t ea, void *buf, size_t size);
  bool read(ea_t ea, void *buf, size_t size) { return read_mem(ea, buf, size) == size; }
  void get_ptr_value(ea_t *val, const uchar *buf) const;

  bool is_shared_cache_lib(ea_t base) const { return shared_cache_ranges.contains(base); }
  bool is_system_lib(ea_t base) const { return base == base_ea || is_shared_cache_lib(base); }

  bool is_dyld_header(ea_t base, char *filename, size_t namesize);
  bool is_exe_header(ea_t base);

  bool untag(ea_t *ea) const;

  void update_bitness();
  bool update_infos();
  bool update_ranges();

  bool parse_info_array(uint32 count, ea_t info_array, dll_visitor_t &dv);

  bool parse_macho_file(const char *path, ea_t base, macho_visitor_t &mv, const bytevec_t &uuid) const;
  bool parse_macho_input(linput_t *li, ea_t base, macho_visitor_t &mv) const;
  bool parse_macho_mem(ea_t base, macho_visitor_t &mv);

  linput_t *create_mem_input(ea_t base);

  bool calc_macho_image_size(asize_t *size, linput_t *li, ea_t *p_base = NULL) const;
  bool calc_macho_uuid(bytevec_t *uuid, linput_t *li) const;
  bool match_macho_uuid(linput_t *li, const bytevec_t &uuid) const;

  void calc_image_info(asize_t *size, bytevec_t *uuid, ea_t base);
  void calc_image_info(ea_t *base, asize_t *size, bytevec_t *uuid, const char *path) const;

  bool parse_dyld_cache_header(dyld_cache_visitor_t &dcv);

  bool get_symbol_file_path(qstring *path, const char *module) const;
  bool parse_local_symbol_file(
          ea_t base,
          const char *module,
          const bytevec_t &uuid,
          macho_visitor_t &mv);
};

#endif // SYMMACHO_H
