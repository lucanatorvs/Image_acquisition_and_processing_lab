/*
 * dh0.cpp
 *
 *  Created on: Dec 3, 2021
 Author: Fidelis Theinert
 Reading DaHeng camera with OpenCV 4
 Link with:
 libopencv_core.so
 libopencv_imgproc.so
 libopencv_highgui.so
 */

#include <iostream>

// include OpenCV headers
#include <opencv.hpp>
#include <opencv2/imgproc/types_c.h>

#include "dh0.h"

#include "GxIAPI.h"
#include "GxImageProc.hpp"

using namespace std;
using namespace cv;


// ****************************************************************************
dh::dh(int camNum) {
// ****************************************************************************

	int retval = 0;

	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	uint32_t ui32DeviceNum = 0;
	size_t nSize = 0;
	bool bStreamTransferSize = false;
	bool bStreamTransferNumberUrb = false;

	// Initialize library
	emStatus = GXInitLib();
	if (emStatus != GX_STATUS_SUCCESS) {
		GetErrorString(emStatus);
		DH_VERIFY_EXIT(emStatus);
	}

	// Get device enumerated number
	emStatus = GXUpdateDeviceList(&ui32DeviceNum, 1000);
	if (emStatus != GX_STATUS_SUCCESS) {
		GetErrorString(emStatus);
		GXCloseLib();
		DH_VERIFY_EXIT(emStatus);
	}

	// If no device found, app exit
	if (ui32DeviceNum <= 0) {
		cout << "<No device found>" << endl;
		GXCloseLib();
		exit (-1);
	}

	// Open first device enumerated
	emStatus = GXOpenDeviceByIndex(1, &g_hDevice);
	if (emStatus != GX_STATUS_SUCCESS) {
		GetErrorString(emStatus);
		GXCloseLib();
		DH_VERIFY_EXIT(emStatus);
	}

	// Get string length of Vendor name
	emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME,
			&nSize);
	DH_VERIFY_EXIT(emStatus);

	// Get the type of Bayer conversion. whether is a color camera.
	emStatus = GXIsImplemented(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER,
			&g_bColorFilter);
	DH_VERIFY_EXIT(emStatus);

	// This app only support color cameras
	if (!g_bColorFilter) {
		cout << "<This app only supports color cameras! App Exit!>" << endl;
		GXCloseDevice(g_hDevice);
		g_hDevice = NULL;
		GXCloseLib();
		exit(-1);
	} else {
		emStatus = GXGetEnum(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER,
				&g_i64ColorFilter);
		DH_VERIFY_EXIT(emStatus);
	}

	emStatus = GXGetInt(g_hDevice, GX_INT_PAYLOAD_SIZE, &g_nPayloadSize);
	DH_VERIFY(emStatus);

	// Set acquisition mode to continuous
	emStatus = GXSetEnum(g_hDevice, GX_ENUM_ACQUISITION_MODE,
			GX_ACQ_MODE_CONTINUOUS);
	DH_VERIFY_EXIT(emStatus);

	// Set trigger mode off
	emStatus = GXSetEnum(g_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
	DH_VERIFY_EXIT(emStatus);

	// Set buffer quantity of acquisition queue
	uint64_t nBufferNum = ACQ_BUFFER_NUM;
	emStatus = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
	DH_VERIFY_EXIT(emStatus);

	emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE,
			&bStreamTransferSize);
	DH_VERIFY_EXIT(emStatus);

	if (bStreamTransferSize) {
		//Set size of data transfer block
		emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE,
		ACQ_TRANSFER_SIZE);
		DH_VERIFY_EXIT(emStatus);
	}

	emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB,
			&bStreamTransferNumberUrb);
	DH_VERIFY_EXIT(emStatus);

	if (bStreamTransferNumberUrb) {
		//Set qty. of data transfer block
		emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB,
		ACQ_TRANSFER_NUMBER_URB);
		DH_VERIFY_EXIT(emStatus);
	}

	// Sets the exposure mode to continuous automatic exposure
	emStatus = GXSetEnum(g_hDevice, GX_ENUM_EXPOSURE_AUTO,
			GX_EXPOSURE_AUTO_CONTINUOUS);
	DH_VERIFY_EXIT(emStatus);

	// Set Balance White Mode : Continuous
	emStatus = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO,
			GX_BALANCE_WHITE_AUTO_ONCE);
	DH_VERIFY_EXIT(emStatus);

	// Allocate the memory for pixel format transform
	PreForAcquisition();

	// Device start acquisition
	emStatus = GXStreamOn(g_hDevice);
	if (emStatus != GX_STATUS_SUCCESS) {
		//Release the memory allocated
		UnPreForAcquisition();
		DH_VERIFY_EXIT(emStatus);
	}

	camId = retval;

	colorMode = CAM_MODE_COL;
	numChannels = 3;
	expoMS = 100.0;

	initialized = true;
}

