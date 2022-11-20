#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#define BGR_BLUE  0
#define BGR_GREEN 1
#define BGR_RED   2


using namespace cv;

void deBayer(Mat *rawImg, Mat *outImg) {
	// the Daheng camera datasheet specifies a GRBG pattern (page 88). Our input values will be 8 bit
	// Split 1 channel image into 3 channels according to bayer pattern
	//   0  1
	//0  G  R
	//1  B  G
	
	//I will create a new "Mat" with OpenCV that contains three channels. The splitting and processing is done by hand.
	if (rawImg->type() != CV_8UC1)
		throw("Sorry, only 1 8-bit channel should be used");
	
	(*outImg) = cv::Mat::zeros(rawImg->rows, rawImg->cols, CV_8UC3); // Fill output buffer with zeros with the correct geometry
	
	//Size s = rawImg->size(); I will be using rows and colums rather than height and width
	long row, col;
	
	std::cout << "Bayer splitting to 3 channels" << std::endl;
	
	for(row=0; row<rawImg->rows; row++){	//todo: make this evaluation smaller to increase speed
		for(col=0; col<rawImg->cols; col++){
			if (row % 2 == 0 && col % 2 == 0) //odd row, odd column
				outImg->at<Vec3b>(row, col).val[BGR_GREEN] = rawImg->at<uint8_t>(row,col);
			if (row % 2 == 0 && col % 2 == 1) //odd row, even column
				outImg->at<Vec3b>(row, col).val[BGR_RED] = rawImg->at<uint8_t>(row,col);
			if (row % 2 == 1 && col % 2 == 0) //even row, odd column
				outImg->at<Vec3b>(row, col).val[BGR_BLUE] = rawImg->at<uint8_t>(row,col);
			if (row % 2 == 1 && col % 2 == 1) //even row, even column
				outImg->at<Vec3b>(row, col).val[BGR_GREEN] = rawImg->at<uint8_t>(row,col);			
		}
	}

	// Interpolate green channel with a 3x3 kernel
	std::cout << "Interpolating green channel" << std::endl;

	for(row=0; row<rawImg->rows; row++){	//todo: make this evaluation smaller to increase speed
		for(col=0; col<rawImg->cols; col++){
			if (row % 2 == 0 && col % 2 == 0) //odd row, odd column
				outImg->at<Vec3b>(row, col).val[BGR_GREEN] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;
			if (row % 2 == 0 && col % 2 == 1) //odd row, even column
				outImg->at<Vec3b>(row, col).val[BGR_GREEN] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;
			if (row % 2 == 1 && col % 2 == 0) //even row, odd column
				outImg->at<Vec3b>(row, col).val[BGR_GREEN] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;
			if (row % 2 == 1 && col % 2 == 1) //even row, even column
				outImg->at<Vec3b>(row, col).val[BGR_GREEN] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;			
		}
	}
	
	// Interpolate red and blue channels with a 3x3 kernel
	std::cout << "Interpolating red and blue channels" << std::endl;

	for(row=0; row<rawImg->rows; row++){	//todo: make this evaluation smaller to increase speed
		for(col=0; col<rawImg->cols; col++){
			if (row % 2 == 0 && col % 2 == 0) //odd row, odd column
				outImg->at<Vec3b>(row, col).val[BGR_RED] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;
			if (row % 2 == 0 && col % 2 == 1) //odd row, even column
				outImg->at<Vec3b>(row, col).val[BGR_BLUE] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;
			if (row % 2 == 1 && col % 2 == 0) //even row, odd column
				outImg->at<Vec3b>(row, col).val[BGR_RED] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;
			if (row % 2 == 1 && col % 2 == 1) //even row, even column
				outImg->at<Vec3b>(row, col).val[BGR_BLUE] = (rawImg->at<uint8_t>(row,col-1) + rawImg->at<uint8_t>(row,col+1) + rawImg->at<uint8_t>(row-1,col) + rawImg->at<uint8_t>(row+1,col))/4;			
		}
	}

	// Convert to RGB
	std::cout << "Converting to RGB" << std::endl;

	for(row=0; row<rawImg->rows; row++){	//todo: make this evaluation smaller to increase speed
		for(col=0; col<rawImg->cols; col++){
			outImg->at<Vec3b>(row, col).val[RGB_RED] = outImg->at<Vec3b>(row, col).val[BGR_RED];
			outImg->at<Vec3b>(row, col).val[RGB_GREEN] = outImg->at<Vec3b>(row, col).val[BGR_GREEN];
			outImg->at<Vec3b>(row, col).val[RGB_BLUE] = outImg->at<Vec3b>(row, col).val[BGR_BLUE];
		}
	}

}

int main() {
	// Read the image (in 8bit grayscale)
    Mat img = imread("test_RAW.png", CV_8UC1);
    if(img.empty()) {
        std::cout << "Could not read the image: " << std::endl;
        return 1;
    }
    
    Mat result;
    deBayer(&img, &result);
    
    
    imshow("Display window", result);
    waitKey(0); // Wait for a keystroke in the window
    return 0;
}
