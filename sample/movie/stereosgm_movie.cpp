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
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <libsgm.h>

#include "demo.h"
#include "renderer.h"
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

	int channel_flag = 0;
	int size_flag = 0;
	if (argc < 3) {
		std::cerr << "usage: stereosgm left_img_fmt right_img_fmt [disp_size] [max_frame_num]" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::string left_filename_fmt, right_filename_fmt;
	left_filename_fmt = argv[1];
	right_filename_fmt = argv[2];
	struct timeval start, end;
    long mtime, seconds, useconds;

	// dangerous
	char buf[1024];
	sprintf(buf, left_filename_fmt.c_str(), 0);
	cv::Mat left = cv::imread(buf, -1);
	sprintf(buf, right_filename_fmt.c_str(), 0);
	cv::Mat right = cv::imread(buf, -1);

	cv::Mat left1;
	cv::Mat right1;
	cv::Mat left2;
	cv::Mat right2;

	int width_ = left.cols;
	int height_ = left.rows;
	int width = left.cols;
	int height = left.rows;

	if ( width_ % 2 != 0 || height_ % 2 != 0 ) {
		if ( width_ % 2 != 0 ) {
			std::cout << "width must be even" << std::endl;		
			width = width_ - 1;
		}
		else if( height_ % 2 != 0 ) {
			std::cout << "height must be even" << std::endl;		
			height = height_ - 1;	
		}
		cv::Mat srcROIl( left, cv::Rect(0,0,width,height));
		srcROIl.copyTo(left1);
		cv::Mat srcROIr( right, cv::Rect(0,0,width,height));
		srcROIr.copyTo(right1);
		size_flag = 2;
	}
	else {
		left1 = left;
		right1 = right;
		size_flag = 1;
	}

	int disp_size = 64;
	if (argc >= 4) {
		disp_size = atoi(argv[3]);
	}

	int max_frame = 200;
	if(argc >= 5) {
		max_frame = atoi(argv[4]);
	}


	if (left1.size() != right1.size() || left1.type() != right1.type()) {
		std::cerr << "mismatch input image size" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	int bits = 8;

	string ty =  type2str( left.type() );
	printf("Matrix: %s %dx%d \n", ty.c_str(), left1.cols, left1.rows );

	switch (left.type()) {
	case CV_8UC1: bits = 8; left2 = left1; right2 = right1; channel_flag = 1; break;
	case CV_16UC1: bits = 16; left2 = left1; right2 = right1; channel_flag = 1; break;
	default:
		std::cerr << "invalid input image color format" << left.type() << " So now converting to gray picture: " << std::endl;
		channel_flag = 2;
		cvtColor(left1, left2, CV_RGB2GRAY);
		cvtColor(right1, right2, CV_RGB2GRAY);
	}

	cudaGLSetGLDevice(0);

	SGMDemo demo(width, height);

	if (demo.init()) {
		printf("fail to init SGM Demo\n");
		std::exit(EXIT_FAILURE);
	}

	sgm::StereoSGM ssgm(width, height, disp_size, bits, 16, sgm::EXECUTE_INOUT_HOST2CUDA);

	Renderer renderer(width, height);
	
	uint16_t* d_output_buffer = NULL;

	int frame_no = 0;
	while (!demo.should_close()) {

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		if (frame_no == max_frame) { frame_no = 0; }

		sprintf(buf, left_filename_fmt.c_str(), frame_no);
		cv::Mat left = cv::imread(buf, -1);
		sprintf(buf, right_filename_fmt.c_str(), frame_no);
		cv::Mat right = cv::imread(buf, -1);

		if( size_flag == 2) {
			cv::Mat srcROIl( left, cv::Rect(0,0,width,height));
			srcROIl.copyTo(left1);
			cv::Mat srcROIr( right, cv::Rect(0,0,width,height));
			srcROIr.copyTo(right1);
		}
		else if( size_flag == 1) {
			left1 = left;
			right1 = right;
		}

		if(channel_flag == 1){
			left2 = left1;
			right2 = right1;
		}
		else if(channel_flag == 2){
			cvtColor(left1, left2, CV_RGB2GRAY);
			cvtColor(right1, right2, CV_RGB2GRAY);
		}
		if (left1.size() == cv::Size(0, 0) || right1.size() == cv::Size(0, 0)) {
			max_frame = frame_no;
			frame_no = 0;
			continue;
		}

		gettimeofday(&start, NULL);
		ssgm.execute(left2.data, right2.data, (void**)&d_output_buffer); // , sgm::DST_TYPE_CUDA_PTR, 16);
		gettimeofday(&end, NULL);
		seconds  = end.tv_sec  - start.tv_sec;
	    useconds = end.tv_usec - start.tv_usec;

	    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

	    std::cout << "Elapsed time: " << mtime << "milliseconds" << std::endl;

		switch (demo.get_flag()) {
		case 0:
			{
				renderer.render_input((uint16_t*)left2.data);
			}
			break;
		case 1:
			renderer.render_disparity(d_output_buffer, disp_size);
			break;
		case 2:
			renderer.render_disparity_color(d_output_buffer, disp_size);
			break;
		}
		
		demo.swap_buffer();
		frame_no ++;
	}
}