// ****************************************************************************
Mat* dh::setMode(int modeRes, int modeCol) {
// ****************************************************************************

	int width, height;

	if (initialized == false) {
		printInitError();
		return NULL;
	}

	switch (modeRes) {
	case CAM_RES_640_480:
		width = 640;
		height = 480;
		break;

	case CAM_RES_1024_768:
		width = 1024;
		height = 768;
		break;

	case CAM_RES_1280_720:
		width = 1280;
		height = 720;
		break;

	case CAM_RES_1280_960:
		width = 1280;
		height = 960;
		break;

	case CAM_RES_1280_1024:
		width = 1280;
		height = 1024;
		break;

	case CAM_RES_1920_1048:
		width = 1920;
		height = 1048;
		break;

	case CAM_RES_1920_1080:
		width = 1920;
		height = 1080;
		break;

	case CAM_RES_1920_1200:
		width = 1920;
		height = 1200;
		break;

	default:
		width = 640;
		height = 480;
		cout << "wrong resolution, using default 640 * 480" << endl;
		break;
	}

	rect_roi = Rect(0, 0, width, height);

	colorMode = modeCol;

	switch (modeCol) {
	case CAM_MODE_GREY:
		numChannels = 1;
		break;

	case CAM_MODE_COL:
	case CAM_MODE_COL3:
		numChannels = 3;
		break;

	case CAM_MODE_COL4:
		numChannels = 4;
		break;

	case CAM_MODE_RAW:
		numChannels = 1;
		break;

	default:
		colorMode = CAM_MODE_COL;
		numChannels = 3;
		cout << "wrong colormode, using default 3 * 8bit" << endl;
		break;
	}

//  cout << "ROI = offset x: " << startx << ", \toffset y: " << starty << endl;
//  cout << "ROI: width = " << width << ", height = " << height << endl;

	return &out;
}

