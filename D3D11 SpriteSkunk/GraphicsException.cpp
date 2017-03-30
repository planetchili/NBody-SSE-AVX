#include "GraphicsException.h"
#include <array>


GraphicsException::GraphicsException( const std::wstring err,const std::wstring dsc,
	const std::wstring not,const std::wstring loc,HRESULT hr )
	:
	hr( hr ),
	name( err ),
	description( dsc ),
	note( not ),
	location( loc ),
	whatString( GenerateWhatString() )
{}

GraphicsException::GraphicsException( HRESULT hr,const std::wstring not,const wchar_t* file,unsigned int line )
	:
	GraphicsException(
	DXGetErrorString( hr ),
	LookupDescription( hr ),
	not,
	std::wstring( L"Line [" ) + std::to_wstring( line ) + L"] in " + std::wstring( file ),
	hr )
{}

inline const std::wstring GraphicsException::GetFullErrorMessage() const
{
	const std::wstring empty = L"";
	return    ( !name.empty() ? std::wstring( L"Error: " ) + name + L"\n"
		: empty )
		+ ( !description.empty() ? std::wstring( L"Description: " ) + description + L"\n"
		: empty )
		+ ( !note.empty() ? std::wstring( L"Note: " ) + note + L"\n"
		: empty )
		+ ( !location.empty() ? std::wstring( L"Location: " ) + location
		: empty );
}

inline std::wstring GraphicsException::LookupDescription( HRESULT hr )
{
	std::array<wchar_t,512> wideDescription;
	DXGetErrorDescription( hr,wideDescription.data(),wideDescription.size() );
	return wideDescription.data();
}

inline std::string GraphicsException::GenerateWhatString() const
{
	const std::wstring temp = GetFullErrorMessage();
	return std::string( temp.begin(),temp.end() );
}