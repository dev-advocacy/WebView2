#pragma once

typedef struct ProfileInformation
{
    std::wstring  browserDirectory;
	std::wstring  userDataDirectory;

} ProfileInformation_t;

class CWebViewProfile
{
public:
	static ProfileInformation_t Profile();

//	std::wstring get_browserDirectory() { return m_browserDirectory; };
//	std::wstring get_userDataDirectory() { return m_userDataDirectory; };
//private:
//	std::wstring m_browserDirectory;
//	std::wstring m_userDataDirectory;
};

