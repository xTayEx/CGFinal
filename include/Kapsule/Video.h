#pragma once
#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <glad/glad.h>

//#include <thread>
//#include <mutex>
//#include <chrono>
#include <vector>

namespace Kapsule {
	using namespace std;
	static int fid = 0;
	static cv::VideoWriter outputVideo;
	static vector<cv::Mat> frameQueue;
	bool isSaved;

	static int width;
	static int height;
	static int totalFrame;

	void openVideo(int _width, int _height, int _totalFrame)
	{
		fid = 0;
		frameQueue.clear();
		width = _width;
		height = _height;
		totalFrame = _totalFrame;
		isSaved = false;
		outputVideo.open("video001.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30.0, cv::Size(width, height));
	}

	void saveVideoFrame(int height, int width)
	{
		if (fid >= totalFrame) return ;
		cv::Mat pixels(height, width, CV_8UC3);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data);
		frameQueue.emplace_back(pixels);
		fid++;
	}

	bool isCompleteRecording()
	{ 
		return fid >= totalFrame;
	}

	void saveAsVideo()
	{
		if (isSaved) return;
		size_t siz = frameQueue.size();
		for (size_t i = 0; i < siz; i++) {
			cerr << "frame " << i << "\n";
			cv::Mat cvPixels(height, width, CV_8UC3);
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					 cvPixels.at<cv::Vec3b>(y, x)[2] = frameQueue[i].at<cv::Vec3b>(height - y - 1, x)[0];
					 cvPixels.at<cv::Vec3b>(y, x)[1] = frameQueue[i].at<cv::Vec3b>(height - y - 1, x)[1];
					 cvPixels.at<cv::Vec3b>(y, x)[0] = frameQueue[i].at<cv::Vec3b>(height - y - 1, x)[2];
				}
			}
			outputVideo << cvPixels;
		}
		vector<cv::Mat> tmp;
		frameQueue.swap(tmp);
		isSaved = true;
		outputVideo.release();
		cerr << "Save as video done!\n";
	}

}

#endif // !__VIDEO_H__
