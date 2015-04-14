#pragma once
#pragma warning(disable : 4995)
#include <string>
#include <vector>
using namespace std;


class CFrmProcessorProps : public CBasePropertyPage
{
public:
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void SetDirty();

    void  CreateSliders(HWND parent);
	HWND  CreateSlider(HWND parent, int x, int y, int width, int height, int value, int min, int max);
	void DestroySliders();

    void OnSliderNotification(WPARAM wParam, LPARAM lParam);
	void UpdateValues(LPARAM lParam);
	vector<HWND> m_Sliders;

    CFrmProcessorProps(LPUNKNOWN lpunk, HRESULT *phr);

    //signed char m_cContrastOnExit; // Remember contrast level for CANCEL
    //signed char m_cContrastLevel;  // And likewise for next activate
	unsigned char m_BrightnessLevel;
	unsigned char m_ContrastLevel;
	unsigned char m_HueLevel;
	unsigned char m_SaturationLevel;
	unsigned char m_GammaLevel;

    IFrameProcessor *m_pProps;

    IFrameProcessor *GetProps() 
	{
        ASSERT(m_pProps);
        return m_pProps;
    };
}; // CContrastProperties

