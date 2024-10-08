#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cassert>
#include <DirectXMath.h>
#include "Util.h"
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DXGI.lib")


/************************************************************************/
//params

static constexpr UINT32                      g_WIDTH              = 800;
static constexpr UINT32                      g_HEIGHT             = 800;
static constexpr D3D_DRIVER_TYPE             g_DriverType         = D3D_DRIVER_TYPE_HARDWARE;
static           HWND                        g_hwnd               = nullptr;
static           IDXGISwapChain*             g_swapchain          = nullptr;
static           ID3D11Device*               g_device             = nullptr;
static           ID3D11DeviceContext*        g_deviceContext      = nullptr;
static           ID3D11RenderTargetView*     g_renderTargetView   = nullptr;

static           ID3D11RasterizerState*      g_rasterizerState    = nullptr ;
static           ID3D11DepthStencilState*    g_depthStencialState = nullptr;
static           ID3D11DepthStencilView*     g_depthStencialView  = nullptr ;
static           ID3D11Texture2D*            g_depthStencialBuffer= nullptr ;
static           ID3D11VertexShader*         g_VS                 = nullptr;
static           ID3D11PixelShader*          g_PS                 = nullptr;
static           ID3D11ComputeShader*        g_CS = nullptr;

static           ID3D11Buffer*               g_vertexBuffer       = nullptr;
static           ID3D11Buffer*               g_indexBuffer        = nullptr;
static           ID3D11InputLayout*          g_inputLayout        = nullptr;
static           ID3D10Blob*                 pVSCode              = nullptr;
static           ID3D10Blob*                 pPSCode              = nullptr;
static           ID3D10Blob*                 pCSCode              = nullptr;
static           ID3D11Texture2D*            m_backBuffer         = nullptr;

// UAV parameters
static           ID3D11Texture2D* m_TextureA = nullptr;
static           ID3D11Texture2D* m_TextureB = nullptr;
static           ID3D11Texture2D* m_TextureC = nullptr;

static           ID3D11RenderTargetView*    g_rtvA = nullptr;
static           ID3D11RenderTargetView*    g_rtvB = nullptr;
static           ID3D11RenderTargetView* g_rtvC = nullptr;

static           ID3D11ShaderResourceView*  g_srvA = nullptr;
static           ID3D11ShaderResourceView*  g_srvB = nullptr;
static           ID3D11ShaderResourceView* g_srvC = nullptr;

static           ID3D11UnorderedAccessView* g_uavC = nullptr;

/*
below will be modified according to app's needs
*******************************************************************************1
*/
// Mip Count
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
const UINT NumMips = 6;
float Colors[NumMips][4] =
{
	{1.f, 0.f, 0.f, 1.f},  // LOD0 - Red
	{1.f, 0.f, 1.f, 1.f},  // LOD1 - Magenta
	{0.f, 0.f, 1.f, 1.f},  // LOD2 - Blue
	{0.f, 1.f, 1.f, 1.f},  // LOD3 - Cyan
	{0.f, 1.f, 0.f, 1.f},  // LOD4 - Green
	{1.f, 1.f, 0.f, 1.f},  // LOD5 - Yellow
};
typedef struct Vertex
{
	DirectX::XMFLOAT2	tex;
	float		        lod;
	DirectX::XMFLOAT3	pos;
};
// Vertex data
Vertex vertex[] =
{
	// TexCoord   LOD   Position

	// LOD 0 Tri
	{ XMFLOAT2(0.5f, 0.0f), 0.0f, XMFLOAT3(-0.5f,  1.00f,  0.5f) },
	{ XMFLOAT2(1.0f, 1.0f), 0.0f, XMFLOAT3(0.0f,  0.34f,  0.5f) },
	{ XMFLOAT2(0.0f, 1.0f), 0.0f, XMFLOAT3(-1.0f,  0.34f,  0.5f) },

	// LOD 1 Tri
	{ XMFLOAT2(0.5f, 0.0f), 1.0f, XMFLOAT3(0.5f,  1.00f,  0.5f) },
	{ XMFLOAT2(1.0f, 1.0f), 1.0f, XMFLOAT3(1.0f,  0.34f,  0.5f) },
	{ XMFLOAT2(0.0f, 1.0f), 1.0f, XMFLOAT3(0.0f,  0.34f,  0.5f) },

	// LOD 2 Tri
	{ XMFLOAT2(0.5f, 0.0f), 2.0f, XMFLOAT3(-0.5f,  0.33f,  0.5f) },
	{ XMFLOAT2(1.0f, 1.0f), 2.0f, XMFLOAT3(0.0f, -0.33f,  0.5f) },
	{ XMFLOAT2(0.0f, 1.0f), 2.0f, XMFLOAT3(-1.0f, -0.33f,  0.5f) },

	// LOD 3 Tri
	{ XMFLOAT2(0.5f, 0.0f), 3.0f, XMFLOAT3(0.5f,  0.33f,  0.5f) },
	{ XMFLOAT2(1.0f, 1.0f), 3.0f, XMFLOAT3(1.0f, -0.33f,  0.5f) },
	{ XMFLOAT2(0.0f, 1.0f), 3.0f, XMFLOAT3(0.0f, -0.33f,  0.5f) },

	// LOD 4 Tri
	{ XMFLOAT2(0.5f, 0.0f), 4.0f, XMFLOAT3(-0.5f, -0.34f,  0.5f) },
	{ XMFLOAT2(1.0f, 1.0f), 4.0f, XMFLOAT3(0.0f, -1.00f,  0.5f) },
	{ XMFLOAT2(0.0f, 1.0f), 4.0f, XMFLOAT3(-1.0f, -1.00f,  0.5f) },

	// LOD 5 Tri
	{ XMFLOAT2(0.5f, 0.0f), 5.0f, XMFLOAT3(0.5f, -0.34f,  0.5f) },
	{ XMFLOAT2(1.0f, 1.0f), 5.0f, XMFLOAT3(1.0f, -1.00f,  0.5f) },
	{ XMFLOAT2(0.0f, 1.0f), 5.0f, XMFLOAT3(0.0f, -1.00f,  0.5f) },
};

