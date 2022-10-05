/*
 * dh0.h

 Created on: 2021/12
 Author: Fidelis Theinert

 Wrapper for using DaHeng Camera with OpenCV 3
 Link with OpenCv libraries:
 libopencv_core.so
 libopencv_imgproc.so
 libopencv_highgui.so
 */


#ifndef SRC_DH0_H_
#define SRC_DH0_H_

#include <string>
#include <iostream>

#include "GxIAPI.h"
#include "GxImageProc.hpp"
#include <opencv2/opencv.hpp>

// Namespace for OpenCV
using namespace cv;
using namespace std;


#define CAM_OK             0
#define CAM_ERROR         -1
//#define CAM_NOT_SUPPORTED -2
//#define CAM_USING_DEFAULT -3
//#define CAM_BUSY          -4
//
#define ACQ_BUFFER_NUM          5               ///< Acquisition Buffer Qty.
#define ACQ_TRANSFER_SIZE       (64 * 1024)     ///< Size of data transfer block
#define ACQ_TRANSFER_NUMBER_URB 64              ///< Qty. of data transfer block
#define FILE_NAME_LEN           50              ///< Save image file name length

#define PIXFMT_CVT_FAIL             -1             ///< PixelFormatConvert fail
#define PIXFMT_CVT_SUCCESS          0              ///< PixelFormatConvert success

// Show error message
#define DH_VERIFY(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
    {                                      \
        GetErrorString(emStatus);          \
    }

// Show error message, close device and lib and exit application
#define DH_VERIFY_EXIT(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
    {                                      \
        GetErrorString(emStatus);          \
        GXCloseDevice(g_hDevice);          \
        g_hDevice = NULL;                  \
        GXCloseLib();                      \
        printf("<App Exit!>\n");           \
        exit (emStatus);                   \
    }


enum CAM_RES_MODE {
  CAM_RES_640_480,
  CAM_RES_1024_768,
  CAM_RES_1280_720,
  CAM_RES_1280_960,
  CAM_RES_1280_1024,
  CAM_RES_1920_1048,
  CAM_RES_1920_1080,
  CAM_RES_1920_1200,

  CAM_RES_NUM
};

enum CAM_COL_MODE {
  CAM_MODE_GREY,
  CAM_MODE_COL,
  CAM_MODE_COL3,  // same as MODE_COL
  CAM_MODE_COL4,
  CAM_MODE_RAW,

  CAM_MODE_NUM
};

#define CAM_AUTO_WB_ON   1
#define CAM_AUTO_WB_OFF  0

// maximum time to wait for exposure before timeout occurs
#define CAM_MAX_EXPOSURE_MS 1000.0
#define CAM_MIN_EXPOSURE_MS 1.0

class dh {
  public:
    dh (int camId);                            // constructor camID: 0 default, 1 first, 2 second ACE-cam
    int setWB (int);                           // CAM_NOT_SUPPORTED
    int setExpoMs (double ms);                 // set exposure time in milliseconds
    int getExpoMs (double * exposuretime);       // get the actual exposuretime
//    double getExpoInc (void);                  // get exposure increment in milliseconds
//    double setFrameRate (double fps);          // CAM_NOT_SUPPORTED
//    double getPixelClock (void);               // CAM_NOT_SUPPORTED
//    int setPixelClock (double pixClk);         // CAM_NOT_SUPPORTED
    Mat* setMode (int modeRes, int modeCol);   // CAM_RES_MODE and CAM_COL_MODE
    int getName (string * name);               // retrieve camera name

    int captureFrame (Mat *frame);             // retrieve single frame
    int close(void);                           // stop grabbing

    virtual ~dh ();                            // destructor

  private:

    // private methods
    // Allocate the memory for pixel format transform
    void PreForAcquisition();

    // Release the memory allocated
    void UnPreForAcquisition();

    // Convert frame date to suitable pixel format
    int PixelFormatConvert(PGX_FRAME_BUFFER);

    // Save one frame to PPM image file
    void SavePPMFile(uint32_t, uint32_t);

    // Get description of error
    void GetErrorString(GX_STATUS);

    void printInitError (void);


    // private variables
    GX_DEV_HANDLE g_hDevice = NULL;                     ///< Device handle
    bool g_bColorFilter = false;                        ///< Color filter support flag
    int64_t g_i64ColorFilter = GX_COLOR_FILTER_NONE;    ///< Color filter of device
    bool g_bAcquisitionFlag = false;                    ///< Thread running flag
    bool g_bSavePPMImage = false;                       ///< Save raw image flag

    unsigned char *g_pRGBImageBuf = NULL;               ///< Memory for RAW8toRGB24
    unsigned char *g_pRaw8Image = NULL;                 ///< Memory for RAW16toRAW8

    int64_t g_nPayloadSize = 0;                         ///< Payload size

    Mat tempBuf;
    Mat out;

    Rect rect_roi = Rect (0, 0, 640, 480);

    // init of camera
    int initialized = false;
    int camId;

    double expoMS; //exposure-time in ms
    int colorMode;
    int numChannels;
};

#endif /* SRC_DH0_H_ */
