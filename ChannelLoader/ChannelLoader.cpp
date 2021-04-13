#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <tsvirtualchannels.h>

// Prints an error string for the provided source code line and HRESULT
// value and returns the HRESULT value as an int.
int PrintError(unsigned int line, HRESULT hr)
{
    wprintf_s(L"ERROR: Line:%d HRESULT: 0x%X\n", line, hr);
    return hr;
}

int wmain()
{
    HRESULT hr;

    // Initialize the COM library.
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return PrintError(__LINE__, hr);
    }

    // Test that plugin works
    IWTSPlugin* plugin = nullptr; // Interface to COM component.
    CLSID plugin_clsid;;
    std::wstring clsid_str = L"{c630c239-3206-45d9-805f-fd6d8dc3d42b}";
    CLSIDFromString(clsid_str.c_str(), &plugin_clsid);

    hr = CoCreateInstance(plugin_clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&plugin));
    if (SUCCEEDED(hr))
    {
        hr = plugin->Connected();
        if (FAILED(hr))
        {
            PrintError(__LINE__, hr);
        }
        else
        {
            std::wcout << L"\nIWTSPlugin registered and working\n";
        }
        plugin->Release();
    }
    else
    {
        // Object creation failed. Print a message.
        PrintError(__LINE__, hr);
    }

    // Test that listener callback works
    IWTSListenerCallback* listener_callback = nullptr; // Interface to COM component.
    CLSID listener_clsid;
    clsid_str = L"{1b3c4177-2008-4574-ae71-507a7a8d4ac6}";
    CLSIDFromString(clsid_str.c_str(), &listener_clsid);

    hr = CoCreateInstance(listener_clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&listener_callback));
    if (SUCCEEDED(hr))
    {
        BOOL b;
        IWTSVirtualChannelCallback* c;
        hr = listener_callback->OnNewChannelConnection(nullptr, nullptr, &b, &c);
        if (FAILED(hr))
        {
            PrintError(__LINE__, hr);
        }
        else
        {
            std::wcout << L"IWTSListenerCallback registered and working\n";
        }

        listener_callback->Release();
    }
    else
    {
        // Object creation failed. Print a message.
        PrintError(__LINE__, hr);
    }

    // Test that channel callback works
    IWTSVirtualChannelCallback* channel_callback = nullptr; // Interface to COM component.
    CLSID channel_clsid;
    clsid_str = L"{38ceaca9-19db-4917-92ea-8c9610b84a02}";
    CLSIDFromString(clsid_str.c_str(), &channel_clsid);

    hr = CoCreateInstance(channel_clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&channel_callback));
    if (SUCCEEDED(hr))
    {
        BOOL b;
        IWTSVirtualChannelCallback* c;
        hr = channel_callback->OnClose();
        if (FAILED(hr))
        {
            PrintError(__LINE__, hr);
        }
        else
        {
            std::wcout << L"IWTSVirtualChannelCallback registered and working\n";
        }

        channel_callback->Release();
    }
    else
    {
        // Object creation failed. Print a message.
        PrintError(__LINE__, hr);
    }

    // Free the COM library.
    CoUninitialize();

    return hr;
}
/* Output:
result = 9
*/