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
};

