#include <StdInc.h>
#include <AyriaExtension.h>

#include <memory>

#include <BaseHooking.h>
#include <ComHooking.h>

#include <d3d9.h>

#include "ayria.bin.h"

// TODO: make sure the globals do not break if there's multiple device interfaces with different parent calls

struct D3D9HookData
{
	ComPtr<IDirect3DTexture9> texture;

	D3D9HookData(IDirect3DDevice9* device)
	{
		device->CreateTexture(480, 272, 1, D3DUSAGE_DYNAMIC, D3DFMT_DXT5, D3DPOOL_DEFAULT, texture.GetAddressOf(), nullptr);

		D3DLOCKED_RECT lr;
		texture->LockRect(0, &lr, nullptr, D3DLOCK_DISCARD);
		
		memcpy(lr.pBits, data, sizeof(data));

		texture->UnlockRect(0);
	}
};

using D3D9HookDataRef = std::shared_ptr<D3D9HookData>;

static HRESULT(__stdcall* g_origEndScene)(IDirect3DDevice9* device);

HRESULT __stdcall EndSceneStub(IDirect3DDevice9* device)
{
	{
		Hooking::COM::RefContainer<D3D9HookDataRef> hookData(device);

		ComPtr<IDirect3DStateBlock9> stateBlock;
		device->CreateStateBlock(D3DSBT_ALL, stateBlock.GetAddressOf());

		stateBlock->Capture();

		device->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
		device->SetVertexShader(nullptr);
		device->SetPixelShader(nullptr);

		/*device->SetTransform(D3DTS_VIEW, nullptr);
		device->SetTransform(D3DTS_WORLD, nullptr);
		device->SetTransform(D3DTS_PROJECTION, nullptr);*/

		device->SetTexture(0, hookData.Get()->texture.Get());

		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);

		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_COLORVERTEX, FALSE);

		D3DMATRIX transform =
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};

		device->SetTransform(D3DTS_PROJECTION, &transform);
		device->SetTransform(D3DTS_VIEW, &transform);
		device->SetTransform(D3DTS_WORLD, &transform);

		device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

		struct
		{
			float x, y, z;
			float u, v;
		} v[6];

		v[0] = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f };
		v[1] = { -1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		v[2] = { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		v[3] = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f };
		v[4] = { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		v[5] = { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f };

		device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, v, sizeof(v[0]));

		stateBlock->Apply();
	}

	return g_origEndScene(device);
}

static void HookInterface(IDirect3DDevice9* pInterface)
{
	Hooking::COM::RefContainer<D3D9HookDataRef> hookData(std::make_shared<D3D9HookData>(pInterface));
	hookData.Attach<IDirect3DDevice9>(pInterface);

	Hooking::COM::ReplaceVmtFunction(pInterface, &IDirect3DDevice9::EndScene, EndSceneStub, &g_origEndScene);
}

static void HookInterface(IDirect3DDevice9Ex* pInterface)
{
	Hooking::COM::RefContainer<D3D9HookDataRef> hookData(std::make_shared<D3D9HookData>(pInterface));
	hookData.Attach<IDirect3DDevice9Ex>(pInterface);

	Hooking::COM::ReplaceVmtFunction(pInterface, &IDirect3DDevice9Ex::EndScene, EndSceneStub, &g_origEndScene);
}

static HRESULT(__stdcall* g_origCreateDevice)(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, IDirect3DDevice9** pDevice);

HRESULT __stdcall IDirect3D9_CreateDeviceStub(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, IDirect3DDevice9** pDevice)
{
	HRESULT hr = g_origCreateDevice(d3d9, adapter, deviceType, hFocusWindow, behaviorFlags, pParams, pDevice);

	if (SUCCEEDED(hr))
	{
		FuncTrace("Hooking IDirect3DDevice9 implementation\n");

		HookInterface(*pDevice);
	}

	return hr;
}

static HRESULT(__stdcall* g_origCreateDeviceEx)(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow,
												DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, D3DDISPLAYMODEEX* pMode, IDirect3DDevice9Ex** pDevice);

HRESULT __stdcall IDirect3D9_CreateDeviceExStub(IDirect3D9* d3d9, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow,
												DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pParams, D3DDISPLAYMODEEX* pMode, IDirect3DDevice9Ex** pDevice)
{
	HRESULT hr = g_origCreateDeviceEx(d3d9, adapter, deviceType, hFocusWindow, behaviorFlags, pParams, pMode, pDevice);

	if (SUCCEEDED(hr))
	{
		FuncTrace("Hooking IDirect3DDevice9Ex implementation\n");

		HookInterface(*pDevice);
	}

	return hr;
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

static void WINAPI SetWindowsHookExA_Nope(__in int idHook, __in HOOKPROC lpfn, __in_opt HINSTANCE hmod, __in DWORD dwThreadId)
{
	return;
}

static InitFunction initFunction([] ()
{
	InitStartEvent.Connect([] ()
	{
		Hooking::InstallAPIHook("user32.dll", "SetWindowsHookExA", SetWindowsHookExA_Nope);

		OverlayHookD3D9();
	});
});