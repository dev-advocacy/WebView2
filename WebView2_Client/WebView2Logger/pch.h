// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define _WIN32_WINNT	0x0601
#define NTDDI_VERSION	0x0A000004

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

#include <wil/com.h>
#include <wil/resource.h>
#include <wil/result.h>
#include <wrl.h>

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
#include <boost/log/sinks/text_ipc_message_queue_backend.hpp>
#include <boost/log/utility/ipc/reliable_message_queue.hpp>
#include <boost/log/utility/ipc/object_name.hpp>


//namespaces

namespace	fs = std::filesystem;
namespace	logging = boost::log;
namespace	src = boost::log::sources;
namespace	attrs = boost::log::attributes;
namespace	sinks = boost::log::sinks;
namespace	expr = boost::log::expressions;
namespace	keywords = boost::log::keywords;

#endif //PCH_H
