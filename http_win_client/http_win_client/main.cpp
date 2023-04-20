
#include "pch.h"

#include <iostream>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Web.Http.Filters.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.Threading.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;
using namespace Windows::Storage::Streams;

IAsyncOperation<InMemoryRandomAccessStream> GetDataAsync()
{
	try
	{
		HttpBaseProtocolFilter filter;
		HttpCookieManager cookieManager(filter.CookieManager());
		HttpClient httpClient(filter);

		
		

		Uri uri{ L"https://github.com" };
		auto httpMethod{ HttpMethod::Get() };

		HttpRequestMessage request{ httpMethod, uri };

		HttpResponseMessage response{ co_await httpClient.SendRequestAsync(request) };

		if (response && response.StatusCode() == HttpStatusCode::Ok)
		{
			IInputStream inputStream{ co_await response.Content().ReadAsInputStreamAsync() };
			InMemoryRandomAccessStream memoryStream;
			co_await RandomAccessStream::CopyAsync(inputStream, memoryStream);
			memoryStream.Seek(0);

			auto cc = cookieManager.GetCookies(uri);
			for (auto c : cc)
			{
				std::wcout << "cookie name:" << c.Name().c_str() << std::endl;
				std::wcout << "cookie value:" << c.Value().c_str() << std::endl;
			}
			co_return memoryStream;
		}
	}
	catch (hresult_error const& ex)
	{
		std::wcout << ex.message().c_str() << std::endl;
	}

}

int main()
{
	init_apartment();
	auto stream = GetDataAsync().get();
	std::wcout << stream.Size() << std::endl;
	getchar();
	return 0;
}
