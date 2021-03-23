// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// a set of wrappers around the often annoying ida api.
#pragma once

#include <QString>

namespace utils {

class NetNode {
 public:
  explicit NetNode(const char* name, bool create = true) {
    node_ = netnode(name, -1, create);
  }

  NetNode() { node_ = BADNODE; }
  NetNode(const NetNode& node) { node_ = node.node_; }

  inline bool open() const { return nodeidx_t() != BADNODE; }
  void clear() { node_ = BADNODE; }

  // check value bool operator

  bool Create(const char* name) { return node_.create(name, -1); }

  template <typename T>
  T LoadScalar(nodeidx_t idx) {
    T temp;
    node_.supval(idx, &temp, sizeof(T));
    return temp;
  }

  template <typename T>
  T LoadScalar(nodeidx_t idx, T fallback) {
    T temp = fallback;
    node_.supval(idx, &temp, sizeof(T));
    return temp;
  }

  template <typename T>
  bool StoreScalar(nodeidx_t idx, T t) {
    return node_.supset(idx, reinterpret_cast<const void*>(&t), sizeof(T));
  }

 private:
  netnode node_;
};

class InputFile {
 public:
  // must be 33 bytes big!!
  static inline std::string RetrieveInputFileNameMD5() {
    uchar md5[16]{};
    bool result = retrieve_input_file_md5(md5);
    assert(result != false);

    // cxx string will null terminate
    char buf[32];

    // convert bytes to str
    constexpr char lookup[] = "0123456789abcdef";
    for (int i = 0; i < 16; i++) {
      buf[i * 2] = lookup[(md5[i]) >> 4 & 0xF];
      buf[i * 2 + 1] = lookup[(md5[i]) & 0xF];
    }

    return buf;
  }
};

template<typename T>
struct RequestFunctor final : exec_request_t {
 public:
  RequestFunctor(std::function<T> callback) { 
      callback_ = callback;
    execute_sync(*this, MFF_WRITE | MFF_NOWAIT);
  }

  int execute() override { 
     callback_();
     return 0;
  }

 private:
  std::function<T> callback_;
};

}