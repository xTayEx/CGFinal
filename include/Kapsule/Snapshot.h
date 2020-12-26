#pragma once
#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#include <glad/glad.h>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>

namespace Kapsule {
	using namespace std;
	void getSnapshot(int width, int height, const string& path)
	{
		cv::Mat pixels(height, width, CV_8UC3);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data);
		cv::Mat fliped(height, width, CV_8UC3);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				fliped.at<cv::Vec3b>(y, x)[2] = pixels.at<cv::Vec3b>(height - y - 1, x)[0];
				fliped.at<cv::Vec3b>(y, x)[1] = pixels.at<cv::Vec3b>(height - y - 1, x)[1];
				fliped.at<cv::Vec3b>(y, x)[0] = pixels.at<cv::Vec3b>(height - y - 1, x)[2];
			}
		}
		cv::imwrite(path, fliped);
	}
};
#endif // __SNAPSHOT_H__