// ****************************************************************************
int dh::captureFrame(Mat *frame) {
// ****************************************************************************
	if (initialized == false) {
		printInitError();
		return CAM_ERROR;
	}

	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	PGX_FRAME_BUFFER pFrameBuffer = NULL;

	emStatus = GXDQBuf(g_hDevice, &pFrameBuffer, 2000);
	if (emStatus != GX_STATUS_SUCCESS) {
		if (emStatus == GX_STATUS_TIMEOUT) {
			cout << "Camera Timeout!" << endl;
			exit(-1);
		} else {
			GetErrorString(emStatus);
		}
	}

	if (colorMode == CAM_MODE_RAW) {
//		cout << "col raw!" << endl;
//		cout << "create Mat 'out'" << endl;
		out = Mat(pFrameBuffer->nHeight, pFrameBuffer->nWidth, CV_8UC1,
				pFrameBuffer->pImgBuf);

	} else {
//		cout << "col col!" << endl;
		tempBuf = Mat(pFrameBuffer->nHeight, pFrameBuffer->nWidth, CV_8UC3,
				g_pRGBImageBuf);

		int nRet = PixelFormatConvert(pFrameBuffer);
		if (nRet != PIXFMT_CVT_SUCCESS) {
			cout << "PixelFormat Convert failed!\n";
		}

		if (colorMode == CAM_MODE_COL) {
			// OpenCV color-format is BGR, so convert it
			cvtColor(tempBuf, out, CV_RGB2BGR);
		} else {
			// colorMode == CAM_MODE_GREY
			cvtColor(tempBuf, out, CV_RGB2GRAY);
		}
	}

	// push back image-buffer
	emStatus = GXQBuf(g_hDevice, pFrameBuffer);
	if (emStatus != GX_STATUS_SUCCESS) {
		GetErrorString(emStatus);
		return CAM_ERROR;
	}

	// crop image to desired size
	rect_roi.x = out.cols / 2 - rect_roi.width / 2;
	rect_roi.y = out.rows / 2 - rect_roi.height / 2;

	if (rect_roi.x < 0)
		rect_roi.x = 0;
	if (rect_roi.y < 0)
		rect_roi.y = 0;
	if (rect_roi.width > out.cols - rect_roi.x)
		rect_roi.width = out.cols - rect_roi.x;
	if (rect_roi.height > out.rows - rect_roi.y)
		rect_roi.height = out.rows - rect_roi.y;

	*frame = out(rect_roi);

	return CAM_OK;
}

// ****************************************************************************
int dh::setExpoMs(double expoMS) {
// ****************************************************************************

	if (initialized == false) {
		printInitError();
		return CAM_ERROR;
	}

	if (expoMS < CAM_MIN_EXPOSURE_MS) {
		cout << "minimum expotime = " << CAM_MIN_EXPOSURE_MS << " ms" << endl;
		expoMS = CAM_MIN_EXPOSURE_MS;
//		return CAM_ERROR;
	}

	GX_STATUS status = GX_STATUS_SUCCESS;

	//Sets the exposure mode automatic exposure off
	GXSetEnum(g_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);

	status = GXSetFloat(g_hDevice, GX_FLOAT_EXPOSURE_TIME, expoMS * 1000.0);
//	cout << "time = " << expoTim << endl;

	if (status != GX_STATUS_SUCCESS)
		return CAM_ERROR;

	return CAM_OK;
}

// ****************************************************************************
int dh::getExpoMs(double *exposuretimeMS) {
// ****************************************************************************

	if (initialized == false) {
		printInitError();
		return CAM_ERROR;
	}

	GX_STATUS status = GX_STATUS_SUCCESS;

	//Sets the exposure mode automatic exposure off
	GXSetEnum(g_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);

	// retrieve actual exposuretime set
	status = GXGetFloat(g_hDevice, GX_FLOAT_EXPOSURE_TIME, exposuretimeMS);
//	cout << "time = " << exposuretimeMS << endl;

	*exposuretimeMS /= 1000.0;

	if (status != GX_STATUS_SUCCESS)
		return CAM_ERROR;

	return CAM_OK;
}

// ****************************************************************************
int dh::setWB(int mode) {
// ****************************************************************************

	if (initialized == false) {
		printInitError();
		return CAM_ERROR;
	}

	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	switch (mode) {
	case CAM_AUTO_WB_OFF:
		//Set Balance White Mode : Continuous
		emStatus = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO,
				GX_BALANCE_WHITE_AUTO_OFF);
		DH_VERIFY_EXIT(emStatus)
		;

		break;

	case CAM_AUTO_WB_ON:
		//Set Balance White Mode : Continuous
		emStatus = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO,
				GX_BALANCE_WHITE_AUTO_CONTINUOUS);
		DH_VERIFY_EXIT(emStatus)
		;

		break;
	}

	return CAM_OK;
}

