#pragma once
#include "DirectXTK\dxerr.h"
#include <exception>
#include <string>

#define CHILI_GFX_EXCEPTION( hr,note ) GraphicsException( hr,note,_CRT_WIDE(__FILE__),__LINE__ )

class GraphicsException : public std::exception
{
public:
	GraphicsException( const std::wstring err = L"",const std::wstring dsc = L"",
		const std::wstring not = L"",const std::wstring loc = L"",HRESULT hr = 0xBADF00D );
	GraphicsException( HRESULT hr,const std::wstring not,const wchar_t* file,unsigned int line );
	inline virtual const char* what() const override
	{
		return whatString.c_str();
	}
	inline operator const char*( ) const
	{
		return what();
	}
	inline const std::wstring& GetError() const
	{
		return name;
	}
	inline const std::wstring& GetDescription() const
	{
		return description;
	}
	inline const std::wstring& GetNote() const
	{
		return note;
	}
	inline const std::wstring& GetLocation() const
	{
		return location;
	}
	const std::wstring  GetFullErrorMessage() const;
private:
	static std::wstring LookupDescription( HRESULT hr );
	std::string GenerateWhatString() const;
private:
	const HRESULT hr;
	const std::wstring name;
	const std::wstring description;
	const std::wstring note;
	const std::wstring location;
	const std::string  whatString;
};
