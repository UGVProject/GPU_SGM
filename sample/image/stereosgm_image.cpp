/*
Copyright 2016 fixstars

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <libsgm.h>
using namespace std;

string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}
int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "usage: stereosgm left_img right_img [disp_size]" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	struct timeval start, end;
    long mtime, seconds, useconds;

	//cv::Mat left = cv::imread(argv[1], -1);
	//cv::Mat right = cv::imread(argv[2], -1);
	cv::Mat left1 = cv::imread(argv[1], -1);
	cv::Mat right1 = cv::imread(argv[2], -1);

	cv::Mat left;
	cv::Mat right;

	string ty =  type2str( left1.type() );
	printf("Matrix: %s %dx%d \n", ty.c_str(), left1.cols, left1.rows );

	int disp_size = 64;
	if (argc >= 4) {
		disp_size = atoi(argv[3]);
	}

	if (left1.size() != right1.size() || left1.type() != right1.type()) {
		std::cerr << "mismatch input image size" << std::endl;
		std::exit(EXIT_FAILURE);
	}



	int bits = 8;

	switch (left1.type()) {
	case CV_8UC1: bits = 8; left = left1; right = right1; break;
	case CV_16UC1: bits = 16; left = left1; right = right1; break;
	default:
		std::cerr << "invalid input image color format" << left1.type() << " So now converting to gray picture: " << std::endl;
		//std::exit(EXIT_FAILURE);
		cvtColor(left1, left, CV_RGB2GRAY);
		cvtColor(right1, right, CV_RGB2GRAY);
		//bits = left.type();
		//string ty1 =  type2str( left.type() );
		//printf("Matrix: %s %dx%d \n", ty1.c_str(), left.cols, left.rows );
		//printf("Matrix: %d %dx%d \n", left.type(), left.cols, left.rows );
	}

	sgm::StereoSGM ssgm(left1.cols, left1.rows, disp_size, bits, 8, sgm::EXECUTE_INOUT_HOST2HOST);

	cv::Mat output(cv::Size(left1.cols, left1.rows), CV_8UC1);
	
	gettimeofday(&start, NULL);

	ssgm.execute(left.data, right.data, (void**)&output.data);

	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    std::cout << "Elapsed time: " << mtime << "milliseconds" << std::endl;

	// show image
	cv::imshow("image", output * 256 / disp_size);
	
	int key = cv::waitKey();
	int mode = 0;
	while (key != 27) {
		if (key == 's') {
			mode += 1;
			if (mode >= 3) mode = 0;

			switch (mode) {
			case 0:
				{
					cv::setWindowTitle("image", "disparity");
					cv::imshow("image", output * 256 / disp_size);
					break;
				}
			case 1:
				{
					cv::Mat m;
					cv::applyColorMap(output * 256 / disp_size, m, cv::COLORMAP_JET);
					cv::setWindowTitle("image", "disparity color");
					cv::imshow("image", m);
					break;
				}
			case 2:
				{
					cv::setWindowTitle("image", "input");
					cv::imshow("image", left);
					break;
				}
			}
		}
		key = cv::waitKey();
	}
}
