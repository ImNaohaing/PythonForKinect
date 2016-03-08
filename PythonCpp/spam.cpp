// spam.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "include\KinectConsole.h"
#include <fstream>

#define C_DEBUG 


static PyObject * spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;
    static CKinectConsole kconsole;
    static bool bConnected = false;

    if (!PyArg_ParseTuple(args, "s", &command))  // 转换tuple至C
        return NULL;

    if (0 == strcmp(command, "connect"))
    {
        if (SUCCEEDED(kconsole.CreateFirstConnected()))
        {
            bConnected = true;
            printf("Kinect connected.\n");
            sts = S_OK;
        }
        else
        {
            printf("No Kinect found.\n");
            return NULL;
        }
    }
    if (0 == strcmp(command, "getcolorframe"))
    {
        SColorFrame aframe = kconsole.GetOneFrame();
        /*for (int i=0; i < 640 * 480; ++i)
        {
            char chrarray[] = { aframe.pBits[i * 4 + 2] ,aframe.pBits[i * 4 + 1] ,aframe.pBits[i * 4 + 0] ,aframe.pBits[i * 4 + 3] };
            memcpy(&kconsole.m_pRGBAFrameBits[i * 4], &chrarray, 4);
        }*/
        return Py_BuildValue("{s:s#,s:s}", "data", aframe.pBits, aframe.size, "format", "BGRA");
        sts = S_OK;

    }

    return Py_BuildValue("i", sts);
}

static PyMethodDef SpamMethods[] = {
    {"system", spam_system, METH_VARARGS,"Execute a command"},
    {NULL, NULL, 0, NULL}       /* Sentinel */
};

PyMODINIT_FUNC initspam(void)
{
    (void) Py_InitModule("spam", SpamMethods);
}