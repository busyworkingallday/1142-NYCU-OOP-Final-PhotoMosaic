#ifndef _TEMP_FILE_GUARD_H_
#define _TEMP_FILE_GUARD_H_

#include <cstdio>   // std::remove
#include <string>

// RAII guard: deletes the named temp file when it goes out of scope, so a
// temporary dumped for Display_CMD is cleaned up unconditionally -- even if
// Dump_* / Display_*_CMD throws between creating and removing it.
class TempFileGuard {
    std::string path;
public:
    explicit TempFileGuard(const std::string &p) : path(p) {}
    ~TempFileGuard() { std::remove(path.c_str()); }

    TempFileGuard(const TempFileGuard &) = delete;
    TempFileGuard &operator=(const TempFileGuard &) = delete;
};

#endif
