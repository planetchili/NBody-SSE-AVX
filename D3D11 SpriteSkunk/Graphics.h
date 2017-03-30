#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <array>
#include <wrl.h>
#include <memory>
#include "DirectXTK/dxerr.h"
#include "DirectXTK/Inc/SpriteBatch.h"
#include "DirectXTK/Inc/CommonStates.h"
#include <assert.h>
#include <d3dcompiler.h>
#include "TextureManager.h"
#include "GraphicsException.h"

#define INITIALSCREENWIDTH 1280
#define INITIALSCREENHEIGHT 720

class Graphics
{
public:
	Graphics( HWND hWnd );
	void EndFrame();
	void BeginFrame();
	inline std::unique_ptr<DirectX::SpriteBatch> MakeSpriteBatch()
	{
		return std::make_unique<DirectX::SpriteBatch>( pImmediateContext.Get() );
	}
	/*Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> MakeTexture()
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = 128;
		desc.Height = 128;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		std::array<Color,128 * 128> image;
		image.fill( GREEN );
		for( int y = 0; y < 128; y++ )
		{
			for( int x = 0; x < 128; x++ )
			{
				const unsigned char alpha = min( (sq( x - 64 ) + sq( y - 64 )) / 32,255 );
				const Color c = image[y * 128 + x];
				image[y * 128 + x] = { 
					alpha,
					unsigned char( c.r * alpha / 255 ),
					unsigned char( c.g * alpha / 255 ),
					unsigned char( c.b * alpha / 255 ) };
			}
		}

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &image;
		initData.SysMemPitch = 128 * 4;
		initData.SysMemSlicePitch = 0;

		HRESULT hr;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
		if( FAILED( hr = pDevice->CreateTexture2D( &desc,&initData,&pTexture ) ) )
		{
			throw CHILI_GFX_EXCEPTION( hr,L"Creating texture" );
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		memset( &SRVDesc,0,sizeof( SRVDesc ) );
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		if( FAILED( hr = pDevice->CreateShaderResourceView( pTexture.Get(),&SRVDesc,&pTextureView ) ) )
		{
			throw CHILI_GFX_EXCEPTION( hr,L"Creating view on created texture" );
		}

		return pTextureView;
	}*/
	inline std::unique_ptr<DirectX::CommonStates> MakeStates()
	{
		return std::make_unique<DirectX::CommonStates>( pDevice.Get() );
	}
	inline TextureManager& GetTexMan()
	{
		return *pTextureManager;
	}
	Microsoft::WRL::ComPtr<ID3D11SamplerState> MakeSamplerState()
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
		HRESULT hr;
		if( FAILED( hr = pDevice->CreateSamplerState(
			&samplerDesc,
			&pSamplerState
			) ) )
		{
			throw CHILI_GFX_EXCEPTION( hr,L"Creating sampler state" );
		}

		return pSamplerState;
	}
	/*std::function<void()> GetShaderConfigurator()
	{
		return [ this ]()
		{
			Microsoft::WRL::ComPtr<ID3DBlob> pShaderBlob;
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
			HRESULT hr;
			if( FAILED( hr = D3DCompileFromFile(
				L"Quadro.hlsl",
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"Quadro",
				"ps_5_0",
				0,
				0,
				&pShaderBlob,
				&pErrorBlob ) ) )
			{
				std::wstring note = L"Compiling hlsl source file";
				if( pErrorBlob )
				{
					const std::string compilerErrors = (char*)pErrorBlob->GetBufferPointer();
					note += L"\nCompiler Output: " 
						 + std::wstring( compilerErrors.begin(),compilerErrors.end() );
				}
				else
				{
					note += L"\nNo compiler output";
				}
				throw CHILI_GFX_EXCEPTION( hr,note );
			}

			Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
			if( FAILED( hr = pDevice->CreatePixelShader(
				pShaderBlob->GetBufferPointer(),
				pShaderBlob->GetBufferSize(),
				nullptr,
				&pPixelShader ) ) )
			{
				throw CHILI_GFX_EXCEPTION( hr,L"Creating pixel shader" );
			}

			pImmediateContext->PSSetShader( pPixelShader.Get(),nullptr,0 );
		};
	}*/
	~Graphics();
private:
	const HWND hWnd;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device>			pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		pImmediateContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	pRenderTargetView;
	std::unique_ptr< TextureManager >				pTextureManager;
};