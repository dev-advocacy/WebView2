

# Win32 sample app

This sample, WebView2APISample, embeds a WebView2 control within a Win32 application.

[![Windows Build/Release](https://github.com/dev-advocacy/WebView2/actions/workflows/windowsbuild.yml/badge.svg)](https://github.com/dev-advocacy/WebView2/actions/workflows/windowsbuild.yml)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Overview
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

Install the vcpkg component from the Visual Studio Installer or from the GitHub repository.
  
``` git clone https://github.com/Microsoft/vcpkg.git```
``` .\vcpkg\bootstrap-vcpkg.bat```
``` vcpkg integrate install```

See for more information: https://vcpkg.io/en/getting-started

Open Visual Studio 2022 and select File > Open > Project/Solution.
Navigate to the directory that contains the sample and open the solution file (WebViewSolution.sln).
Press F5 to build and run the sample.

## Standalone installation

### The WebView2 Runtime is a part of the Microsoft Edge browser.
If you don't have Microsoft Edge installed, you can install the WebView2 Runtime separately. The WebView2 Runtime is available for x86 and x64 platforms. 
You can download the WebView2 Runtime from the Microsoft Edge WebView2 Runtime page : https://developer.microsoft.com/en-us/microsoft-edge/webview2

### Install Microsoft Visual C++ Redistributable packages 
You can install Microsoft Visual C++ Redistributable packages from : https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#visual-studio-2015-2017-2019-and-2022.


## Features
> [!NOTE]
> This project is a sample. It is not intended to be used in production.

- [x] Embeds a WebView2 control within a Win32 application using WTL
- [x] Embeds a WebView2 control within a Modal Dialog
- [x] Embeds a WebView2 control within a Modeless Dialog
- [x] Handle WebView2 events
- [x] Intercept client certificates when WebView2 is making a request to an Http server that needs a client certificate for Http authentication, replace default client certificate dialog prompt
- [x] WebView installation and update
- [ ] Sync cookies between WebView2 and WinInet/WinHTTP

## Screenshots

Main window with WebView2 control embedded:

[<img src="https://github.com/dev-advocacy/WebView2/blob/main/SampleApps/WebView2WTL.Sample/docs/WebView2_Dialog.png" width="500"/>](WebView2.png)

Dialog with WebView2 control embedded:

[<img src="https://github.com/dev-advocacy/WebView2/blob/main/SampleApps/WebView2WTL.Sample/docs/WebView2_Dialog.png" width="500"/>](WebView2_Dialog)


Client Certificate Dialog:

[<img src="https://github.com/dev-advocacy/WebView2/blob/main/SampleApps/WebView2WTL.Sample/docs/WebView2_Certificate.png" width="500"/>](WebView2_Dialog)
