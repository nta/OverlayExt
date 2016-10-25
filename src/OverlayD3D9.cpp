#include <StdInc.h>
#include <AyriaExtension.h>

#include <BaseHooking.h>
#include <ComHooking.h>

#include <d3d9.h>

// TODO: make sure this does not break if there's multiple device interfaces with different parent calls
static HRESULT(__stdcall* g_origCreateDevice)(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, IDirect3DDevice9** pDevice);

HRESULT __stdcall IDirect3D9_CreateDeviceStub(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, IDirect3DDevice9** pDevice)
{
	return g_origCreateDevice(d3d9, adapter, deviceType, hFocusWindow, behaviorFlags, pParams, pDevice);
}

static HRESULT(__stdcall* g_origCreateDeviceEx)(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow,
												DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, D3DDISPLAYMODEEX* pMode, IDirect3DDevice9Ex** pDevice);

HRESULT __stdcall IDirect3D9_CreateDeviceExStub(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow,
												DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, D3DDISPLAYMODEEX* pMode, IDirect3DDevice9Ex** pDevice)
{
	return g_origCreateDeviceEx(d3d9, adapter, deviceType, hFocusWindow, behaviorFlags, pParams, pMode, pDevice);
}

static void HookInterface(IDirect3D9* pInterface)
{
	Hooking::COM::ReplaceVmtFunction(pInterface, &IDirect3D9::CreateDevice, IDirect3D9_CreateDeviceStub, &g_origCreateDevice);
}

static void HookInterface(IDirect3D9Ex* pInterface)
{
	// stock Direct3D should be inheriting Ex from the regular device - some hook implementations may not, replace if this is needed
	//Hooking::COM::ReplaceVmtFunction(pInterface, &IDirect3D9::CreateDevice, IDirect3D9_CreateDeviceStub, &g_origCreateDevice);
	Hooking::COM::ReplaceVmtFunction(pInterface, &IDirect3D9Ex::CreateDevice, IDirect3D9_CreateDeviceStub, &g_origCreateDevice);
	Hooking::COM::ReplaceVmtFunction(pInterface, &IDirect3D9Ex::CreateDeviceEx, IDirect3D9_CreateDeviceExStub, &g_origCreateDeviceEx);
}

static IDirect3D9*(WINAPI* g_origDirect3DCreate9)(UINT);
static HRESULT(WINAPI* g_origDirect3DCreate9Ex)(UINT, IDirect3D9Ex**);

static IDirect3D9* WINAPI Direct3DCreate9Wrap(UINT sdkVersion)
{
	FuncTrace("Called from %p.\n", _ReturnAddress());

	IDirect3D9* pInterface = g_origDirect3DCreate9(sdkVersion);

	// initialize hooks if needed
	if (pInterface)
	{
		HookInterface(pInterface);
	}

	return pInterface;
}

static HRESULT WINAPI Direct3DCreate9ExWrap(UINT sdkVersion, IDirect3D9Ex** pInterface)
{
	FuncTrace("Called from %p.\n", _ReturnAddress());

	HRESULT hr = g_origDirect3DCreate9Ex(sdkVersion, pInterface);

	// if succeeded, initialize hooks
	if (SUCCEEDED(hr))
	{
		HookInterface(*pInterface);
	}

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