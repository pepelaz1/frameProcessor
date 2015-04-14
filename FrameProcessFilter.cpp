#include "FrameProcessFilter.h"
#include "FrmProcessPropPage.h"

bool CFrameProcessFilter::IsValidYV12(const CMediaType *pmt)
{
    // Note: The pmt->formattype member indicates what kind of data
    // structure is contained in pmt->pbFormat. But it's important
    // to check that pbFormat is non-NULL and the size (cbFormat) is
    // what we think it is. 	
    if ((pmt->majortype == MEDIATYPE_Video) &&
        (pmt->subtype == MEDIASUBTYPE_YUY2) &&
        (pmt->formattype == FORMAT_VideoInfo) &&
        (pmt->pbFormat != NULL) &&
        (pmt->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
        BITMAPINFOHEADER *pBmi = &(pVih->bmiHeader);

        // Sanity check
        //if ((pBmi->biBitCount = 12) &&
         //   (pBmi->biCompression = FCC('YV12')) &&
        if ((pBmi->biBitCount = 16) &&
            (pBmi->biCompression = FCC('YUY2')) &&
		(pBmi->biSizeImage >= DIBSIZE(*pBmi)))
        {
            return true;
        }
        // Note: The DIBSIZE macro calculates the real size of the bitmap,
        // taking DWORD alignment into account. For YUV formats, this works
        // only when the bitdepth is an even power of 2, not for all YUV types.
    }
    return false;
}



//----------------------------------------------------------------------------
// GetVideoInfoParameters
//
// Helper function to get the important information out of a VIDEOINFOHEADER
//
//-----------------------------------------------------------------------------
void CFrameProcessFilter::GetVideoInfoParameters(
    const VIDEOINFOHEADER *pvih, // Pointer to the format header.
    BYTE  * const pbData,        // Pointer to the first address in the buffer.
    DWORD *pdwWidth,         // Returns the width in pixels.
    DWORD *pdwHeight,        // Returns the height in pixels.
    LONG  *plStrideInBytes,  // Add this to a row to get the new row down
    BYTE **ppbTop,           // Returns pointer to the first byte in the top row of pixels.
    bool bYuv
    )
{
    LONG lStride;


    //  For 'normal' formats, biWidth is in pixels. 
    //  Expand to bytes and round up to a multiple of 4.
    if (pvih->bmiHeader.biBitCount != 0 &&
        0 == (7 & pvih->bmiHeader.biBitCount)) 
    {
        lStride = (pvih->bmiHeader.biWidth * (pvih->bmiHeader.biBitCount / 8) + 3) & ~3;
    } 
    else   // Otherwise, biWidth is in bytes.
    {
        lStride = pvih->bmiHeader.biWidth;
    }

    //  If rcTarget is empty, use the whole image.
    if (IsRectEmpty(&pvih->rcTarget)) 
    {
        *pdwWidth = (DWORD)pvih->bmiHeader.biWidth;
        *pdwHeight = (DWORD)(abs(pvih->bmiHeader.biHeight));
        
        if (pvih->bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap. 
        {
            *plStrideInBytes = lStride; // Stride goes "down"
            *ppbTop           = pbData; // Top row is first.
        } 
        else        // Bottom-up bitmap
        {
            *plStrideInBytes = -lStride;    // Stride goes "up"
            *ppbTop = pbData + lStride * (*pdwHeight - 1);  // Bottom row is first.
        }
    } 
    else   // rcTarget is NOT empty. Use a sub-rectangle in the image.
    {
        *pdwWidth = (DWORD)(pvih->rcTarget.right - pvih->rcTarget.left);
        *pdwHeight = (DWORD)(pvih->rcTarget.bottom - pvih->rcTarget.top);
        
        if (pvih->bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap.
        {
            // Same stride as above, but first pixel is modified down
            // and and over by the target rectangle.
            *plStrideInBytes = lStride;     
            *ppbTop = pbData +
                     lStride * pvih->rcTarget.top +
                     (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
        } 
        else  // Bottom-up bitmap.
        {
            *plStrideInBytes = -lStride;
            *ppbTop = pbData +
                     lStride * (pvih->bmiHeader.biHeight - pvih->rcTarget.top - 1) +
                     (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
        }
    }
}


//----------------------------------------------------------------------------
// CFrameProcessFilter::CheckInputType
//  
// checks whether a specified media type is acceptable for input.
// Examine a proposed input type. Returns S_OK if we can accept his input type
// or VFW_E_TYPE_NOT_ACCEPTED otherwise. This filter accepts YV12 types only.
//-----------------------------------------------------------------------------
HRESULT CFrameProcessFilter::CheckInputType(const CMediaType *pmt)
{
    if (IsValidYV12(pmt))
    {
        return S_OK;
    }
    else
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
}
//----------------------------------------------------------------------------
// CFrameProcessFilter::CheckTransform
//
// Compare an input type with an output type, and see if we can convert from 
// one to the other. The input type is known to be OK from ::CheckInputType,
// so this is really a check on the output type.
//-----------------------------------------------------------------------------
HRESULT CFrameProcessFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    if (!IsValidYV12(mtOut))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
	
    BITMAPINFOHEADER *pBmi = HEADER(mtIn);
    BITMAPINFOHEADER *pBmi2 = HEADER(mtOut);

    if ((pBmi->biWidth <= pBmi2->biWidth) &&
        (pBmi->biHeight == abs(pBmi2->biHeight)))
    {
       return S_OK;
    }
    return VFW_E_TYPE_NOT_ACCEPTED;
}
//----------------------------------------------------------------------------
// CFrameProcessFilter::GetMediaType
//
// Return an output type that we like, in order of preference, by index number.
//
// iPosition: index number
// pMediaType: Write the media type into this object.
//-----------------------------------------------------------------------------
HRESULT CFrameProcessFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // The output pin calls this method only if the input pin is connected.
    ASSERT(m_pInput->IsConnected());

    // There is only one output type that we want, which is the input type.
    if (iPosition < 0)
    {
        return E_INVALIDARG;
    }
    else if (iPosition == 0)
    {  // retrieves the media type for the current pin connection
        return m_pInput->ConnectionMediaType(pMediaType);
    }
    return VFW_S_NO_MORE_ITEMS;
}

//----------------------------------------------------------------------------
// CFrameProcessFilter::DecideBufferSize
//
// Decide the buffer size and other allocator properties, for the downstream
// allocator.
//
// pAlloc: Pointer to the allocator. 
// pProp: Contains the downstream filter's request (or all zeroes)
//-----------------------------------------------------------------------------
HRESULT CFrameProcessFilter::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp)
{
    // Make sure the input pin connected.
    if (!m_pInput->IsConnected()) 
    {
        return E_UNEXPECTED;
    }
    // Our strategy here is to use the upstream allocator as the guideline, but
    // also defer to the downstream filter's request when it's compatible with us.

    // First, find the upstream allocator...
    ALLOCATOR_PROPERTIES InputProps;

    IMemAllocator *pAllocInput = 0;
    HRESULT hr = m_pInput->GetAllocator(&pAllocInput);

    if (FAILED(hr))
    {
        return hr;
    }
    // ... now get the properters
    hr = pAllocInput->GetProperties(&InputProps);
    pAllocInput->Release();
    if (FAILED(hr)) 
    {
        return hr;
    }
    // Buffer alignment should be non-zero [zero alignment makes no sense!]
    if (pProp->cbAlign == 0)
    {
        pProp->cbAlign = 1;
    }
    // Number of buffers must be non-zero
    if (pProp->cbBuffer == 0)
    {
        pProp->cBuffers = 1;
    }
    // For buffer size, find the maximum of the upstream size and 
    // the downstream filter's request.
    pProp->cbBuffer = max(InputProps.cbBuffer, pProp->cbBuffer);
	   
    // Now set the properties on the allocator that was given to us,
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProp, &Actual);
    if (FAILED(hr)) 
    {
		 return hr;
    }
    
    // Even if SetProperties succeeds, the actual properties might be
    // different than what we asked for. We check the result, but we only
    // look at the properties that we care about. The downstream filter
    // will look at them when NotifyAllocator is called.    
    if (InputProps.cbBuffer > Actual.cbBuffer) 
    {
        return E_FAIL;
    }    
    return S_OK;
}

//----------------------------------------------------------------------------
// CFrameProcessFilter::SetMediaType
//
// The CTransformFilter class calls this method when the media type is 
// set on either pin. This gives us a chance to grab the format block. 
//
// direction: Which pin (input or output) 
// pmt: The media type that is being set.
//
// The method is called when the media type is set on one of the filter's pins.
// Note: If the pins were friend classes of the filter, we could access the
// connection type directly. But this is easier than sub-classing the pins.
//-----------------------------------------------------------------------------
HRESULT CFrameProcessFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
    if (direction == PINDIR_INPUT)
    {
        ASSERT(pmt->formattype == FORMAT_VideoInfo);
        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;

        // WARNING! In general you cannot just copy a VIDEOINFOHEADER
        // struct, because the BITMAPINFOHEADER member may be followed by
        // random amounts of palette entries or color masks. (See VIDEOINFO
        // structure in the DShow SDK docs.) Here it's OK because we just
        // want the information that's in the VIDEOINFOHEADER stuct itself.

        CopyMemory(&m_VihIn, pVih, sizeof(VIDEOINFOHEADER));
    }
    else   // output pin
    {
        ASSERT(direction == PINDIR_OUTPUT);
        ASSERT(pmt->formattype == FORMAT_VideoInfo);
        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
		
        CopyMemory(&m_VihOut, pVih, sizeof(VIDEOINFOHEADER));
    }
    return S_OK;
}

