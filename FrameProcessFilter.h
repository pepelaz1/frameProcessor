#pragma once
#include <streams.h>  // DirectShow base class library
#include <initguid.h>
#include <aviriff.h>  // defines 'FCC' macro
#include "IFrameProcessor.h"
#include "consts.h"


class CFrameProcessFilter : public CTransformFilter,
						    public IFrameProcessor,
							public ISpecifyPropertyPages
{
private:
    VIDEOINFOHEADER m_VihIn;   // Holds the current video format (input)
    VIDEOINFOHEADER m_VihOut;  // Holds the current video format (output)

	bool IsValidYV12(const CMediaType *pmt);
	void GetVideoInfoParameters(
		const VIDEOINFOHEADER *pvih, // Pointer to the format header.
		BYTE  * const pbData,        // Pointer to the first address in the buffer.
		DWORD *pdwWidth,         // Returns the width in pixels.
		DWORD *pdwHeight,        // Returns the height in pixels.
		LONG  *plStrideInBytes,  // Add this to a row to get the new row down
		BYTE **ppbTop,           // Returns pointer to the first byte in the top row of pixels.
		bool bYuv);
	//HRESULT ProcessFrameYV12(BYTE *pbInput, BYTE *pbOutput, long *pcbByte);
	HRESULT ProcessFrameYUY2(BYTE *pbInput, BYTE *pbOutput, long *pcbByte);

	unsigned char m_Brightness;
	unsigned char m_Contrast;
	unsigned char m_Hue;
	unsigned char m_Saturation;
	unsigned char m_Gamma;

	short m_Lumas1[256][256];
	unsigned char m_Lumas2[512][256];
	unsigned char m_Lumas3[256][256];


	short m_Chromas1[256][256];
	short m_Chromas2[256][256];
	short m_Chromas3[360][360];
	short m_Chromas4[360][360];
	unsigned char m_Chromas5[360][256];

	//int hueSin[360+1],hueCos[360+1];
	unsigned int gammaTab[256];
	void InitArrays();

	unsigned char ProcessLuma(unsigned char src);
	//unsigned char ProcessChromaU(unsigned char srcU, unsigned char srcV);
	void ProcessChroma(unsigned char srcU, unsigned char srcV,
		unsigned char *dstU, unsigned char *dstV);

	unsigned char m_Luma[256];
	unsigned char m_ChromaU[256][256];
	unsigned char m_ChromaV[256][256];
	void UpdateLuma();
	void UpdateChroma();
public:
    CFrameProcessFilter(LPUNKNOWN pUnk, HRESULT *phr)
		:  CTransformFilter((TCHAR *)g_Name, pUnk, CLSID_FrameProcessor)    
	{
		m_Brightness = g_DefaultBrightnessLevel;
		m_Contrast = g_DefaultContrastLevel;
		m_Hue = g_DefaultHueLevel;
		m_Saturation = g_DefaultSaturationLevel;
		m_Gamma = g_DefaultGammaLevel;
		UpdateLuma();
		UpdateChroma();
	}

	  // Overridden CTransformFilter methods
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);

    // Override this so we can grab the video format
    HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

	 // Static object-creation method (for the class factory)
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

	// Reveals IFrameProcessor and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    DECLARE_IUNKNOWN;


	//
	// ISpecifyPropertiPages implementation
	//
	STDMETHODIMP GetPages(CAUUID *pPages);

	//
	// IFrameProcessor implementation
	//
	STDMETHODIMP get_BrightnessLevel(unsigned char *BrightnessLevel);
    STDMETHODIMP put_BrightnessLevel(unsigned char BrightnessLevel);
	STDMETHODIMP get_ContrastLevel(unsigned char *ContrastLevel);
    STDMETHODIMP put_ContrastLevel(unsigned char ContrastLevel);
 	STDMETHODIMP get_HueLevel(unsigned char *HueLevel);
    STDMETHODIMP put_HueLevel(unsigned char HueLevel);
	STDMETHODIMP get_SaturationLevel(unsigned char *SaturationLevel);
    STDMETHODIMP put_SaturationLevel(unsigned char SaturationLevel);
	STDMETHODIMP get_GammaCorrectionLevel(unsigned char *GammaCorrectionLevel);
    STDMETHODIMP put_GammaCorrectionLevel(unsigned char GammaCorrectionLevel);
};

