#include "stdafx.h"
#include <iostream>
#include "Windows.h"
#include "comdef.h"


#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <atlcomcli.h>
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")

void GetSpeakerAudio()
{
    CComPtr<IMMDeviceEnumerator> pIMMEnumerator = NULL;
    CComPtr<IMMDevice> pIMMDeivce = NULL;
    CComPtr<IAudioEndpointVolume> pIAudioEndpointVolume = NULL;

    HRESULT hr = S_OK;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pIMMEnumerator);
    if (SUCCEEDED(hr))
    {
        hr = pIMMEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pIMMDeivce);
        if (SUCCEEDED(hr))
        {
            hr = pIMMDeivce->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pIAudioEndpointVolume);
            if (SUCCEEDED(hr))
            {
                float m_fVolum = -1;
                hr = pIAudioEndpointVolume->GetMasterVolumeLevelScalar(&m_fVolum);
            }
        }
    }
}

//dwVolume:可以忽略，IsRecover：true恢复其他程序音量，false是设置当前程序音量
BOOL SetCurrentProcessVolume(DWORD dwVolume, BOOL IsRecover)
{
    HRESULT hr = S_OK;
    IMMDeviceCollection *pMultiDevice = NULL;
    IMMDevice *pDevice = NULL;
    IAudioSessionEnumerator *pSessionEnum = NULL;
    IAudioSessionManager2 *pASManager = NULL;
    IMMDeviceEnumerator *m_pEnumerator = NULL;
    const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
    const IID IID_IAudioSessionControl2 = __uuidof(IAudioSessionControl2);
    GUID m_guidMyContext;
    CoInitialize(NULL);
    hr = CoCreateGuid(&m_guidMyContext);
    if (FAILED(hr))
        return FALSE;
    // Get enumerator for audio endpoint devices.  
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_pEnumerator);
    if (FAILED(hr))
        return FALSE;

    /*if (IsMixer)
    {
    hr = m_pEnumerator->EnumAudioEndpoints(eRender,DEVICE_STATE_ACTIVE, &pMultiDevice);
    }
    else
    {
    hr = m_pEnumerator->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE, &pMultiDevice);
    } */
    hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pMultiDevice);
    if (FAILED(hr))
        return FALSE;

    UINT deviceCount = 0;
    hr = pMultiDevice->GetCount(&deviceCount);
    if (FAILED(hr))
        return FALSE;

    if ((int)dwVolume < 0)
        dwVolume = 0;
    if ((int)dwVolume > 100)
        dwVolume = 100;
    for (UINT ii = 0; ii<deviceCount; ii++)
    {
        pDevice = NULL;
        hr = pMultiDevice->Item(ii, &pDevice);
        if (FAILED(hr))
            return FALSE;
        hr = pDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&pASManager);

        if (FAILED(hr))
            return FALSE;
        hr = pASManager->GetSessionEnumerator(&pSessionEnum);
        if (FAILED(hr))
            return FALSE;
        int nCount;
        hr = pSessionEnum->GetCount(&nCount);
        for (int i = 0; i < nCount; i++)
        {
            IAudioSessionControl *pSessionCtrl;
            hr = pSessionEnum->GetSession(i, &pSessionCtrl);
            if (FAILED(hr))
                continue;
            IAudioSessionControl2 *pSessionCtrl2;
            hr = pSessionCtrl->QueryInterface(IID_IAudioSessionControl2, (void **)&pSessionCtrl2);
            if (FAILED(hr))
                continue;
            ULONG pid;
            hr = pSessionCtrl2->GetProcessId(&pid);
            if (FAILED(hr))
                continue;

            ISimpleAudioVolume *pSimplevol;
            hr = pSessionCtrl2->QueryInterface(IID_ISimpleAudioVolume, (void **)&pSimplevol);
            if (FAILED(hr))
                continue;
            ULONG currentId = GetCurrentProcessId();
            if (pid == currentId) {
                float f = -1;
                pSimplevol->GetMasterVolume(&f);
                //pSimplevol->SetMasterVolume((float)dwVolume / 100, NULL);
            }

        }
    }
    m_pEnumerator->Release();
    return TRUE;
}

//设置当前程序音量
void SetApplicationVolume(int size)
{
    CComPtr<IMMDeviceEnumerator >	pIMMEnumerator = NULL;	//主要用于枚举设备接口
    CComPtr<ISimpleAudioVolume>	pRenderSimpleVol = NULL;	//扬声器的会话音量控制接口
    HRESULT hr = S_OK;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pIMMEnumerator);
    if (SUCCEEDED(hr))
    {
        CComPtr<IMMDevice> pIMMDeivce = NULL;
        hr = pIMMEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pIMMDeivce);
        if (SUCCEEDED(hr))
        {
            CComPtr<IAudioSessionManager> pSessionManager = NULL;
            hr = pIMMDeivce->Activate(__uuidof(IAudioSessionManager), CLSCTX_INPROC_SERVER, NULL, (void **)(&pSessionManager));
            if (SUCCEEDED(hr))
            {
                hr = pSessionManager->GetSimpleAudioVolume(NULL, FALSE, &pRenderSimpleVol);
                if (SUCCEEDED(hr))
                {
                    float fLevel = (float)size / 100;
                    fLevel += 0.000001;
                    if (fLevel >= 1.000)
                        fLevel = 1.000;
                    hr = pRenderSimpleVol->SetMasterVolume(fLevel, NULL);
                }
            }
        }
    }
}
#include <thread>
void test() {
    PlaySoundA("SystemStart", NULL, SND_ALIAS | SND_SYNC);
    Sleep(1000);
    SetCurrentProcessVolume(10, false);
}

int main(int argc, const char * argv[]){
    std::thread s(test);
    while (1) {
        Sleep(100);
    }
	return 0;
}


