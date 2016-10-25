#include <StdInc.h>
#include <AyriaExtension.h>

#include <BaseHooking.h>

#include <d3d9.h>

static IDirect3D9*(WINAPI* g_origDirect3DCreate9)(UINT);
static HRESULT(WINAPI* g_origDirect3DCreate9Ex)(UINT, IDirect3D9Ex**);

static IDirect3D9* WINAPI Direct3DCreate9Wrap(UINT sdkVersion)
{
	FuncTrace("Called from %p.\n", _ReturnAddress());

	IDirect3D9* pInterface = g_origDirect3DCreate9(sdkVersion);

	return pInterface;
}

static HRESULT WINAPI Direct3DCreate9ExWrap(UINT sdkVersion, IDirect3D9Ex** pInterface)
{
	FuncTrace("Called from %p.\n", _ReturnAddress());

	HRESULT hr = g_origDirect3DCreate9Ex(sdkVersion, pInterface);

	return hr;
}


static void OverlayHookD3D9()
{
	Hooking::InstallAPIHook("d3d9.dll", "Direct3DCreate9", Direct3DCreate9Wrap, &g_origDirect3DCreate9);
	Hooking::InstallAPIHook("d3d9.dll", "Direct3DCreate9Ex", Direct3DCreate9ExWrap, &g_origDirect3DCreate9Ex);
}

static InitFunction initFunction([] ()
{
	InitStartEvent.Connect([] ()
	{
		OverlayHookD3D9();
	});
});