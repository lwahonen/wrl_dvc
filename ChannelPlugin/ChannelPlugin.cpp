#include "pch.h" // Use stdafx.h in Visual Studio 2017 and earlier


#include <sstream>
#include <string>

#include "ChannelPlugin_h.h"
#include <wrl.h>

using namespace Microsoft::WRL;
// You can call launchDebugger as needed to breakpoint execution
bool launchDebugger();

// Our IWTSPlugin implementation skeleton
class ChannelPlugin : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IWTSPlugin>
{
public:
	HRESULT STDMETHODCALLTYPE Initialize(
		__RPC__in_opt IWTSVirtualChannelManager* pChannelMgr) override
	{
		OutputDebugString(L"\n[DVC SKELETON]: Initialize registered\n");
		// Create a IWTSListenerCallback via the COM components registered in register.reg
		IWTSListenerCallback* listener_callback = nullptr;
		CLSID listener_clsid;
		std::wstring clsid_str = L"{1b3c4177-2008-4574-ae71-507a7a8d4ac6}";
		CLSIDFromString(clsid_str.c_str(), &listener_clsid);
		
		// You can change CLSCTX_INPROC_SERVER to CLSCTX_LOCAL_SERVER if you want to run the plugin in it's own process
		// NB This only works with RDP, Citrix requires the plugins to be in-process servers
		auto hr = CoCreateInstance(listener_clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&listener_callback));
		if (SUCCEEDED(hr))
		{
			IWTSListener* listener;
			hr=pChannelMgr->CreateListener("DVC_Sample", 0, listener_callback, &listener);
			if (SUCCEEDED(hr))
			{
				OutputDebugString(L"[DVC SKELETON]: Created and installed IWTSListenerCallback successfully");
				// Just leak the references to IWTSVirtualChannelManager, IWTSListener and IWTSListenerCallback.
				// Memory management does not matter in sample code, we just need the objects to stay alive forever
				listener_callback->AddRef();
				listener->AddRef();
				pChannelMgr->AddRef();
			}
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Connected() override
	{
		OutputDebugString(L"\n[DVC SKELETON]: Connected registered\n");
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Disconnected(
		DWORD dwDisconnectCode) override
	{
		OutputDebugString(L"\n[DVC SKELETON]: Disconnected registered\n");
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Terminated() override
	{
		OutputDebugString(L"\n[DVC SKELETON]: Terminated registered\n");
		return S_OK;
	}
};
CoCreatableClass(ChannelPlugin);

// Our IWTSVirtualChannelCallback implementation skeleton
class ChannelCallback : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IWTSVirtualChannelCallback>
{
public:
	HRESULT STDMETHODCALLTYPE OnDataReceived(ULONG cbSize, __RPC__in_ecount_full(cbSize) BYTE* pBuffer) override
	{
		OutputDebugString(L"\n[DVC SKELETON]: OnDataReceived registered\n");
		// Echo everything back to the server
		pChannel->Write(cbSize, pBuffer, nullptr);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE OnClose(void) override
	{
		OutputDebugString(L"\n[DVC SKELETON]: OnClose registered\n");
		return S_OK;
	}

	IWTSVirtualChannel* pChannel=nullptr;
};
CoCreatableClass(ChannelCallback);

// Our IWTSListenerCallback implementation skeleton
class ListenerCallback : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IWTSListenerCallback>
{
public:
	HRESULT STDMETHODCALLTYPE OnNewChannelConnection(__RPC__in_opt IWTSVirtualChannel* pChannel,
		__RPC__in_opt BSTR data,
		__RPC__out BOOL* pbAccept,
		__RPC__deref_out_opt IWTSVirtualChannelCallback** ppCallback) override
	{
		OutputDebugString(L"\n[DVC SKELETON]: OnNewChannelConnection registered\n");
		if (pChannel == nullptr)
			return S_OK; // We were called from the ChannelLoader test bench, we have no channel to register against
		// Set the values at false / null initially
		*pbAccept = false;
		*ppCallback = nullptr;

		// Create a callback for this channel
		ChannelCallback* channel_callback = nullptr;
		CLSID channel_clsid;
		std::wstring clsid_str = L"{38ceaca9-19db-4917-92ea-8c9610b84a02}";
		CLSIDFromString(clsid_str.c_str(), &channel_clsid);

		// If you want to run each individual DVC connection in it's own process, change CLSCTX_INPROC_SERVER to CLSCTX_LOCAL_SERVER.
		// Com will create surrogate processes for you as needed.
		// NB: This also only works with RDP, Citrix requires the servers to be in-process
		auto hr = CoCreateInstance(channel_clsid, nullptr, CLSCTX_INPROC_SERVER, IID_IWTSVirtualChannelCallback, (void**)(&channel_callback));
		if (SUCCEEDED(hr))
		{
			OutputDebugString(L"[DVC SKELETON]: Created and installed IWTSVirtualChannelCallback successfully");
			// Inform channel manager that we succeeded in creating the channel, set pointer
			*pbAccept = true;
			*ppCallback = (static_cast<IWTSVirtualChannelCallback*>(channel_callback));
			// Provide the IWTSVirtualChannel to IWTSVirtualChannelCallback so we can echo bytes back as needed
			channel_callback->pChannel = pChannel;
		 	// Just leak the references to IWTSVirtualChannel and IWTSVirtualChannelCallback.
			// Memory management does not matter in sample code, we just need the objects to stay alive forever
			channel_callback->AddRef();
			pChannel->AddRef();
		}
		return S_OK;
	}
};
CoCreatableClass(ListenerCallback);

// Utility function to request Windows to open a debugger session for us
// Lifted from Stack Overflow
boolean debugging = false;
bool launchDebugger()
{
	// Don't hang if called twice
	if (debugging)
	{
		DebugBreak();
		return true;
	}
	debugging = true;
	// Get System directory, typically c:\windows\system32
	std::wstring systemDir(MAX_PATH + 1, '\0');
	UINT nChars = GetSystemDirectoryW(&systemDir[0], systemDir.length());
	if (nChars == 0) return false; // failed to get system directory
	systemDir.resize(nChars);

	// Get process ID and create the command line
	DWORD pid = GetCurrentProcessId();
	std::wostringstream s;
	s << systemDir << L"\\vsjitdebugger.exe -p " << pid;
	std::wstring cmdLine = s.str();

	// Start debugger process
	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessW(NULL, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return false;

	// Close debugger process handles to eliminate resource leak
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	// Wait for the debugger to attach
	while (!IsDebuggerPresent()) Sleep(100);

	// Stop execution so the debugger can take over
	DebugBreak();
	return true;
}

