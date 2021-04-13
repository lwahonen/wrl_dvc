# Dynamic virtual channel skeleton

Dynamic virtual channel (DVC) APIs provide a byte stream between a server endpoint and a listener endpoint. Microsoft provides documentation here https://docs.microsoft.com/en-us/windows/win32/termserv/dynamic-virtual-channels , with a reasonably good server-side sample project at https://docs.microsoft.com/en-us/windows/win32/termserv/dvc-server-component-example

The client-side sample ( https://docs.microsoft.com/en-us/windows/win32/termserv/dvc-client-plug-in-example ) is quite incomplete, and requires a lot of work before even compiling successfully.

This skeleton provides a batteries-included implementation of a WTS plugin.

# Installation

To use the echo listener, just register the COM elements with register.reg and then add the keys for current user at 

```
[HKEY_CURRENT_USER\SOFTWARE\Microsoft\Terminal Server Client\Default\AddIns\WRL_Sample]
"Name"="{c630c239-3206-45d9-805f-fd6d8dc3d42b}"
```

RDP clients are 64-bit, Citrix clients are 32-bit. Just compile both, register.reg includes all the needed registry keys for both clients

# Development

If you use the skeleton to implement your own channel, you should change the four GUIDs to something randomly generated. Use any online GUID generator to create 4 GUIDs, then do a simple search-and-replace in ChannelPlugin.idl, register.reg, unregister.reg, ChannelPlugin.cpp and optionally ChannelLoader.cpp.

The GUIDs you want to replace are:

```
d3c48573-ef8e-4f17-b3ee-528cbef93645
c630c239-3206-45d9-805f-fd6d8dc3d42b
1b3c4177-2008-4574-ae71-507a7a8d4ac6
38ceaca9-19db-4917-92ea-8c9610b84a02
```

Just make sure to replace all instances of each and re-run register.reg. And obviously change the plugin guid under HKEY_CURRENT_USER\SOFTWARE\Microsoft\Terminal Server Client\Default\AddIns\WRL_Sample

The included ChannelLoader project creates all three of your WTS callbacks via COM, calls a single method on each to verify DLLs and dependencies load correctly, and exits gracefully. It's quite useful when debugging customer issues.