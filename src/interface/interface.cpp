#include <interface/interface.h>

#include <cstring>

static InterfaceReg *g_pCurrentInterfaceReg = nullptr;

InterfaceReg::InterfaceReg(void *(*CreateFN)(), const char *pInterfaceName) : m_pInterfaceName(pInterfaceName) {
    m_CreateInterface = CreateFN;
    m_pNext = g_pCurrentInterfaceReg;
    g_pCurrentInterfaceReg = this;
}

extern "C" void *CreateInterface(const char *pName, int *pReturnCode) {
    for (const InterfaceReg *pCur = g_pCurrentInterfaceReg; pCur; pCur = pCur->m_pNext) {
        if (strcmp(pCur->m_pInterfaceName, pName) == 0) {
            if (pReturnCode)
                *pReturnCode = IFACE_OK;

            return pCur->m_CreateInterface();
        }
    }

    if (pReturnCode)
        *pReturnCode = IFACE_FAILED;

    return nullptr;
}