void CreateAPPSpecificResource() {
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC  Desc;

	// CreateTexutreA, TextureB for computer shader.
	ZeroMemory(&Desc, sizeof(Desc));
	Desc.ArraySize = 1;
	Desc.MipLevels = 1;
	Desc.Width = g_WIDTH;
	Desc.Height = g_HEIGHT;
	Desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	Desc.MiscFlags = 0;
	Desc.BindFlags = (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	hr = g_device->CreateTexture2D(&Desc, NULL, &m_TextureA);
	assert(SUCCEEDED(hr));
	hr = g_device->CreateTexture2D(&Desc, NULL, &m_TextureB);
	assert(SUCCEEDED(hr));

	hr = g_device->CreateTexture2D(&Desc, NULL, &m_TextureC);
	assert(SUCCEEDED(hr));
	
	hr = g_device->CreateRenderTargetView(m_TextureA, nullptr, &g_rtvA);
	assert(SUCCEEDED(hr));

	hr = g_device->CreateRenderTargetView(m_TextureB, nullptr, &g_rtvB);
	assert(SUCCEEDED(hr));
	hr = g_device->CreateRenderTargetView(m_TextureC, nullptr, &g_rtvC);
	assert(SUCCEEDED(hr));
	hr = g_device->CreateShaderResourceView(m_TextureA, nullptr, &g_srvA);
	assert(SUCCEEDED(hr));

	hr = g_device->CreateShaderResourceView(m_TextureB, nullptr, &g_srvB);
	assert(SUCCEEDED(hr));

	
	hr = g_device->CreateShaderResourceView(m_TextureC, nullptr, &g_srvC);
	assert(SUCCEEDED(hr));

	hr = g_device->CreateUnorderedAccessView(m_TextureC, nullptr, &g_uavC);
	assert(SUCCEEDED(hr));
}

ID3D11SamplerState* pSamplerState1;

void SetAppSpecificCommand()
{
	#define FLT_MAX          3.402823466e+38F        // max value
	HRESULT hr = S_OK;


	D3D11_SAMPLER_DESC  sDesc;
	ZeroMemory(&sDesc, sizeof(sDesc));
	sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	hr = g_device->CreateSamplerState(&sDesc, &pSamplerState1);
	assert(SUCCEEDED(hr));
	g_deviceContext->PSSetSamplers(0, 1, &pSamplerState1);

	g_deviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
    ID3D11RasterizerState* pRState = NULL;
    D3D11_RASTERIZER_DESC rasterizer;
    rasterizer.FillMode              = D3D11_FILL_SOLID;
    rasterizer.CullMode              = D3D11_CULL_NONE;
    rasterizer.FrontCounterClockwise = TRUE;
    rasterizer.DepthBias             = 0;
    rasterizer.DepthBiasClamp        = 0;
    rasterizer.SlopeScaledDepthBias  = 0;
    rasterizer.DepthClipEnable       = FALSE;
    rasterizer.ScissorEnable         = FALSE;
    rasterizer.MultisampleEnable     = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;

    hr = g_device->CreateRasterizerState(&rasterizer, &pRState);
    assert(SUCCEEDED(hr));
    g_deviceContext->RSSetState(pRState);

	g_deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &g_renderTargetView, g_depthStencialView, 0, 0, NULL, NULL);
	g_deviceContext->PSSetShaderResources(0, 1, &g_srvC);
	g_deviceContext->DrawIndexed(18, 0, 0);
	ID3D11ShaderResourceView* temp = NULL;
	g_deviceContext->PSSetShaderResources(0, 1, &temp);

}

