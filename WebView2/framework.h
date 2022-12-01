#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <cstdlib>
#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <string_view>
#include <filesystem>
#include <map>
#include <functional>
#include <regex>
#include <chrono>
#include <future>

#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <d2d1_3.h>
#include <wininet.h>

#include <wincodec.h>

#include <atlbase.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlframe.h>
#include <atlsplit.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlddx.h>
#include <atlimage.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include <wil/com.h>
#include <wil/resource.h>
#include <wil/result.h>
#include <wrl.h>

// WebView
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>

//json
#include <cpprest/json.h>

// logs


#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


//namespaces

namespace	fs = std::filesystem;
namespace	logging = boost::log;
namespace	src = boost::log::sources;
namespace	attrs = boost::log::attributes;
namespace	sinks = boost::log::sinks;
namespace	expr = boost::log::expressions;
namespace	keywords = boost::log::keywords;


// custom messages
static constexpr UINT MSG_NAVIGATE = WM_APP + 123;
static constexpr UINT MSG_RUN_ASYNC_CALLBACK = WM_APP + 124;
static constexpr UINT MSG_NAVIGATE_CALLBACK = WM_APP + 125;
static constexpr UINT MSG_GETCOOKIES_CALLBACK = WM_APP + 126;
static constexpr UINT MSG_CALLBACK = WM_APP + 127;
static constexpr int TEXT_SIZE = 1024;
static constexpr int ERR_WEBVIEW_NOT_INSTALLED = -1024;
static constexpr int ERR_RESOURCE_NOT_FOUND = -1025;

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