//----------------------------------------------------------------------------
// CFrameProcessFilter::Transform
//
// Transform the image.
//
// pSource: Contains the source image.
// pDest:   Write the transformed image here.
//-----------------------------------------------------------------------------
HRESULT CFrameProcessFilter::Transform(IMediaSample *pSource, IMediaSample *pDest)
{
    // Note: The filter has already set the sample properties on pOut,
    // (see CTransformFilter::InitializeOutputSample).
    // You can override the timestamps if you need - but not in our case.

    // The filter already locked m_csReceive so we're OK.
    // Look for format changes from the video renderer.
    CMediaType *pmt = 0;
    if (S_OK == pDest->GetMediaType((AM_MEDIA_TYPE**)&pmt) && pmt)
    {

        // Notify our own output pin about the new type.
        m_pOutput->SetMediaType(pmt);
        DeleteMediaType(pmt);
    }
    // Get the addresses of the actual bufffers.
    BYTE *pBufferIn, *pBufferOut;

    pSource->GetPointer(&pBufferIn);
    pDest->GetPointer(&pBufferOut);

    long cbByte = 0;
    // Process the buffers
    //HRESULT hr = ProcessFrameYV12(pBufferIn, pBufferOut, &cbByte);
	HRESULT hr = ProcessFrameYUY2(pBufferIn, pBufferOut, &cbByte);

    // Set the size of the destination image.
    ASSERT(pDest->GetSize() >= cbByte);
    
    pDest->SetActualDataLength(cbByte);
    return hr;
}

