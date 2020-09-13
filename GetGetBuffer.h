#pragma once

using namespace System;

namespace GetGetBuffer {
	public value class Device
	{
	public:
		IntPtr GetBuffer;
		IntPtr ReleaseBuffer;
		String^ Name;
	};
	public ref class Class1
	{
	public:
		static array<Device>^ DoIt();
	};
}
