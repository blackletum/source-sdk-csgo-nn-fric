#pragma once

#include <SDL3/SDL_platform_defines.h>

class InterfaceReg {
public:
    InterfaceReg(void *(*CreateFN)(), const char *pInterfaceName);

    ~InterfaceReg() = default;

    void *(*m_CreateInterface)();

    const char *m_pInterfaceName;

    InterfaceReg *m_pNext;
};

extern "C"
#ifdef SDL_PLATFORM_WINDOWS
__declspec(dllexport)
#endif
void *CreateInterface(const char *pName, int *pReturnCode);

#define EXPOSE_INTERFACE(className, interfaceName, versionName) \
	static void *__Create##className##_interface() { return static_cast<interfaceName *>(new className); } \
	static InterfaceReg __g_Create##className##_reg(__Create##className##_interface, versionName);

enum {
    IFACE_OK,
    IFACE_FAILED
};
