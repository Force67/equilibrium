// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// a set of wrappers around the often annoying ida api.
#pragma once

#include <QString>
#include <functional>

namespace utils {

class NetNode {
 public:
  explicit NetNode(const char* name, bool create = true) {
    node_ = netnode(name, -1, create);
  }
  NetNode() { node_ = BADNODE; }
  NetNode(const NetNode& node) { node_ = node.node_; }

  void Reset(const char* name, bool create = true) {
    node_ = netnode(name, -1, create);
  }
  bool Create(const char* name) { return node_.create(name, -1); }

  bool IsOpen() const { return nodeidx_t() != BADNODE; }
  void Clear() { node_ = BADNODE; }

  static bool Exists(const char* name) {
    return utils::NetNode(name, false).IsOpen();
  }

  template <typename T,
            class = typename std::enable_if<std::is_pod<T>::value>::type>
  T Read(nodeidx_t idx) {
    T temp;
    node_.supval(idx, &temp, sizeof(T));
    return temp;
  }

  qstring Read(nodeidx_t idx) {
    qstring str;
    node_.supstr(&str, idx);
    return str;
  }

  template <typename T,
            class = typename std::enable_if<std::is_pod<T>::value>::type>
  T Read(nodeidx_t idx, T fallback) {
    T temp = fallback;
    node_.supval(idx, &temp, sizeof(T));
    return temp;
  }

  template <typename T,
            class = typename std::enable_if<std::is_pod<T>::value>::type>
  bool Write(nodeidx_t idx, T t) {
    return node_.supset(idx, reinterpret_cast<const void*>(&t), sizeof(T));
  }

  bool Write(nodeidx_t idx, qstring& str) { return node_.supstr(&str, idx); }

 private:
  netnode node_;
};

namespace InputFile {
static inline std::string RetrieveInputFileNameMD5() {
  uchar md5[16]{};
  bool result = retrieve_input_file_md5(md5);
  assert(result != false);

  // cxx string will null terminate
  char buf[32 + 1]{};

  // convert bytes to str
  constexpr char lookup[] = "0123456789abcdef";
  for (int i = 0; i < 16; i++) {
    buf[i * 2] = lookup[(md5[i]) >> 4 & 0xF];
    buf[i * 2 + 1] = lookup[(md5[i]) & 0xF];
  }

  return buf;
}

static inline std::string GetInputFileName() {
  char fileName[128]{};
  get_root_filename(fileName, sizeof(fileName) - 1);

  return fileName;
}
}  // namespace InputFile

namespace IDB {
static inline bool IsBusy() {
  return !auto_is_ok();
}
}  // namespace IDB

template <typename T>
struct RequestFunctor final : exec_request_t {
 public:
  RequestFunctor(std::function<T> callback, int flags) {
    callback_ = callback;
    execute_sync(*this, flags);
  }

  int execute() override {
    callback_();
    return 0;
  }

 private:
  std::function<T> callback_;
};

template <hook_type_t HT, typename NT>
class EventHandler {
 public:
  EventHandler() { hook_to_notification_point(HT, StaticHandler, this); }
  ~EventHandler() { unhook_from_notification_point(HT, StaticHandler, this); }

  // TODO: CXX 20 constexpr virtual
  // we make overriding multiple inherited event handlers possible by
  // specializing function name with NT
  //    This allows us to do:
  /*
  class... : public UiEventHandler,
             public IdbEventHandler {
  void HandleEvent(ui_notification_t code, va_list args) override {}
  void HandleEvent(idb_event::event_code_t code, va_list args) override {}
  */
  virtual void HandleEvent(NT code, va_list args) = 0;

 private:
  static inline ssize_t StaticHandler(void* userp, int code, va_list args) {
    if (auto* self = reinterpret_cast<EventHandler<HT, NT>*>(userp))
      self->HandleEvent(static_cast<NT>(code), args);
    return 0;
  }
};
using UiEventHandler = EventHandler<hook_type_t::HT_UI, ui_notification_t>;
using IdbEventHandler =
    EventHandler<hook_type_t::HT_IDB, idb_event::event_code_t>;
}  // namespace utils