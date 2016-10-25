#pragma once

//
// Base initialization function (for running on library init and so on)
//

class InitFunctionBase
{
protected:
    InitFunctionBase* m_next;

    int m_order;

public:
    InitFunctionBase(int order = 0);

    virtual void Run() = 0;

    void Register();

    static void RunAll();
};

//
// Initialization function that will be called around initialization of the plugin system.
//

class InitFunction : public InitFunctionBase
{
private:
    void(*m_function)();

public:
    InitFunction(void(*function)(), int order = 0)
        : InitFunctionBase(order)
    {
        m_function = function;

        Register();
    }

    virtual void Run()
    {
        m_function();
    }
};