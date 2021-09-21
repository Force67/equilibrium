// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace ial {
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
    return NetNode(name, false).IsOpen();
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
}