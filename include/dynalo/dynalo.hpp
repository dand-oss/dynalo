#pragma once

#include "detail/config.hpp"

#if defined(DYNALO_HAS_LINUX)
    #include "detail/linux/dynalo.hpp"
#elif defined(DYNALO_HAS_WINDOWS)
    #include "detail/windows/dynalo.hpp"
#elif defined(DYNALO_HAS_MACOS)
    #include "detail/macos/dynalo.hpp"
#endif

/// DYNAmic LOading of shared libraries and access to their exported functions
namespace dynalo
{

namespace native
{

/// Native handle (usuall a pointer) on the loaded shared library
using handle = detail::native::handle;

/// @return An invalid library handle
inline
handle invalid_handle()
{
    return detail::native::invalid_handle();
}

namespace name
{

/// @return The name prefix of a shared library on the current system
inline
std::string prefix()
{
    return detail::native::name::prefix();
}

/// @return The name suffix of a shared library on the current system
inline
std::string suffix()
{
    return detail::native::name::suffix();
}

/// @return The file extension of a shared library on the current system
inline
std::string extension()
{
    return detail::native::name::extension();
}

} // namespace name

} // namespace native


/// @param lib_name Name of the library without neither the lib prefix, suffix nor file extension
///
/// @return The native name of the shared library.
///         e.g. If @p lib_name is `awesome`,
///         then this function will return `libawesome.so` in Linux and `awesome.dll` in Windows
inline
std::string to_native_name(const std::string& lib_name)
{
    using namespace native::name;
    if (!extension().empty())
    {
        return prefix() + lib_name + suffix() + std::string(".") + extension();
    }
    else
    {
        return prefix() + lib_name + suffix();
    }
}

/// Loads a shared library
///
/// @param dyn_lib_path Path to the shared library file to be loaded
///
/// @return The handle of the loaded shared library
///
/// @throw std::runtime_error If it fails to load the library
inline
native::handle open(const std::string& dyn_lib_path)
{
    return detail::open(dyn_lib_path);
}

/// Unloads the shared library which handle is @p lib_handle
///
/// @param lib_handle The handle of the library to be unloaded
///
/// @throw std::runtime_error If it fails to unload the shared library
inline
void close(native::handle lib_handle)
{
    detail::close(lib_handle);
}

/// Looks up a function in the shared library and returns pointer to it
///
/// @tparam FunctionSignature The signature of the function to be looked up.
///                           i.e. `return_type(param_types...)`
///                           e.g. `void(const char*)`, `bool(int, int)`
///
/// @param lib_handle The handle of the library that contains the function
/// @param func_name  The name of the function to find
///
/// @return A pointer to the @p func_name function
///
/// @throw std::runtime_error If it fails to find the @p func_name function
template <typename FunctionSignature>
inline
FunctionSignature* get_function(native::handle lib_handle, const std::string& func_name)
{
    return detail::get_function<FunctionSignature>(lib_handle, func_name);
}

inline
native::handle get_exe_handle()
{
    return detail::get_exe_handle();
}


/// A shared library
///
/// This class wraps the open/close/get_function functions of the dynalo namespace:
/// <ul>
///     <li>The shared library is loaded in the class' constructor</li>
///     <li>You can get pointer to a functio using library::get_function</li>
///     <li>The shared library is automatically unloaed in the destructor</li>
/// </ul>
class library
{
private:
    native::handle m_handle { native::invalid_handle() } ;

public:
    library() = default;
    // copy
    library(const library& rhs) = delete ;
    library& operator=(const library& rhs ) = delete ;

    // move
    library(library&& rhs)
        : m_handle(rhs.m_handle) {
        rhs.invalidate();
    }

    library& operator=(library&& rhs) {
        if ( &rhs != this ) {
            m_handle       = rhs.m_handle;
            rhs.invalidate();
        }
        return *this;
    }

    /// Unloads the shared library using dynalo::close
    ~library() {
        close();
    }

    /// Loads a shared library using dynalo::open
    explicit library(const std::string& dyn_lib_path) {
        open(dyn_lib_path);
    }

    void open(const std::string& dyn_lib_path) {
        m_handle = dynalo::open(dyn_lib_path);
    }

    void close() {
       if ( is_loaded() ) {
           dynalo::close(m_handle);
           invalidate();
       }
    }
    
    void invalidate() {
        m_handle = native::invalid_handle();
    }

    bool is_loaded() const {
        return m_handle != native::invalid_handle() ;
    }

    /// Returns a pointer to the @p func_name function using dynalo::get_function
    template <typename FunctionSignature>
    FunctionSignature* get_function(const std::string& func_name) const
    {
        return dynalo::get_function<FunctionSignature>(m_handle, func_name);
    }

    /// Returns the native handle of the loaded shared library
    native::handle get_native_handle() const
    {
        return m_handle;
    }

};

}
