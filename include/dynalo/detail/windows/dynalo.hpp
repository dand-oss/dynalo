#pragma once

#include <string>
#include <stdexcept>

#include <Windows.h>
#include <strsafe.h>

namespace dynalo { namespace detail
{
inline
std::string last_error()
{
    // https://msdn.microsoft.com/en-us/library/ms680582%28VS.85%29.aspx
    const auto dw = ::GetLastError();

    STRSAFE_LPSTR lpMsgBuf = 0; // will alloc this
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&lpMsgBuf),
        0,
        nullptr );

    // get enough space for formatted string
    auto lpDisplayBuf = (STRSAFE_LPSTR) LocalAlloc(LMEM_ZEROINIT, strlen(lpMsgBuf) + 40);

    // fancy sprintf
    StringCchPrintfA(
        lpDisplayBuf,
        LocalSize(lpDisplayBuf),
        "Failed with error %d: %s",
        dw,
        lpMsgBuf);

    // copy out
    std::string err_str(lpDisplayBuf);

    // done
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

    return err_str;
}

namespace native
{

using handle = HMODULE;

inline handle invalid_handle() { return nullptr; }

namespace name
{

inline std::string prefix()    { return std::string();      }
inline std::string suffix()    { return std::string();      }
inline std::string extension() { return std::string("dll"); }

}

}

inline
native::handle open(const std::string& dyn_lib_path)
{
    native::handle lib_handle = ::LoadLibraryA(dyn_lib_path.c_str());
    if (lib_handle == nullptr)
    {
        throw std::runtime_error(std::string("Failed to open [dyn_lib_path:") + dyn_lib_path + "]: " + last_error());
    }

    return lib_handle;
}

inline
void close(native::handle lib_handle)
{
    const BOOL rc = ::FreeLibrary(lib_handle);
    if (rc == 0)  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms683152(v=vs.85).aspx
    {
        throw std::runtime_error(std::string("Failed to close the dynamic library: ") + last_error());
    }
}

inline
native::handle get_exe_handle()
{
    return GetModuleHandle(nullptr); // assume exe
}

template <typename FunctionSignature>
inline
FunctionSignature* get_function(native::handle lib_handle, const std::string& func_name)
{
    FARPROC func_ptr = ::GetProcAddress(lib_handle, func_name.c_str());
    if (func_ptr == nullptr)
    {
        throw std::runtime_error(std::string("Failed to get [func_name:") + func_name + "]: " + last_error());
    }

    return reinterpret_cast<FunctionSignature*>(func_ptr);
}

}}
