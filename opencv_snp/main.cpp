#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
int main()
{
	// Read the image (in BGR)
    Mat img = imread("pixerror.png", IMREAD_COLOR);
    if(img.empty())
    {
        std::cout << "Could not read the image: " << std::endl;
        return 1;
    }
    
    // Split the image into 3 new images for blue, green and red.
	Mat Bands[3],merged;
	split(img, Bands);
	
	// To merge the image, create a vector that contains the channels and write the output to 'merged'
	std::vector<Mat> channels = {Bands[0],Bands[1],Bands[2]};
	merge(channels,merged);
    
    // Display the image until q is pressed
    imshow("Display window", merged);
    int k = waitKey(0); // Wait for a keystroke in the window
    if(k == 'q')
    {
        imwrite("starry_night.png", img);
    }
    return 0;
}
