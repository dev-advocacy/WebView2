# Win32 sample app

This sample, WebView2APISample, embeds a WebView2 control within a Win32 application.

This sample is built as a Win32 Visual Studio 2022 project.
It uses C++ in the native environment together with HTML/CSS/JavaScript in the WebView2 environment.
This sample showcases many of WebView2's event handlers and API methods that allow a native Win32 application to directly interact with a WebView, and vice versa.

- Sample name: WebViewSolution
- Solution file: WebViewSolution.sln (located in the parent directory, \SampleApps\WebView2WTL.Sample)
- Project name in Solution Explorer: WebViewSolution


## Prerequisites
Visual Studio 2022 with the Desktop Development with C++ workload installed.
vcpkg

## To build and run the sample

1. Install the vcpkg component from the Visual Studio Installer or from the GitHub repository.
 https://github.com/Microsoft/vcpkg.git
 https://vcpkg.io/en/getting-started

2. Use vcpkg integrate install to integrate vcpkg with Visual Studio.
3. Open Visual Studio 2022 and select File > Open > Project/Solution.
4. Navigate to the directory that contains the sample and open the solution file (WebViewSolution.sln).
5. Press F5 to build and run the sample.




[![Windows Build/Release](https://github.com/dev-advocacy/WebView2/actions/workflows/windowsbuild.yml/badge.svg)](https://github.com/dev-advocacy/WebView2/actions/workflows/windowsbuild.yml)
