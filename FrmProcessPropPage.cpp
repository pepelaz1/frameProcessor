#include <streams.h>
#include <commctrl.h>
#include "resource.h"
#include "consts.h"
#include "IFrameProcessor.h"
#include "FrameProcessFilter.h"
#include "FrmProcessPropPage.h"


CUnknown * WINAPI CFrmProcessorProps::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);
    CUnknown *punk = new CFrmProcessorProps(lpunk, phr);
    if(punk == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return punk;
}

CFrmProcessorProps::CFrmProcessorProps(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage((TCHAR *)g_PPName, pUnk, IDD_FRMPROCESSPP, IDS_TITLE),
    m_pProps(NULL)
{
    InitCommonControls();

	m_BrightnessLevel = g_DefaultBrightnessLevel;
	m_ContrastLevel = g_DefaultContrastLevel;
	m_HueLevel = g_DefaultHueLevel;
	m_SaturationLevel = g_DefaultSaturationLevel;
	m_GammaLevel = g_DefaultGammaLevel;
} 

void CFrmProcessorProps::SetDirty()
{
    m_bDirty = TRUE;
    if(m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

} 

INT_PTR CFrmProcessorProps::OnReceiveMessage(HWND hwnd,UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            CreateSliders(hwnd);
            return (LRESULT) 1;
        }
        case WM_VSCROLL:
        {           
            OnSliderNotification(wParam, lParam);
            return (LRESULT) 1;
        }
		case WM_COMMAND:
        {
            if(LOWORD(wParam) == IDB_BRIGHTNESS_DEF)
            {
				GetProps()->put_BrightnessLevel(g_DefaultBrightnessLevel);
                SendMessage(m_Sliders[0], TBM_SETPOS, TRUE, g_DefaultBrightnessLevel);
                SetDirty();
            }
			else if(LOWORD(wParam) == IDB_CONTRAST_DEF)
            {
				GetProps()->put_ContrastLevel(g_DefaultContrastLevel);
                SendMessage(m_Sliders[1], TBM_SETPOS, TRUE, g_DefaultContrastLevel);
                SetDirty();
            }
			else if(LOWORD(wParam) == IDB_HUE_DEF)
            {
				GetProps()->put_HueLevel(g_DefaultHueLevel);
                SendMessage(m_Sliders[2], TBM_SETPOS, TRUE, g_DefaultHueLevel);
                SetDirty();
            }
			else if(LOWORD(wParam) == IDB_SATURATION_DEF)
            {
				GetProps()->put_SaturationLevel(g_DefaultSaturationLevel);
                SendMessage(m_Sliders[3], TBM_SETPOS, TRUE, g_DefaultSaturationLevel);
                SetDirty();
            }
			else if(LOWORD(wParam) == IDB_GAMMA_DEF)
            {
				GetProps()->put_GammaCorrectionLevel(g_DefaultGammaLevel);
                SendMessage(m_Sliders[4], TBM_SETPOS, TRUE, g_DefaultGammaLevel);
                SetDirty();
            }
            return (LRESULT) 1;
        }
        case WM_DESTROY:
        {
			DestroySliders();
            return (LRESULT) 1;
        }
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
} 


HRESULT CFrmProcessorProps::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pProps == NULL);
    CheckPointer(pUnknown,E_POINTER);

    HRESULT hr = pUnknown->QueryInterface(IID_IFrameProcessor, (void **) &m_pProps);
    if(FAILED(hr))
        return E_NOINTERFACE;

    ASSERT(m_pProps);

    // Get the initial values
    m_pProps->get_BrightnessLevel(&m_BrightnessLevel);
	m_BrightnessLevel = g_MaxBrightnessLevel - m_BrightnessLevel;

	m_pProps->get_ContrastLevel(&m_ContrastLevel);
	m_ContrastLevel = g_MaxContrastLevel - m_ContrastLevel;

	m_pProps->get_HueLevel(&m_HueLevel);
	m_HueLevel = g_MaxHueLevel - m_HueLevel;
	    
	m_pProps->get_SaturationLevel(&m_SaturationLevel);
	m_SaturationLevel = g_MaxSaturationLevel - m_SaturationLevel;

	m_pProps->get_GammaCorrectionLevel(&m_GammaLevel);
	m_GammaLevel = g_MaxGammaLevel - m_GammaLevel;

    return NOERROR;
} 

