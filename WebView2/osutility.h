#pragma once
// Returns Program Files directory x86.

namespace os
{
	class utility
	{
		public:
			enum imagesize 
			{
				size24,
				size32
			};

		public:
			static void GetImages(imagesize isize);
			static std::wstring GetimageFolder(imagesize isize);
			static std::wstring GetSvgFolder();
			static std::wstring BoolToString(BOOL value);
			static std::wstring EncodeQuote(std::wstring raw);
			static std::wstring SecondsToString(UINT32 time);
			static std::wstring CookieToString(ICoreWebView2Cookie* cookie);		
		private:
			static std::wstring GetLangStringFromLangId(DWORD dwLangID_i, bool returnShortCode);
	};
	class svg
	{
	public:
		void SaveImage(std::wstring str_sourcefile, std::wstring str_destfile, D2D1_SIZE_F imagesize, FLOAT factor);
		HRESULT Init2D2();
		HRESULT GetSvgDocumentSize(std::wstring str_sourcefile, D2D1_SIZE_F& imagesize);

	private:
		HRESULT DrawImage(wil::com_ptr<ID2D1RenderTarget> pRT, std::wstring str_sourcefile, D2D1_SIZE_F imagesize, FLOAT factor);
		void ExportImage(std::wstring str_destfile, D2D1_SIZE_F imagesize, FLOAT factor);
	private:
		
		wil::com_ptr<ID2D1Factory>			m_pD2DFactory = nullptr;
		wil::com_ptr<IWICBitmap>			m_pWICBitmap = nullptr;
		wil::com_ptr<IWICImagingFactory>	m_pWICFactory = nullptr;
	};
};
