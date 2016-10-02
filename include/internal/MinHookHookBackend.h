#pragma once

#include <internal/HookingBackend.h>

namespace Hooking::Internal
{
	class MinHookHookBackend : public IHookTrampolineBackend
	{
	public:
		virtual ~MinHookHookBackend() override = default;

		virtual bool InstallTrampolineHook(void* hookAddress, void* replacementFunction, void** originalFunction);
	};
}