#define PI 3.1415926

void CFrameProcessFilter::InitArrays()
{
	// Luma precomputed data
	for ( int i = 0; i < 256; i++ )
	{
		for ( int j = 0; j < 256; j++)
		{
			double C = (double)j / 127.0;
			double Lu = (double)i;

			Lu = ((Lu-16) * C);
			if ( Lu < 0)
			{
				Lu = 0;	
			}	
			m_Lumas1[i][j] = (short)Lu;
		}
	}

	for ( int i = 0; i < 512; i++ )
	{
		for ( int j = 0; j < 256; j++)
		{
			int B =  (int)j;	
			int Lu = (int)i;

			Lu = Lu + (B-127) + 16;
			if ( Lu < 0 ) Lu = 0;
			if (Lu > 255) Lu = 255;
			m_Lumas2[i][j] = (unsigned char)Lu;
		}
	}

	// Gamma correction
	for ( int i = 0; i < 256; i++ )
	{
		for ( int j = 0; j < 256; j++)
		{
			double G = (double)j;
			if ( G < 0.0001) G = 0.01;
			G = 128.0/G;
			double Lu = (double)i;
			Lu = 255.0 * pow((Lu/255.0),G);
			if (Lu < 0) Lu = 0;
			if (Lu > 255) Lu = 255;
			m_Lumas3[i][j] = (unsigned char)Lu;
		}
	}

	// Chroma precomputed data
	//__asm {emms};
	/*for (int i=-180,ii=0;i<=180;ii++,i++)
	{
		double Hue=(i * 3.1415926) / 180.0;
		hueSin[ii]=int(sin(Hue) * 128);
		hueCos[ii]=int(cos(Hue) * 128);
	}*/

	// U-128(V-128)*Cos
	for ( int i = 0; i < 256; i++)
	{
		for ( int j = 0; j < 256; j++)
		{
			double H = (double)j - 128.0;
			H *= 180.0/128.0;
			int Cr = (int)((i - 128) * cos(H*PI/180.0) + 180);
			// -180 -> 0 -> 180
			if (Cr < 0) Cr = 0;
			if (Cr > 360) Cr = 360;
			m_Chromas1[i][j] = (short)Cr;
		}
	}

	// U-128(V-128)*Sin
	for ( int i = 0; i < 256; i++)
	{
		for ( int j = 0; j < 256; j++)
		{
			double H = (double)j - 128.0;
			H *= 180.0/128.0;
			int Cr = (int)((i - 128) * sin(H*PI/180.0) + 180);
			// -180 -> 0 -> 180
			if (Cr < 0) Cr = 0;
			if (Cr > 360) Cr = 360;
			m_Chromas2[i][j] = (short)Cr;
		}
	}

	//(U-128) x Cos(H) + (V-128) x Sin(H)
	for ( int i = 0; i < 360; i++)
	{
		for ( int j = 0; j < 360; j++)
		{
			int Cr1 = i - 180;
			int Cr2 = j - 180;
			int Cr = (Cr1 + Cr2) + 180;
			if (Cr < 0) Cr = 0;
			if (Cr > 360) Cr = 360;
			m_Chromas3[i][j] = (short)Cr;
		}
	}

	//(U-128) x Cos(H) - (V-128) x Sin(H)
	for ( int i = 0; i < 360; i++)
	{
		for ( int j = 0; j < 360; j++)
		{
			int Cr1 = i - 180;
			int Cr2 = j - 180;
			int Cr = (Cr1 - Cr2) + 180;
			if (Cr < 0) Cr = 0;
			if (Cr > 360) Cr = 360;
			m_Chromas4[i][j] = (short)Cr;
		}
	}

	// Saturation
	for ( int i = 0; i < 360; i++)
	{
		for ( int j = 0; j < 256; j++)
		{
			int Cr = i - 180;
			int S = j/4;
			int res =  (Cr * S)/32 + 128;
			if (res < 0) res = 0;
			if (res > 255) res = 255;
			m_Chromas5[i][j] = (unsigned char )res;

		}
	}

}

