#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <string>
#include <map>


class TextureManager
{
public:
	TextureManager( Microsoft::WRL::ComPtr<ID3D11Device>& pDevice );
	std::shared_ptr< Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > 
		GetTexture( const std::wstring fileName );
private:
	typedef	std::map< const std::wstring,
		std::weak_ptr< Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > >
		map_type;
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	map_type textures;
};

