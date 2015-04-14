#pragma once

static const GUID CLSID_FrameProcessor = 
{ 0xeebd0577, 0x967, 0x4da9, { 0xab, 0x5f, 0x64, 0x57, 0xf2, 0x27, 0x1a, 0x5f } };

static const GUID CLSID_FrameProcessorPropPage = 
{ 0x410565a, 0x1e5f, 0x40b1, { 0x8a, 0x66, 0xdc, 0x73, 0xa6, 0x31, 0xe2, 0x73 } };

static const TCHAR g_Name[] = L"Frame processor filter";    
static const TCHAR g_PPName[] = L"Frame processor property page";   


const int g_MinBrightnessLevel = 0;
const int g_DefaultBrightnessLevel = 127;
const int g_MaxBrightnessLevel = 255;

const int g_MinContrastLevel = 0;
const int g_DefaultContrastLevel = 127;
const int g_MaxContrastLevel = 255;

const int g_MinHueLevel = 0;
const int g_DefaultHueLevel = 127;
const int g_MaxHueLevel = 255;

const int g_MinSaturationLevel = 0;
const int g_DefaultSaturationLevel = 127;
const int g_MaxSaturationLevel = 255;

const int g_MinGammaLevel = 0;
const int g_DefaultGammaLevel = 127;
const int g_MaxGammaLevel = 255;