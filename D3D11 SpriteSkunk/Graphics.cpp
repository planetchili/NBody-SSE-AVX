#include "Graphics.h"
#include "GraphicsException.h"
#include <assert.h>
#include <wrl.h>

#pragma comment( lib,"d3d11.lib" )
#pragma comment( lib,"d3dcompiler.lib")

namespace wrl = Microsoft::WRL;

Graphics::Graphics( HWND hWnd )
	:
	hWnd( hWnd )
{
	assert( hWnd != nullptr );

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd,sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = INITIALSCREENWIDTH;
	sd.BufferDesc.Height = INITIALSCREENHEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL	FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT				numLevelsRequested = 1;
	D3D_FEATURE_LEVEL	FeatureLevelsSupported;
	HRESULT				hr;
	
	if( FAILED( hr = D3D11CreateDeviceAndSwapChain( 
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&FeatureLevelsRequested,
		numLevelsRequested,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		&FeatureLevelsSupported,
		&pImmediateContext ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating device and swap chain" );
	}

	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	if( FAILED( hr = pSwapChain->GetBuffer(
		0,
		__uuidof( ID3D11Texture2D ),
		(LPVOID*)pBackBuffer.GetAddressOf() ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Getting back buffer" );
	}

	if( FAILED( hr = pDevice->CreateRenderTargetView( 
		pBackBuffer.Get(),
		nullptr,
		&pRenderTargetView ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating render target view on backbuffer" );
	}

	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory( &descDepth,sizeof( descDepth ) );
	descDepth.Width = INITIALSCREENWIDTH;
	descDepth.Height = INITIALSCREENHEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	if( FAILED( hr = pDevice->CreateTexture2D( &descDepth,nullptr,&pDepthStencil ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating depth stencil texture" );
	}

	wrl::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV,sizeof( descDSV ) );
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	if( FAILED( hr = pDevice->CreateDepthStencilView( 
		pDepthStencil.Get(),
		&descDSV,
		&pDepthStencilView ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating depth stencil view");
	}

	pImmediateContext->OMSetRenderTargets( 1,pRenderTargetView.GetAddressOf(),pDepthStencilView.Get() );

	D3D11_VIEWPORT vp;
	vp.Width = INITIALSCREENWIDTH;
	vp.Height = INITIALSCREENHEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pImmediateContext->RSSetViewports( 1,&vp );

	pTextureManager = std::make_unique< TextureManager >( pDevice );
}

Graphics::~Graphics()
{}

void Graphics::EndFrame()
{
	HRESULT hr;
	if( FAILED( hr = pSwapChain->Present( 1,0 ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Presenting back buffer" );
	}
}

void Graphics::BeginFrame()
{
	pImmediateContext->ClearRenderTargetView(
		pRenderTargetView.Get(),
		DirectX::Colors::Black );
}