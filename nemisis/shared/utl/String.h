#pragma once

//
// Copyright (c) Fireboyd78 2019
//

#include <cstdarg>
#include <string>
#include <memory>

namespace utl
{
	template <typename _Ty>
	struct string_traits
	{
		using type = _Ty;
		using ctype = const _Ty;

		using pointer = _Ty *;
		using cpointer = const _Ty *;

		using reference = _Ty &;
		using creference = const _Ty &;

		static constexpr _Ty null = 0;
	};

	template <class _Elem,
		class _Traits = string_traits<_Elem>>
	class basic_string_alloc {};

	template <>
	class basic_string_alloc<char>
	{
	public:
		using traits = string_traits<char>;

		using type = typename traits::type;
		using pointer = typename traits::pointer;
		using cpointer = typename traits::cpointer;

		size_t _Strlen(pointer str)
		{
			return strlen(str);
		}

		inline size_t _Strnlen(pointer str, size_t size)
		{
			return strnlen_s(str, size);
		}

		inline void _Assign(pointer dst, size_t size, cpointer src)
		{
			strcpy_s(dst, size, src);
		}

		inline void _Assign(pointer dst, size_t dstSize, cpointer src, size_t size)
		{
			strncpy_s(dst, dstSize, src, size);
		}

		inline size_t _Format(pointer dst, size_t size, cpointer fmt, va_list args)
		{
			return vsprintf_s(dst, size, fmt, args);
		}
	};

	template <>
	class basic_string_alloc<wchar_t>
	{
	public:
		using traits = string_traits<wchar_t>;

		using type = typename traits::type;
		using pointer = typename traits::pointer;
		using cpointer = typename traits::cpointer;

		inline size_t _Strlen(pointer str)
		{
			return wcslen(str);
		}

		inline size_t _Strnlen(pointer str, size_t size)
		{
			return wcsnlen_s(str, size);
		}

		inline void _Assign(pointer dst, size_t size, cpointer src)
		{
			wcscpy_s(dst, size, src);
		}

		inline void _Assign(pointer dst, size_t dstSize, cpointer src, size_t size)
		{
			wcsncpy_s(dst, dstSize, src, size);
		}

		inline size_t _Format(pointer dst, size_t size, cpointer fmt, va_list args)
		{
			return vswprintf_s(dst, size, fmt, args);
		}
	};

	template <class _Elem,
		class _Traits = string_traits<_Elem>>
	class string_alloc
		: public basic_string_alloc<_Elem>
	{
	public:
		using traits = _Traits;

		using type = typename traits::type;
		using pointer = typename traits::pointer;
		using cpointer = typename traits::cpointer;

		constexpr void _Assign(pointer dst, size_t size, int val)
		{
			memset(dst, val, size);
		}
	};

	template <class _Elem,
		size_t _Size,
		class _Traits = string_traits<_Elem>>
	class basic_string_buf
		: string_alloc<_Elem>
	{
		static_assert(_Size > 0, "You must specify a size greater than zero!");

	public:
		using traits = _Traits;

		using type = typename traits::type;
		using pointer = typename traits::pointer;
		using cpointer = typename traits::cpointer;

		constexpr basic_string_buf() {}

		constexpr basic_string_buf(type(&buf)[_Size])
			: buffer(buf) {}

		constexpr basic_string_buf(cpointer src) {
			assign(src);
		}

		template <typename ...TArgs>
		constexpr basic_string_buf(cpointer fmt, TArgs ...args) {
			format(fmt, args...);
		}

		constexpr pointer operator &() const {
			return &buffer;
		}

		constexpr pointer operator[](int index) const {
			return &buffer[index];
		}

		constexpr operator pointer() const {
			return buffer;
		}

		constexpr operator cpointer() const {
			return buffer;
		}

		constexpr size_t size() const {
			return _Size;
		}

		constexpr size_t length() const {
			return _Strnlen(buffer, _Size);
		}

		constexpr void clear() {
			buffer[0] = traits::null;
		}

		constexpr void reset() {
			// reset the entire buffer			
			_Assign(buffer, traits::null, size());
		}

		constexpr cpointer ptr() const {
			return buffer;
		}

		constexpr void assign(cpointer str) {
			_Assign(buffer, str, size());
		}

		constexpr void assign(cpointer str, size_t length) {
			_Assign(buffer, str, size());
		}

		constexpr size_t format(cpointer fmt, ...) {
			size_t result = 0;

			va_list args;
			va_start(args, fmt);
			result = _Format(buffer, size(), fmt, args);
			va_end(args);

			return result;
		}

		constexpr void append(cpointer str) {
			size_t idx = length();

			_Assign(&buffer[idx], size() - idx, str);
		}

		constexpr void append(cpointer fmt, ...) {
			size_t idx = length();

			va_list args;
			va_start(args, fmt);
			result = _Format(&buffer[idx], size() - idx, fmt, args);
			va_end(args);
		}
	private:
		type buffer[_Size] = { traits::null };
	};

	template <size_t _Size>
	using string_buf = basic_string_buf<char, _Size>;

	template <size_t _Size>
	using wstring_buf = basic_string_buf<wchar_t, _Size>;

	/*
		basic_string_buf -> std::basic_string
	*/
	template <class _Elem,
		size_t _Size,
		class _Target = basic_string_buf<_Elem, _Size>
	>
	_Target
	convert_string(std::basic_string<_Elem> &str)
	{
		_Target result;
		result.assign(str.ptr(), str.length());

		return result;
	}

	/*
		std::basic_string -> basic_string_buf
	*/
	template <class _Elem,
		size_t _Size
	>
	std::basic_string<_Elem>
	convert_string(basic_string_buf<_Elem, _Size> &str)
	{
		std::basic_string<_Elem> result;
		result.assign(str.ptr());

		return result;
	}
}