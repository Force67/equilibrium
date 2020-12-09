// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QVariant>

namespace utils {

  struct OptBase;

  class OptRegistry {
  public:
	OptRegistry() = default;
	~OptRegistry() = default;

	explicit OptRegistry(OptBase *handler) noexcept;

	// initialize values for OPTS
	static size_t Init();

  private:
	static OptRegistry *&Root() noexcept;

	OptRegistry *next = nullptr;
	OptBase *item = nullptr;
  };

  // Register next item
  inline OptRegistry::OptRegistry(OptBase *item) noexcept :
      next(Root()),
      item(item)
  {
	Root() = this;
  }

  inline OptRegistry *&OptRegistry::Root() noexcept
  {
	static OptRegistry *root{ nullptr };
	return root;
  }

  // Option Base
  struct OptBase : OptRegistry {
	OptBase() noexcept;

	const char *name = nullptr;
	QVariant data;
  };

  inline OptBase::OptBase() noexcept :
      OptRegistry(this)
  {
  }

  template <typename T>
  struct Opt : OptBase {
	constexpr explicit Opt(T default, const char *name)
	{
	  OptBase::data.setValue(default);
	  OptBase::name = name;
	}

	// assignment operator
	T operator=(const T &val)
	{
	  OptBase::data.setValue(val);
	  return OptBase::data.value<T>();
	}

	// convert to type
	operator T() { return OptBase::data.value<T>(); }
  };
} // namespace utils