HRESULT CFrmProcessorProps::OnDisconnect()
{
    // Release of Interface after setting the appropriate contrast value
    if (!m_pProps)
        return E_UNEXPECTED;

    m_pProps->Release();
    m_pProps = NULL;
    return NOERROR;

} 

HRESULT CFrmProcessorProps::OnDeactivate(void)
{
    return NOERROR;
}

HRESULT CFrmProcessorProps::OnApplyChanges()
{
    m_bDirty = FALSE;
    return(NOERROR);

} 

HWND CFrmProcessorProps::CreateSlider( HWND parent, int x, int y, int width, int height
	, int value, int min, int max)
{
	ULONG Styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_VERT | TBS_DOWNISLEFT | TBS_BOTH | WS_GROUP ;
    HWND slider = CreateWindow(TRACKBAR_CLASS, L"", Styles,
                                   x, y, width, height,
                                   parent, NULL, g_hInst, NULL);
	SendMessage(slider, TBM_SETRANGE, TRUE, MAKELONG(min,max));
    SendMessage(slider, TBM_SETTIC, 0, 0L);
    SendMessage(slider, TBM_SETPOS, TRUE, value);
	return slider;
}

void CFrmProcessorProps::CreateSliders(HWND parent)
{
	m_Sliders.push_back(CreateSlider(parent,25,15,40,170, m_BrightnessLevel
		, g_MinBrightnessLevel, g_MaxBrightnessLevel));

	m_Sliders.push_back(CreateSlider(parent,110,15,40,170, m_ContrastLevel
		, g_MinContrastLevel, g_MaxContrastLevel));

	m_Sliders.push_back(CreateSlider(parent,195,15,40,170, m_HueLevel
		, g_MinHueLevel, g_MaxHueLevel));

	m_Sliders.push_back(CreateSlider(parent,275,15,40,170, m_SaturationLevel
		, g_MinSaturationLevel, g_MaxSaturationLevel));

	m_Sliders.push_back(CreateSlider(parent,360,15,40,170, m_GammaLevel
		, g_MinGammaLevel, g_MaxGammaLevel));
} 

void CFrmProcessorProps::DestroySliders()
{
	vector<HWND>::iterator it;
	for (it = m_Sliders.begin(); it != m_Sliders.end(); it++)
		DestroyWindow(*it);
}

void CFrmProcessorProps::OnSliderNotification(WPARAM wParam, LPARAM lParam)
{
	SetDirty();
	UpdateValues(lParam);
} 

void CFrmProcessorProps::UpdateValues(LPARAM lParam)
{
	HWND slider = (HWND)lParam;
	if (slider == m_Sliders[0])
	{
		unsigned char brightness = (unsigned char)SendMessage(slider, TBM_GETPOS, 0, 0L);	
		GetProps()->put_BrightnessLevel(g_MaxBrightnessLevel-brightness);
	}
	else if (slider == m_Sliders[1])
	{
		unsigned char contrast = (unsigned char)SendMessage(slider, TBM_GETPOS, 0, 0L);	 
		GetProps()->put_ContrastLevel(g_MaxContrastLevel - contrast);
	}
	else if (slider == m_Sliders[2])
	{
		unsigned char hue = (unsigned char)SendMessage(slider, TBM_GETPOS, 0, 0L);	 
		GetProps()->put_HueLevel(g_MaxHueLevel - hue);
	}
	else if (slider == m_Sliders[3])
	{
		unsigned char saturation = (unsigned char)SendMessage(slider, TBM_GETPOS, 0, 0L);	 
		GetProps()->put_SaturationLevel(g_MaxSaturationLevel - saturation);
	}
	else if (slider == m_Sliders[4])
	{
		unsigned char gamma = (unsigned char)SendMessage(slider, TBM_GETPOS, 0, 0L);	 
		GetProps()->put_GammaCorrectionLevel(g_MaxGammaLevel - gamma);
	}
}