unsigned char CFrameProcessFilter::ProcessLuma(unsigned char src)
{
	short C = m_Lumas1[src][m_Contrast];
	unsigned char L =  m_Lumas2[C][m_Brightness];	
	return m_Lumas3[L][m_Gamma];
}

void CFrameProcessFilter::ProcessChroma(unsigned char srcU, unsigned char srcV,
	unsigned char *dstU, unsigned char *dstV)
{
	short p1 = m_Chromas1[srcU][m_Hue]; // Cos
	short p2 = m_Chromas2[srcV][m_Hue]; // Sin
	short p3 = m_Chromas3[p1][p2];

	/*double H = (double)m_Hue - 128.0;
	H *= 180.0/128.0;
	int sat = m_Saturation/4.0;
	double u = (double)srcU - 128;
	double v = (double)srcV - 128;
	double u2 = u * cos(H*PI/180.0) + v * sin(H*PI/180.0);
	u = ((int)(u2 * m_Saturation/4.0) >> 6) + 128;*/

	*dstU = m_Chromas5[p3][m_Saturation];

	/*if (  (int)u != *dstU
		&& (int)u != *dstU-1
		&& (int)u != *dstU+1
		&& (int)u != *dstU-2
		&& (int)u != *dstU+2
		&& (int)u != *dstU-3
		&& (int)u != *dstU+3
		&& (int)u != *dstU-4
		&& (int)u != *dstU+4
		&& (int)u != *dstU-5
		&& (int)u != *dstU+5
		&& (int)u != *dstU-6
		&& (int)u != *dstU+6
		&& (int)u != *dstU-7
		&& (int)u != *dstU+7
		&& (int)u != *dstU-8
		&& (int)u != *dstU+8
		)

	{
		int t = 4;
		t = 4;

	}*/

	short p4 = m_Chromas1[srcV][m_Hue]; // Cos
	short p5 = m_Chromas2[srcU][m_Hue]; // Sin
	short p6 = m_Chromas3[p4][p5];
	*dstV = m_Chromas5[p6][m_Saturation];

	// Old
	/*double h = (double)m_Hue;
	h -= 128;
	h *= 180.0/128.0;

	int sat = m_Saturation/4.0;

	int Cos = hueCos[(int)h+180];
	int Sin = hueSin[(int)h+180];

	int u = srcU - 128;
    int v = srcV - 128;

	
	int u2 = ((u * Cos)>>7) + ((v * Sin)>>7);
    int v2 = ((v * Cos)>>7) - ((u * Sin)>>7);

    u = ((u2 * sat) >> 6) + 128;
    v = ((v2 * sat) >> 6) + 128;

    if (u < 0) u = 0;
    if (u > 255) u = 255;
    if (v < 0) v = 0;
    if (v > 255) v = 255;

    *dstU = u;
    *dstV = v;*/

}