// ****************************************************************************
int dh::getName(string *name) {
// ****************************************************************************

	char pszModelName[1000];
//	unsigned long int nSize = 1000;
	unsigned long int nSize = size_t(pszModelName);

	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	//Get Model name
	emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, pszModelName,
			&nSize);
	if (emStatus != GX_STATUS_SUCCESS) {
		DH_VERIFY_EXIT(emStatus);
		*name = (string) "noname";
		return CAM_ERROR;
	}

	*name = (string) pszModelName;

	return CAM_OK;
}

// ****************************************************************************
int dh::close(void) {
// ****************************************************************************

	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	//Device stop acquisition
	emStatus = GXStreamOff(g_hDevice);
	if (emStatus != GX_STATUS_SUCCESS) {
		//Release the memory allocated
		UnPreForAcquisition();
		DH_VERIFY_EXIT(emStatus);
	}

	//Release the resources and stop acquisition thread
	UnPreForAcquisition();

	//Close device
	emStatus = GXCloseDevice(g_hDevice);
	if (emStatus != GX_STATUS_SUCCESS) {
		GetErrorString(emStatus);
		g_hDevice = NULL;
		GXCloseLib();
		return CAM_ERROR;;
	}

	//Release libary
	emStatus = GXCloseLib();
	if (emStatus != GX_STATUS_SUCCESS) {
		GetErrorString(emStatus);
		return CAM_ERROR;;
	}

	initialized = false;

	return CAM_OK;
}

// ****************************************************************************
dh::~dh() {
// ****************************************************************************

//	cout << "release resources!" << endl;
}

// ****************************************************************************
//                          local private functions
// ****************************************************************************

// ****************************************************************************
void dh::printInitError(void) {
// ****************************************************************************
	cout << "Cam not initialized!" << endl;
}

//-------------------------------------------------
/**
 \brief Save PPM image
 \param ui32Width[in]       image width
 \param ui32Height[in]      image height
 \return void
 */
//-------------------------------------------------
void dh::SavePPMFile(uint32_t ui32Width, uint32_t ui32Height) {
	char szName[FILE_NAME_LEN] = { 0 };

	static int nRawFileIndex = 0;

	if (g_pRGBImageBuf != NULL) {
		FILE *phImageFile = NULL;
		snprintf(szName, FILE_NAME_LEN, "Frame_%d.ppm", nRawFileIndex++);
		phImageFile = fopen(szName, "wb");
		if (phImageFile == NULL) {
			cout << "Create or Open " << szName << " failed!" << endl;
			return;
		}
		//Save color image
		fprintf(phImageFile, "P6\n%u %u 255\n", ui32Width, ui32Height);
		fwrite(g_pRGBImageBuf, 1, g_nPayloadSize * 3, phImageFile);
		fclose(phImageFile);
		phImageFile = NULL;
		cout << "Saving " << szName << " succeeded!" << endl;
	} else {
		cout << "Saving " << szName << " failed!" << endl;
	}
}

//-------------------------------------------------
/**
 \brief Convert frame date to suitable pixel format
 \param pParam[in]           pFrameBuffer       FrameData from camera
 \return void
 */
