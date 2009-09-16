#if defined(HIPPOENV_SPARC)
#include "NativeSparc.cpp"
#elif defined(HIPPOENV_PPC)
#include "NativePPC.cpp"
#elif defined(HIPPOENV_ARM)
#include "NativeARM.cpp"
#elif defined(HIPPOENV_X64)
#include "NativeX64.cpp"
#elif defined(HIPPOENV_X32)
#include "Nativei386.cpp"
#else
#error Unknown platform for JIT
#endif
