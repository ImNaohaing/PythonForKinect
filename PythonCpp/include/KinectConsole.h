/************************************************************************
*                                                                       *
*   KinectConsole.h --    created by Jane/Santaizi 3/7/2016             *
*                                                                       *
*                                                                       *
*   Copyright (c) Jane. All rights reserved.                            *
*                                                                       *
************************************************************************/

#ifndef __KINECT_CONSOLE_H__
#define __KINECT_CONSOLE_H__

#include <NuiApi.h>
#include <vector>

typedef struct _SColorFrame {
    const byte * const pBits;
    const size_t size;

    _SColorFrame(byte *pData, size_t size) :
        pBits(pData),
        size(size)
    {
    }
    static _SColorFrame None() { return _SColorFrame(NULL, 0); }
} SColorFrame;


class CKinectConsole
{
#define  MEMBUFFER_SIZE  640*480*sizeof(LONG)
public:
    BYTE * const                             m_pRGBAFrameBits;

    CKinectConsole();
    ~CKinectConsole();
    HRESULT                                  CreateFirstConnected();
    SColorFrame                              DumpFrameColor();
    SColorFrame                              GetOneFrame();

private:
    INuiSensor *                             m_pNuiSensor;
    HANDLE                                   m_hNextColorFrameEvent;
    HANDLE                                   m_pColorStreamHandle;
    //std::vector<SColorFrame>                 m_vColorFrame;
    BYTE * const                             m_pFrameBits;
    
};

#endif // !__KINECT_CONSOLE_H__