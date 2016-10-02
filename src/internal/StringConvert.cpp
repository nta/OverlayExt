#include <StdInc.h>
#include <codecvt>

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_converter;

std::string StringToNarrow(const std::wstring& wide)
{
	return m_converter.to_bytes(wide);
}

std::wstring StringToWide(const std::string& narrow)
{
	return m_converter.from_bytes(narrow);
}