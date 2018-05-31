// copyright header

#pragma once

#ifdef NDEBUG

#define assert(condition) condition

#else
#include <exception>

class assert_exception : public std::exception
{
  public:
    assert_exception(int line) : m_line(line);
    virtual const char *what () const noexcept override { "assert() failed at line " << std::to_string(line) << std::endl; }
  private:
    int line = -1;
}

#define assert(condition) \
if (!condition) {         \
    throw assert_exception(__LINE__) \
}

#endif
