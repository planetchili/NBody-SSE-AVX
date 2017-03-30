#include "TextureManager.h"
#include "GraphicsException.h"
#include "DirectXTK/Inc/WICTextureLoader.h"

namespace wrl = Microsoft::WRL;

TextureManager::TextureManager( Microsoft::WRL::ComPtr<ID3D11Device>& pDevice )
	:
	pDevice( pDevice )
{}

std::shared_ptr< wrl::ComPtr<ID3D11ShaderResourceView> > 
	TextureManager::GetTexture( const std::wstring fileName )
{
	std::shared_ptr< wrl::ComPtr<ID3D11ShaderResourceView> > ppTextureView;
	auto& ppTextureViewWeak = textures[fileName];

	if( ( ppTextureView = ppTextureViewWeak.lock() ) == nullptr )
	{
		// if living texture not in map
		ppTextureView = std::make_shared<wrl::ComPtr<ID3D11ShaderResourceView>>();
		HRESULT hr;
		if( FAILED( hr = DirectX::CreateWICTextureFromFile(
			pDevice.Get(),
			fileName.c_str(),
			nullptr,
			&*ppTextureView ) ) )
		{
			throw CHILI_GFX_EXCEPTION( hr,
				( L"Loading texture from bitmap.\nFilename: " + fileName ).c_str() );
		}
		ppTextureViewWeak = ppTextureView;
	}

	return ppTextureView;
}