void CreateIndexBuffer()
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	// Create index buffer
	WORD indices[] =
	{
		0,1,2,
		3,4,5,
		6,7,8,
		9,10,11,
		12,13,14,
		15,16,17
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 18;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;

	HRESULT hr = g_device->CreateBuffer(&bd, &InitData, &g_indexBuffer);

}




void CreateVertexBufferAndLayout()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertex);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = vertex;

	HRESULT hr = g_device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &g_vertexBuffer);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC layout[] =
	{
	{ "texcoord", 0,	DXGI_FORMAT_R32G32_FLOAT,	0,	0,								D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "lod",      0,	DXGI_FORMAT_R32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "position", 0,	DXGI_FORMAT_R32G32B32_FLOAT,0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
    UINT numElements = ARRAYSIZE(layout);
    hr = g_device->CreateInputLayout(layout, numElements, pVSCode->GetBufferPointer(), pVSCode->GetBufferSize(), &g_inputLayout);
    assert(SUCCEEDED(hr));
}



/***********************************************************************/
void render() 
{

    HRESULT hr = S_OK;
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	g_deviceContext->ClearRenderTargetView(g_renderTargetView, bgcolor);
	g_deviceContext->ClearRenderTargetView(g_rtvC, bgcolor);


	float bgcolorA[] = { 0.0f, 0.0f, 0.5f, 1.0f };
	g_deviceContext->ClearRenderTargetView(g_rtvA, bgcolorA);
	float bgcolorB[] = { 0.5f, 0.0f, 0.0f, 1.0f };
	g_deviceContext->ClearRenderTargetView(g_rtvB, bgcolorB);
	g_deviceContext->CSSetShaderResources(0, 1, &g_srvA);
	g_deviceContext->CSSetShaderResources(1, 1, &g_srvB);
	g_deviceContext->CSSetUnorderedAccessViews(0, 1, &g_uavC, NULL);
	g_deviceContext->CSSetShader(g_CS, NULL, 0);
	g_deviceContext->Dispatch(50, 50, 1);
	ID3D11UnorderedAccessView* temp = NULL;
	g_deviceContext->CSSetUnorderedAccessViews(0, 1, &temp, NULL);


    /* SetRenderTargets */
	g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencialView);
    assert(SUCCEEDED(hr));
    /* Create depth stencil state for deviceContext*/
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    hr = g_device->CreateDepthStencilState(&depthstencildesc, &g_depthStencialState);
    assert(SUCCEEDED(hr));

    /* resteriation setup, Create the Viewport */
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = g_WIDTH;
	viewport.Height = g_HEIGHT;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	g_deviceContext->RSSetViewports(1, &viewport);
    assert(SUCCEEDED(hr));

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	hr = g_device->CreateRasterizerState(&rasterizerDesc, &g_rasterizerState);
    assert(SUCCEEDED(hr));


	g_deviceContext->ClearDepthStencilView(g_depthStencialView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	g_deviceContext->OMSetDepthStencilState(g_depthStencialState, 0);

	g_deviceContext->IASetInputLayout(g_inputLayout);
	g_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_deviceContext->VSSetShader(g_VS, NULL, 0);
	g_deviceContext->PSSetShader(g_PS, NULL, 0);
	g_deviceContext->RSSetState(g_rasterizerState);
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_deviceContext->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

	// Set index buffer
	g_deviceContext->IASetIndexBuffer(g_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	g_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    SetAppSpecificCommand();

	g_swapchain->Present(1, NULL);
}

void CreateOnceAndForAllResource()
{
    /**
     * @brief  
       craete swapchain, device, contextDevice
     */
    DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferDesc.Width = g_WIDTH;
	scd.BufferDesc.Height = g_HEIGHT;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = g_hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(	NULL, //IDXGI Adapter
										g_DriverType,
										NULL, //FOR SOFTWARE DRIVER TYPE
		D3D11_CREATE_DEVICE_DEBUG, //FLAGS FOR RUNTIME LAYERS
										NULL, //FEATURE LEVELS ARRAY
										0, //# OF FEATURE LEVELS IN ARRAY
										D3D11_SDK_VERSION,
										&scd, //Swapchain description
										&g_swapchain, //Swapchain Address
										&g_device, //Device Address
										NULL, //Supported feature level
										&g_deviceContext); //Device Context Address
    assert(SUCCEEDED(hr));
    hr = g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_backBuffer));                                  
    assert(SUCCEEDED(hr));
    hr = g_device->CreateRenderTargetView(m_backBuffer, NULL, &g_renderTargetView);
    assert(SUCCEEDED(hr));

    /* Describe our Depth/Stencil Buffer */
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	depthStencilBufferDesc.Width = g_WIDTH;
	depthStencilBufferDesc.Height = g_HEIGHT;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;
    hr = g_device->CreateTexture2D(&depthStencilBufferDesc, NULL, &g_depthStencialBuffer);
    assert(SUCCEEDED(hr));

    /* createDepthStencialView */
	hr = g_device->CreateDepthStencilView(g_depthStencialBuffer, NULL, &g_depthStencialView);
    assert(SUCCEEDED(hr));
}