void CFrameProcessFilter::UpdateLuma()
{
	double C = (double)m_Contrast / 127.0;
	double G = (double)m_Gamma;
	if ( G < 0.0001) G = 0.01;
	G = 128.0/G;
	for ( int i = 0; i < 256; i++)
	{
		double L =  ((i-16) * C)+ (m_Brightness-127) + 16;
		L = 255.0 * pow((L/255.0),G);
		if (L < 0 ) L = 0;
		if (L > 255) L = 255;
		m_Luma[i] = (unsigned char)L;
	}
}

void CFrameProcessFilter::UpdateChroma()
{
	double H = (double)m_Hue - 128.0;
	H *= 180.0/128.0;
	double cosH = cos(H*PI/180.0);
	double sinH = sin(H*PI/180.0);

	int S = m_Saturation/4;

	for ( int i = 0; i < 256; i++)
	{
		for ( int j = 0; j < 256; j++)
		{
			double Cr = (((i - 128) * cosH + (j-128) * sinH) * S)/32 + 128;
			if (Cr < 0) Cr = 0;
			if (Cr > 256) Cr = 256;
			m_ChromaU[i][j] = (unsigned char)Cr;
		}
	}

	for ( int i = 0; i < 256; i++)
	{
		for ( int j = 0; j < 256; j++)
		{
			double Cr = (((j - 128) * cosH - (i-128) * sinH) * S)/32 + 128;
			if (Cr < 0) Cr = 0;
			if (Cr > 256) Cr = 256;
			m_ChromaV[i][j] = (unsigned char)Cr;
		}
	}

}
	

// CFrameProcessFilter::ProcessFrame
//int k = 0;
//int p1 = 0;
//int p2 = 0 ;
/*HRESULT CFrameProcessFilter::ProcessFrameYV12(BYTE *pbInput, BYTE *pbOutput, long *pcbByte)
{

    DWORD dwWidth, dwHeight;      // Width and height in pixels
    LONG  lStrideIn, lStrideOut;  // Stride in bytes
    BYTE  *pbSource, *pbTarget;   // First byte in first row, for source and target.

    *pcbByte = m_VihOut.bmiHeader.biSizeImage;

    GetVideoInfoParameters(&m_VihIn, pbInput, &dwWidth, &dwHeight, &lStrideIn, &pbSource, true);
    GetVideoInfoParameters(&m_VihOut, pbOutput, &dwWidth, &dwHeight, &lStrideOut, &pbTarget, true);


//	DWORD start = timeGetTime();

	unsigned int i = 0;
	for (i = 0; i < dwHeight; i++)
    {
		for ( int j = 0; j < lStrideIn; j++)
		{
			//pbSource[j] = ProcessLuma(pbSource[j]);
			//pbTarget[j] = m_Luma[pbSource[j]];
			pbSource[j] = m_Luma[pbSource[j]];
		}
		CopyMemory(pbTarget,pbSource, lStrideOut);
		pbTarget += lStrideOut;
		pbSource += lStrideIn;
	}

	//DWORD point1 = timeGetTime();

	BYTE *pbSourceU = pbSource;
	BYTE *pbTargetU = pbTarget;

	BYTE *pbSourceV = pbSource + (dwHeight/2 * lStrideIn/2);
	BYTE *pbTargetV = pbTarget + (dwHeight/2 * lStrideOut/2);


	for (i = 0; i < dwHeight/2; i++)
    {
		for ( int j = 0; j < lStrideIn; j++)
		{
			//unsigned char u;
			//unsigned char v;

			//ProcessChroma(pbSourceU[j], pbSourceV[j], &u, &v);

			//pbSourceU[j] = u;
			//pbSourceV[j] = v;
			//pbTargetU[j] = pbSourceU[j];
			//pbTargetV[j] = pbSourceV[j];

		//	pbTargetU[j] = m_ChromaU[pbSourceU[j]][pbSourceV[j]];
		//	pbTargetV[j] = m_ChromaV[pbSourceU[j]][pbSourceV[j]];

			unsigned char u =  m_ChromaU[pbSourceU[j]][pbSourceV[j]];
			unsigned char v  = m_ChromaV[pbSourceU[j]][pbSourceV[j]];

			pbSourceU[j] = u;
			pbSourceV[j] = v;

		}

		CopyMemory(pbTargetU,pbSourceU, lStrideOut);
		CopyMemory(pbTargetV,pbSourceV, lStrideOut);

		pbTargetU += lStrideOut/2;
		pbSourceU += lStrideIn/2;

		pbTargetV += lStrideOut/2;
		pbSourceV += lStrideIn/2;
	}


    return S_OK;

}*/

