#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <string>
#include "TextureManager.h"
#include "Graphics.h"
#include "Vec2.h"

class Sprite
{
public:
	Sprite( const std::wstring& path,const RECT& srcRect,float scale,Vec2 org,TextureManager& tman )
		:
		srcRect( srcRect ),
		scale( scale ),
		tex( tman.GetTexture( path ) )
	{
		// test rect against texture dimensions
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex;
			( *tex )->GetResource( (ID3D11Resource**)pTex.GetAddressOf() );
			D3D11_TEXTURE2D_DESC desc;
			pTex->GetDesc( &desc );
			if( srcRect.left > int( desc.Width ) || srcRect.left < 0 || srcRect.right > int( desc.Width ) || srcRect.right < 0 ||
				srcRect.top > int( desc.Height ) || srcRect.top < 0 || srcRect.bottom > int( desc.Height ) || srcRect.bottom < 0 )
			{
				const auto errorString = std::string( "Loading Sprite\nBad source rect\nRect: " ) +
					std::to_string( srcRect.top ) + "," + std::to_string( srcRect.bottom ) + "," + std::to_string( srcRect.left ) + "," + std::to_string( srcRect.right ) +
					std::string( "\nTex:\n" ) +
					std::to_string( desc.Width ) + "," + std::to_string( desc.Height );
				throw std::exception( errorString.c_str() );
			}
		}

		origin = { float( srcRect.left + org.x ),float( srcRect.top + org.y ) };
	}
	Sprite( Sprite&& donor )
		:
		srcRect( donor.srcRect ),
		origin( donor.origin ),
		scale( donor.scale ),
		tex( std::move( donor.tex ) )
	{}
	void Draw( DirectX::SpriteBatch& sb,Vec2 pos,float theta = 0.0f ) const
	{
		sb.Draw( tex->Get(),DirectX::XMFLOAT2( pos.x,pos.y ),&srcRect,DirectX::Colors::White,
			theta,origin,scale );
	}
private:
	Sprite( const Sprite& ) = delete;
	void operator=( const Sprite ) = delete;
private:
	std::shared_ptr< Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > tex;
	DirectX::XMFLOAT2 origin;
	RECT srcRect;
	float scale;
};