void CreateVertexShader() 
{
    HRESULT hr = D3DReadFileToBlob(L"..\\x64\\Debug\\VertexShader.cso", &pVSCode);
	if (!SUCCEEDED(hr))
	{
		 hr = D3DReadFileToBlob(L"..\\x64\\Release\\VertexShader.cso", &pVSCode);
	}
	if (!SUCCEEDED(hr))
	{
		hr = D3DReadFileToBlob(L".\\VertexShader.cso", &pVSCode);
	}
    assert(SUCCEEDED(hr));
    hr = g_device->CreateVertexShader(pVSCode->GetBufferPointer(), pVSCode->GetBufferSize(), NULL, &g_VS);
    assert(SUCCEEDED(hr));
}

void CreatePixelShader()
{
    HRESULT hr = D3DReadFileToBlob(L"..\\x64\\Debug\\PixelShader.cso", &pPSCode);
	if (!SUCCEEDED(hr)) {
		hr = D3DReadFileToBlob(L"..\\x64\\Release\\PixelShader.cso", &pPSCode);
	}
	if (!SUCCEEDED(hr)) {
		hr = D3DReadFileToBlob(L".\\PixelShader.cso", &pPSCode);
	}
    assert(SUCCEEDED(hr));
    hr = g_device->CreatePixelShader(pPSCode->GetBufferPointer(), pPSCode->GetBufferSize(), NULL, &g_PS);
    assert(SUCCEEDED(hr));
    // Pixel shader to render the scene containing lod value
}

void CreateComputerShader()
{
	HRESULT hr = D3DReadFileToBlob(L"..\\x64\\Debug\\ComputerShader.cso", &pCSCode);
	if (!SUCCEEDED(hr)) {
		hr = D3DReadFileToBlob(L"..\\x64\\Release\\ComputerShader.cso", &pCSCode);
	}
	if (!SUCCEEDED(hr)) {
		hr = D3DReadFileToBlob(L".\\ComputerShader.cso", &pCSCode);
	}
	assert(SUCCEEDED(hr));
	hr = g_device->CreateComputeShader(pCSCode->GetBufferPointer(), pCSCode->GetBufferSize(), NULL, &g_CS);
	assert(SUCCEEDED(hr));
	// Pixel shader to render the scene containing lod value
}

void Initialize()
{
    CreateOnceAndForAllResource();
    CreateVertexShader();
    CreatePixelShader();
	CreateComputerShader();

    CreateVertexBufferAndLayout();
    CreateIndexBuffer();
    CreateAPPSpecificResource();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void CreateWindows(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"BaseLine For Develop Other Program";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    g_hwnd = CreateWindowEx(
                                0,                              // Optional window styles.
                                CLASS_NAME,                     // Window class
                                L"Reference BaseLine",                    // Window text
                                WS_OVERLAPPEDWINDOW,            // Window style

                                // Size and position
								0, 0, g_WIDTH, g_HEIGHT,

                                NULL,       // Parent window    
                                NULL,       // Menu
                                hInstance,  // Instance handle
                                NULL        // Additional application data
                                );
    ShowWindow(g_hwnd, nCmdShow);
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{


    CreateWindows(hInstance, hPrevInstance, pCmdLine, nCmdShow);
	Initialize();
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        render();
    }
    return 0;
}
