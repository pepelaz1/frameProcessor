
#ifndef __IFRAMEPROCESSOR__
#define __IFRAMEPROCESSOR__

#ifdef __cplusplus
extern "C" {
#endif

	// {8870E62E-8275-40FD-B1D0-64E0A7BE532F}
	DEFINE_GUID(IID_IFrameProcessor, 
	0x8870e62e, 0x8275, 0x40fd, 0xb1, 0xd0, 0x64, 0xe0, 0xa7, 0xbe, 0x53, 0x2f);


    DECLARE_INTERFACE_(IFrameProcessor, IUnknown)
    {
		//
		// Brightness
		//
        STDMETHOD(get_BrightnessLevel) (THIS_
            unsigned char *BrightnessLevel      // The current brigntness level
        ) PURE;

        STDMETHOD(put_BrightnessLevel) (THIS_
            unsigned char BrightnessLevel      // Change to the brigntness level
        ) PURE;

		//
		// Contrast
		//
        STDMETHOD(get_ContrastLevel) (THIS_
            unsigned char *ContrastLevel      // The current contrast level
        ) PURE;

        STDMETHOD(put_ContrastLevel) (THIS_
            unsigned char ContrastLevel      // Change to the contrast level
        ) PURE;

		//
		// Hue
		//
        STDMETHOD(get_HueLevel) (THIS_
            unsigned char *HueLevel      // The current hue level
        ) PURE;

        STDMETHOD(put_HueLevel) (THIS_
            unsigned char HueLevel      // Change to the hue level
        ) PURE;

		//
		// Saturation
		//
        STDMETHOD(get_SaturationLevel) (THIS_
            unsigned char *SaturationLevel      // The current saturation level
        ) PURE;

        STDMETHOD(put_SaturationLevel) (THIS_
            unsigned char SaturationLevel      // Change to the saturation level
        ) PURE;

		//
		// Gamma correction
		//
        STDMETHOD(get_GammaCorrectionLevel) (THIS_
            unsigned char *GammaCorrectionLevel      // The current gamma correction level
        ) PURE;

        STDMETHOD(put_GammaCorrectionLevel) (THIS_
            unsigned char GammaCorrectionLevel      // Change to the gamma correction level
        ) PURE;

    };

#ifdef __IFRAMEPROCESSOR__
}
#endif

#endif // __ICONTRAST__

