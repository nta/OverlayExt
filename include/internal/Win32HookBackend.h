#pragma once

#include <internal/HookingBackend.h>

namespace Hooking::Internal
{
	class Win32HookBackend : public IHookAPIBackend
	{
	public:
		virtual ~Win32HookBackend() override = default;

		virtual void* LookupAPIStart(const char* moduleName, const char* functionName) override;
	};
}