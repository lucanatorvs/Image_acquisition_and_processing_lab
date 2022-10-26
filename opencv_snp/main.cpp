#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

Mat filter(Mat& input) {
	Size s = input.size();
	int h, w;
	uint32_t sum;
	
	std::cout << "Input type was : " << input.type() << std::endl;
	uint8_t out[s.height][s.width];
	
	std::cout << s.height << " " << s.width << std::endl;
	
	for (h=0; h<s.height; h++){
		for(w=0; w<s.width; w++){
			sum = 0;
			std::vector<uint8_t> median;
			
			for (int _x = -1; _x < 2; ++_x)
			{
				for (int _y = -1; _y < 2; ++_y)
				{
					int idx_y = h + _y;
					int idx_x = w + _x;
					
					if (idx_x < 0 || idx_x > s.width)
						break;
						
					if (idx_y < 0 || idx_y > s.height)
						break;
					
					median.push_back(input.at<uint8_t>(idx_y,idx_x));
				}
			}
			std::sort(std::begin(median), std::end(median));
					
			for (auto it = median.begin(); it != median.end(); ++it) {
				sum = median.at(median.size()/2);
			}
			out[h][w]=(uint8_t)sum;
		}
	}
	Mat result = Mat(s.height, s.width, CV_8U, out); //or maybe CV_8UC1?
	Size _s = result.size();
	std::cout << "done: " << _s.height << " " << _s.width << std::endl;
	return result;
}


int main() {
	// Read the image (in BGR)
    Mat img = imread("pixerror.png", IMREAD_COLOR);
    if(img.empty())
    {
        std::cout << "Could not read the image: " << std::endl;
        return 1;
    }
    
    // Split the image into 3 new images for blue, green and red.
    std::cout << "Splitting channels: " << std::endl;
	Mat Bands[3];
	split(img, Bands);

	Mat Bands_filtered[3];
	Bands_filtered[0] = filter(Bands[0]);
	Bands_filtered[1] = filter(Bands[1]);
	Bands_filtered[2] = filter(Bands[2]);
	
	//mix the channels together
	Mat bgr( img.rows, img.cols, CV_8UC3 );
	Mat out[] = {Bands_filtered[0],Bands_filtered[1], Bands_filtered[2]};
	
	/*
	Mat bgra( 100, 100, CV_8UC4, Scalar(255,0,0,255) );
	Mat bgr( bgra.rows, bgra.cols, CV_8UC3 );
	Mat alpha( bgra.rows, bgra.cols, CV_8UC1 );
	// forming an array of matrices is a quite efficient operation,
	// because the matrix data is not copied, only the headers
	Mat out[] = { bgr, alpha };
	// bgra[0] -> bgr[2], bgra[1] -> bgr[1],
	// bgra[2] -> bgr[0], bgra[3] -> alpha[0]
	int from_to[] = { 0,2, 1,1, 2,0, 3,3 };
	mixChannels( &bgra, 1, out, 2, from_to, 4 );
	*/
	
	
	// To merge the image, create a vector that contains the channels and write the output to 'merged'
	std::cout << "Merging: " << std::endl;
	std::vector<Mat> channels = {Bands_filtered[0],Bands_filtered[1],Bands_filtered[2]};
	//std::vector<Mat> channels = {Bands[0],Bands[1],Bands[2]};
	//merge(channels, merged); //replace this with mixChannels idk
    
    
    // Display the image until q is pressed
    std::cout << "Displaying result: " << std::endl;
    //imshow("Display window", merged);
    imshow("Display window", Bands[0]);
    waitKey(0); // Wait for a keystroke in the window
    imshow("Display window", Bands[1]);
    waitKey(0); // Wait for a keystroke in the window
    imshow("Display window", Bands[2]);
    waitKey(0); // Wait for a keystroke in the window
    imshow("Display window", Bands_filtered[0]);
    waitKey(0); // Wait for a keystroke in the window
    return 0;
}