BYTE g_frm[3686400];
int n = 500;
int cur = 0;

HRESULT CFrameProcessFilter::ProcessFrameYUY2(BYTE *pbInput, BYTE *pbOutput, long *pcbByte)
{

    DWORD dwWidth, dwHeight;      // Width and height in pixels
    LONG  lStrideIn, lStrideOut;  // Stride in bytes
    BYTE  *pbSource, *pbTarget;   // First byte in first row, for source and target.

    *pcbByte = m_VihOut.bmiHeader.biSizeImage;

    GetVideoInfoParameters(&m_VihIn, pbInput, &dwWidth, &dwHeight, &lStrideIn, &pbSource, true);
    GetVideoInfoParameters(&m_VihOut, pbOutput, &dwWidth, &dwHeight, &lStrideOut, &pbTarget, true);
	
//	DWORD start = timeGetTime();
	BYTE *pbSource2 = (BYTE *)g_frm;

	unsigned int i = 0;
	for (i = 0; i < dwHeight; i++)
    {
		for ( int j = 0; j < lStrideIn; j+=4)
		{
			
			/*pbSource[j] = m_Luma[pbSource[j]];
			pbSource[j+1] = m_ChromaU[pbSource[j+1]][pbSource[j+3]];
			pbSource[j+2] = m_Luma[pbSource[j+2]];
			pbSource[j+3] = m_ChromaV[pbSource[j+1]][pbSource[j+3]];*/

			pbSource[j] = m_Luma[pbSource[j]];
			BYTE u = pbSource[j+1];
			pbSource[j+1] = m_ChromaU[u][pbSource[j+3]];
			pbSource[j+2] = m_Luma[pbSource[j+2]];
			pbSource[j+3] = m_ChromaV[u][pbSource[j+3]];

			if ( cur > n)
			{
				pbSource[j] = 0.5*(double)pbSource[j] + 0.5*(double)pbSource2[j];
				pbSource[j+1] = 0.5*(double)pbSource[j+1] + 0.5*(double)pbSource2[j+1];
				pbSource[j+2] = 0.5*(double)pbSource[j+2] + 0.5*(double)pbSource2[j+2];
				pbSource[j+3] = 0.5*(double)pbSource[j+3] + 0.5*(double)pbSource2[j+3];
				/*pbSource[j] = pbSource2[j];
				pbSource[j+1] = pbSource2[j+1];
				pbSource[j+2] = pbSource2[j+2];
				pbSource[j+3] = pbSource2[j+3];*/
			}

		}
		if (cur == n)
		{
			CopyMemory(pbSource2, pbSource,lStrideIn);
			
		}
		
		//CopyMemory(pbTarget,pbSource, lStrideOut*2);
		CopyMemory(pbTarget,pbSource, lStrideIn);		

		pbTarget += lStrideOut;
		pbSource += lStrideIn;

		//if (cur == n)
			pbSource2 += lStrideIn;
	}

	cur++;

	/*if (cur > n)
	{
		GetVideoInfoParameters(&m_VihIn, pbInput, &dwWidth, &dwHeight, &lStrideIn, &pbSource, true);
		GetVideoInfoParameters(&m_VihOut, pbOutput, &dwWidth, &dwHeight, &lStrideOut, &pbTarget, true);

		//	

		for (i = 0; i < dwHeight; i++)
		{
			for ( int j = 0; j < lStrideIn; j++)
			{
				pbSource[j] = pbSource2[j];
			}

			CopyMemory(pbTarget,pbSource, lStrideIn);		

			pbTarget += lStrideOut;
			pbSource += lStrideIn;
			pbSource2 += lStrideIn;
		}	
	}*/
		


    return S_OK;

}



