#include <StdInc.h>
#include <AyriaInit.h>

static InitFunctionBase* g_initFunctions;

InitFunctionBase::InitFunctionBase(int order /* = 0 */)
    : m_order(order)
{

}

void InitFunctionBase::Register()
{
    if (!g_initFunctions)
    {
        m_next = nullptr;
        g_initFunctions = this;
    }
    else
    {
        InitFunctionBase* cur = g_initFunctions;
        InitFunctionBase* last = nullptr;

        while (cur && m_order >= cur->m_order)
        {
            last = cur;
            cur = cur->m_next;
        }

        m_next = cur;

        (!last ? g_initFunctions : last->m_next) = this;
    }
}

void InitFunctionBase::RunAll()
{
    for (InitFunctionBase* func = g_initFunctions; func; func = func->m_next)
    {
        func->Run();
    }
}