//-------------------------------------------------
int dh::PixelFormatConvert(PGX_FRAME_BUFFER pFrameBuffer) {
	VxInt32 emDXStatus = DX_OK;

	// Convert RAW8 or RAW16 image to RGB24 image
	switch (pFrameBuffer->nPixelFormat) {
	case GX_PIXEL_FORMAT_BAYER_GR8:
	case GX_PIXEL_FORMAT_BAYER_RG8:
	case GX_PIXEL_FORMAT_BAYER_GB8:
	case GX_PIXEL_FORMAT_BAYER_BG8: {
		// Convert to the RGB image
		emDXStatus = DxRaw8toRGB24((unsigned char*) pFrameBuffer->pImgBuf,
				g_pRGBImageBuf, pFrameBuffer->nWidth, pFrameBuffer->nHeight,
//				RAW2RGB_NEIGHBOUR3, DX_PIXEL_COLOR_FILTER(g_i64ColorFilter),
				RAW2RGB_ADAPTIVE, DX_PIXEL_COLOR_FILTER(g_i64ColorFilter),
				false);
		if (emDXStatus != DX_OK) {
			cout << "DxRaw8toRGB24 Failed, Error Code: " << emDXStatus << endl;
			return PIXFMT_CVT_FAIL;
		}
		break;
	}
	case GX_PIXEL_FORMAT_BAYER_GR10:
	case GX_PIXEL_FORMAT_BAYER_RG10:
	case GX_PIXEL_FORMAT_BAYER_GB10:
	case GX_PIXEL_FORMAT_BAYER_BG10:
	case GX_PIXEL_FORMAT_BAYER_GR12:
	case GX_PIXEL_FORMAT_BAYER_RG12:
	case GX_PIXEL_FORMAT_BAYER_GB12:
	case GX_PIXEL_FORMAT_BAYER_BG12: {
		// Convert to the Raw8 image
		emDXStatus = DxRaw16toRaw8((unsigned char*) pFrameBuffer->pImgBuf,
				g_pRaw8Image, pFrameBuffer->nWidth, pFrameBuffer->nHeight,
				DX_BIT_2_9);
		if (emDXStatus != DX_OK) {
			cout << "DxRaw16toRaw8 Failed, Error Code: " << emDXStatus << endl;
			return PIXFMT_CVT_FAIL;
		}
		// Convert to the RGB24 image
		emDXStatus = DxRaw8toRGB24(g_pRaw8Image, g_pRGBImageBuf,
				pFrameBuffer->nWidth, pFrameBuffer->nHeight, RAW2RGB_NEIGHBOUR3,
				DX_PIXEL_COLOR_FILTER(g_i64ColorFilter), false);
		if (emDXStatus != DX_OK) {
			cout << "DxRaw8toRGB24 Failed, Error Code: " << emDXStatus << endl;
			return PIXFMT_CVT_FAIL;
		}
		break;
	}
	default: {
		cout << "Error : PixelFormat of this camera is not supported\n";
		return PIXFMT_CVT_FAIL;
	}
	}
	return PIXFMT_CVT_SUCCESS;
}

//-------------------------------------------------
/**
 \brief Allocate the memory for pixel format transform
 \return void
 */
//-------------------------------------------------
void dh::PreForAcquisition() {
	g_pRGBImageBuf = new unsigned char[g_nPayloadSize * 3];
	g_pRaw8Image = new unsigned char[g_nPayloadSize];

	return;
}

//-------------------------------------------------
/**
 \brief Release the memory allocated
 \return void
 */
//-------------------------------------------------
void dh::UnPreForAcquisition() {
	//Release resources
	if (g_pRaw8Image != NULL) {
		delete[] g_pRaw8Image;
		g_pRaw8Image = NULL;
	}
	if (g_pRGBImageBuf != NULL) {
		delete[] g_pRGBImageBuf;
		g_pRGBImageBuf = NULL;
	}

	return;
}

//----------------------------------------------------------------------------------
/**
 \brief  Get description of input error code
 \param  emErrorStatus  error code

 \return void
 */
//----------------------------------------------------------------------------------
void dh::GetErrorString(GX_STATUS emErrorStatus) {
	char *error_info = NULL;
	size_t size = 0;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	// Get length of error description
	emStatus = GXGetLastError(&emErrorStatus, NULL, &size);
	if (emStatus != GX_STATUS_SUCCESS) {
		cout << "<Error when calling GXGetLastError>" << endl;
		return;
	}

	// Alloc error resources
	error_info = new char[size];
	if (error_info == NULL) {
		cout << "<Failed to allocate memory>" << endl;
		return;
	}

	// Get error description
	emStatus = GXGetLastError(&emErrorStatus, error_info, &size);
	if (emStatus != GX_STATUS_SUCCESS) {
		cout << "<Error when calling GXGetLastError>" << endl;
	} else {
		cout << error_info << endl;
	}

	// Release error resources
	if (error_info != NULL) {
		delete[] error_info;
		error_info = NULL;
	}
}

