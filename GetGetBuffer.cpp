#include "pch.h"

#include <mmdeviceapi.h>
#define __IAudioRenderClient_INTERFACE_DEFINED__
#include <audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>



typedef struct IAudioRenderClientVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            IAudioRenderClient* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IAudioRenderClient* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IAudioRenderClient* This);

    HRESULT(STDMETHODCALLTYPE* GetBuffer)(
        IAudioRenderClient* This,
        /* [annotation][in] */
        _In_  UINT32 NumFramesRequested,
        /* [annotation][out] */
        _Outptr_result_buffer_(_Inexpressible_("NumFramesRequested * pFormat->nBlockAlign"))  BYTE** ppData);

    HRESULT(STDMETHODCALLTYPE* ReleaseBuffer)(
        IAudioRenderClient* This,
        /* [annotation][in] */
        _In_  UINT32 NumFramesWritten,
        /* [annotation][in] */
        _In_  DWORD dwFlags);

    END_INTERFACE
} IAudioRenderClientVtbl;

interface IAudioRenderClient
{
    CONST_VTBL struct IAudioRenderClientVtbl* lpVtbl;
};



#include "GetGetBuffer.h"



#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
//const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioRenderClient = { 0xf294acfc, 0x3146, 0x4483,
  {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}
};





array<GetGetBuffer::Device>^ GetGetBuffer::Class1::DoIt()
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pDevices = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 bufferFrameCount;
    DWORD flags = 0;
    UINT numDevices = 0;
    IPropertyStore* pProps = NULL;
    PROPVARIANT varName = { 0 };

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
    EXIT_ON_ERROR(hr);

    hr = pDevices->GetCount(&numDevices);
    EXIT_ON_ERROR(hr);

    array<GetGetBuffer::Device>^ output = gcnew array<GetGetBuffer::Device>(numDevices);

    for (UINT i = 0; i < numDevices; i++)
    {

        hr = pDevices->Item(i, &pDevice);
        EXIT_ON_ERROR(hr)

            hr = pDevice->Activate(
                IID_IAudioClient, CLSCTX_ALL,
                NULL, (void**)&pAudioClient);
        EXIT_ON_ERROR(hr)

            hr = pAudioClient->GetMixFormat(&pwfx);
        EXIT_ON_ERROR(hr)

            hr = pAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                0,
                hnsRequestedDuration,
                0,
                pwfx,
                NULL);
        EXIT_ON_ERROR(hr)

            // Tell the audio source which format to use.
            //hr = pMySource->SetFormat(pwfx);
            //EXIT_ON_ERROR(hr)

            // Get the actual size of the allocated buffer.
            hr = pAudioClient->GetBufferSize(&bufferFrameCount);
        EXIT_ON_ERROR(hr)

            hr = pAudioClient->GetService(
                IID_IAudioRenderClient,
                (void**)&pRenderClient);
        EXIT_ON_ERROR(hr)


            hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
        EXIT_ON_ERROR(hr)

            hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr)



            output[i].GetBuffer = IntPtr((void*)pRenderClient->lpVtbl->GetBuffer);
        output[i].ReleaseBuffer = IntPtr((void*)pRenderClient->lpVtbl->ReleaseBuffer);
        output[i].Name = gcnew String(varName.pwszVal);



        hr = pAudioClient->Stop();  // Stop playing.
        EXIT_ON_ERROR(hr);

        PropVariantClear(&varName);
        CoTaskMemFree(pwfx);
        pwfx = NULL;
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pProps);
        SAFE_RELEASE(pAudioClient);
    if (pRenderClient) {
        pRenderClient->lpVtbl->Release(pRenderClient);
        pRenderClient = NULL;
    }
    }

Exit:
    PropVariantClear(&varName);
    CoTaskMemFree(pwfx);
    pwfx = NULL;
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pAudioClient);
    if (pRenderClient) {
        pRenderClient->lpVtbl->Release(pRenderClient);
        pRenderClient = NULL;
    }


    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevices);

    if (FAILED(hr)) throw gcnew System::Runtime::InteropServices::COMException(gcnew String("something went wrong"), hr);

    return output;
}
