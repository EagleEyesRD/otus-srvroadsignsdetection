#ifndef IMAGE_CONVERTER_HPP
#define IMAGE_CONVERTER_HPP

#include "../FrameOptions.h"
#include <iostream>
#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

//#define use_cvtColor 1
enum cvConvertType {
	OpenCV_YCrCb2RGB,
	YUV2RGBforHDTV_BT601,
	YUV2RGBforHDTV_BT709,
	YCbCr2RGB,
	YUV420toRGB
};
enum ColorSpace {
	CS_YUV444,
	CS_YUV420
};
class ImageConvertor {
public:
	ImageConvertor();
	~ImageConvertor();
	cv::Mat readImage(FrameOptions _fo);
private:
	std::string rootDir;
	cv::Mat yuv_nv21_to_rgb_new(uchar yuv[], int width, int height);
	uchar CalcByte(long chVal);
};

#endif //IMAGE_CONVERTER_HPP