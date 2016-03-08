// KinectConsole.cpp : 定义控制台应用程序的入口点。
//
#include "../stdafx.h"
#include "../include/KinectConsole.h"

//int main()
//{
//    CKinectConsole kinect;
//    if (-1 == kinect.Run())
//    {
//        return -1;
//    }
//    return 0;
//}

CKinectConsole::CKinectConsole():
    m_pFrameBits((BYTE *)malloc(MEMBUFFER_SIZE)),
    m_pRGBAFrameBits((BYTE *)malloc(MEMBUFFER_SIZE))
{
    if (NULL == m_pFrameBits || NULL == m_pRGBAFrameBits)
    {
        printf("Fatal Error: NO Memory Allocated for KinectConsole instance!\n");
        delete this;
    }
}

CKinectConsole::~CKinectConsole()
{
    free(m_pFrameBits);
    free(m_pRGBAFrameBits);
}
HRESULT CKinectConsole::CreateFirstConnected()
{
    INuiSensor * pNuiSensor;
    HRESULT hr;

    int iSensorCount = 0;
    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_pNuiSensor = pNuiSensor;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }

    if (NULL != m_pNuiSensor)
    {
        // Initialize the Kinect and specify that we'll be using color
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
        if (SUCCEEDED(hr))
        {
            // Create an event that will be signaled when color data is available
            m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            // Open a color image stream to receive color frames
            hr = m_pNuiSensor->NuiImageStreamOpen(
                NUI_IMAGE_TYPE_COLOR,
                NUI_IMAGE_RESOLUTION_640x480,
                0,
                2,
                m_hNextColorFrameEvent,
                &m_pColorStreamHandle);
        }
    }

    if (NULL == m_pNuiSensor || FAILED(hr))
    {
        //printf("No Kinect found!\n");
        return E_FAIL;
    }

    return hr;
}


SColorFrame CKinectConsole::GetOneFrame()
{
    const int eventCount = 1;
    HANDLE hEvents[eventCount];

    hEvents[0] = m_hNextColorFrameEvent;
    MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);
    if (NULL == m_pNuiSensor)
    {
        return SColorFrame::None();
    }

    if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextColorFrameEvent, 0))
    {
        return DumpFrameColor();
    }
}

SColorFrame CKinectConsole::DumpFrameColor()
{
    HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;

        // Attempt to get the color frame
        hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        return SColorFrame::None();
    }

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);
    
    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        // 在这里 copy 一份内存数据,pBits不属于本地环境,它只是kinect数据的引用
        if (MEMBUFFER_SIZE >= LockedRect.size)
            memcpy(m_pFrameBits, LockedRect.pBits, LockedRect.size);
        else
            printf("Frame bit size is beyond MEMBUFFER_SIZE\n"), hr = STATUS_STACK_BUFFER_OVERRUN;
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);

    assert(STATUS_STACK_BUFFER_OVERRUN != hr);
    return SColorFrame(m_pFrameBits, LockedRect.size);
}
