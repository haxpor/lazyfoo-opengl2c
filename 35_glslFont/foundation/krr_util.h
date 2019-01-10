#ifndef krr_util_h_
#define krr_util_h_

///
/// Print callstack up to this point of calling this function.
/// Stack size is 32.
///
/// Only supported on Linux, Unix, and macOS. It will be no-op for Windows and other platforms.
///
void krr_util_print_callstack();

#endif
