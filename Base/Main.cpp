#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cassert>
#include <DirectXMath.h>
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
static           ID3D11Buffer*               g_vertexBuffer       = nullptr;
static           ID3D11Buffer*               g_indexBuffer        = nullptr;
static           ID3D11InputLayout*          g_inputLayout        = nullptr;
static           ID3D10Blob*                 pVSCode              = nullptr;
static           ID3D10Blob*                 pPSCode              = nullptr;
/*
below will be modified according to app's needs
*******************************************************************************1
*/
struct Vertex
{
	Vertex() { }
	Vertex(float x, float y, float z, float r, float g, float b)
		: pos(x, y, z), color(r,g,b) { }
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

void CreateAPPSpecificResource() {

}

void SetAppSpecificCommand()
{
   	g_deviceContext->Draw(3, 0);
}

void CreateIndexBuffer()
{

}

void CreateVertexBufferAndLayout()
{
    Vertex v[] =
	{
		Vertex(-0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f), //Center Point
		Vertex( 0.0f,  0.5f, 1.0f, 0.0f, 1.0f, 0.0f), //Center Point
		Vertex( 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f), //Center Point
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(v);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;

	HRESULT hr = g_device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &g_vertexBuffer);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};
    UINT numElements = ARRAYSIZE(layout);
    hr = g_device->CreateInputLayout(layout, numElements, pVSCode->GetBufferPointer(), pVSCode->GetBufferSize(), &g_inputLayout);
    assert(SUCCEEDED(hr));


}

/***********************************************************************/
void render() 
{

    HRESULT hr = S_OK;
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

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	g_deviceContext->ClearRenderTargetView(g_renderTargetView, bgcolor);
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
										NULL, //FLAGS FOR RUNTIME LAYERS
										NULL, //FEATURE LEVELS ARRAY
										0, //# OF FEATURE LEVELS IN ARRAY
										D3D11_SDK_VERSION,
										&scd, //Swapchain description
										&g_swapchain, //Swapchain Address
										&g_device, //Device Address
										NULL, //Supported feature level
										&g_deviceContext); //Device Context Address
    assert(SUCCEEDED(hr));
    ID3D11Texture2D* backBuffer = nullptr;
    hr = g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));                                  
    assert(SUCCEEDED(hr));
    g_device->CreateRenderTargetView(backBuffer, NULL, &g_renderTargetView);
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
    g_device->CreateTexture2D(&depthStencilBufferDesc, NULL, &g_depthStencialBuffer);
    assert(SUCCEEDED(hr));

    /* createDepthStencialView */
	g_device->CreateDepthStencilView(g_depthStencialBuffer, NULL, &g_depthStencialView);
    assert(SUCCEEDED(hr));
}

void CreateVertexShader() 
{
    HRESULT hr = D3DReadFileToBlob(L"..\\x64\\Debug\\vertexshader.cso", &pVSCode);
    assert(SUCCEEDED(hr));
    hr = g_device->CreateVertexShader(pVSCode->GetBufferPointer(), pVSCode->GetBufferSize(), NULL, &g_VS);
    assert(SUCCEEDED(hr));
}

void CreatePixelShader()
{
    HRESULT hr = D3DReadFileToBlob(L"..\\x64\\Debug\\pixelshader.cso", &pPSCode);
    assert(SUCCEEDED(hr));
    hr = g_device->CreatePixelShader(pPSCode->GetBufferPointer(), pPSCode->GetBufferSize(), NULL, &g_PS);
    assert(SUCCEEDED(hr));
    // Pixel shader to render the scene containing lod value
}

void Initialize()
{
    CreateOnceAndForAllResource();
    CreateVertexShader();
    CreatePixelShader();
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
                                L"BaseLine",                    // Window text
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