// COM stuff

//----------------------------------------------------------------------------
// CFrameProcessor::CreateInstance
//
// Static method that returns a new instance of our filter.
// Note: The DirectShow class factory object needs this method.
//
// pUnk: Pointer to the controlling IUnknown (usually NULL)
// pHR:  Set this to an error code, if an error occurs
//-----------------------------------------------------------------------------
CUnknown * WINAPI CFrameProcessFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHR) 
{
    CFrameProcessFilter *pFilter = new CFrameProcessFilter(pUnk, pHR );
    if (pFilter == NULL) 
    {
        *pHR = E_OUTOFMEMORY;
    }
    return pFilter;
} 


// The next bunch of structures define information for the class factory.
AMOVIESETUP_FILTER FilterInfo =
{
    &CLSID_FrameProcessor,     // CLSID
    g_Name,          // Name
    MERIT_DO_NOT_USE,   // Merit
    0,                  // Number of AMOVIESETUP_PIN structs
    NULL                // Pin registration information.
};

CFactoryTemplate g_Templates[2] = 
{
    { 
      g_Name,                // Name
      &CLSID_FrameProcessor,           // CLSID
      CFrameProcessFilter::CreateInstance, // Method to create an instance of MyComponent
      NULL,                     // Initialization function
      &FilterInfo               // Set-up information (for filters)
    }
	,
    {
	  g_PPName,
	  &CLSID_FrameProcessorPropPage,
      CFrmProcessorProps::CreateInstance 
	}

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    


// Functions needed by the DLL, for registration.
STDAPI DllRegisterServer(void)
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

//
// NonDelegatingQueryInterface
//
// Reveals IFrameProcessor and ISpecifyPropertyPages
//
STDMETHODIMP CFrameProcessFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IFrameProcessor) {
        return GetInterface((IFrameProcessor *) this, ppv);

    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);

    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface


//
// ISpecifyPropertyPage implementation
//
STDMETHODIMP CFrameProcessFilter::GetPages(CAUUID *pPages)
{
	CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if(pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_FrameProcessorPropPage;
    return NOERROR;
}


//
// IFrameProcessor implementation
//
STDMETHODIMP CFrameProcessFilter::get_BrightnessLevel(unsigned char *BrightnessLevel)
{
  *BrightnessLevel = m_Brightness;
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::put_BrightnessLevel(unsigned char BrightnessLevel)
{
  m_Brightness = BrightnessLevel;
  UpdateLuma();
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::get_ContrastLevel(unsigned char *ContrastLevel)
{
  *ContrastLevel = m_Contrast;
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::put_ContrastLevel(unsigned char ContrastLevel)
{
  m_Contrast = ContrastLevel;
  UpdateLuma();
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::get_HueLevel(unsigned char *HueLevel)
{
  *HueLevel = m_Hue;
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::put_HueLevel(unsigned char HueLevel)
{
  m_Hue = HueLevel;
  UpdateChroma();
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::get_SaturationLevel(unsigned char *SaturationLevel)
{
  *SaturationLevel = m_Saturation;
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::put_SaturationLevel(unsigned char SaturationLevel)
{
  m_Saturation = SaturationLevel;
  UpdateChroma();
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::get_GammaCorrectionLevel(unsigned char *GammaCorrectionLevel)
{
  *GammaCorrectionLevel = m_Gamma;
  return NOERROR;
}
STDMETHODIMP CFrameProcessFilter::put_GammaCorrectionLevel(unsigned char GammaCorrectionLevel)
{
  m_Gamma = GammaCorrectionLevel;
  UpdateLuma();
  return NOERROR;
}
