#include "ImageConverter.h"

ImageConvertor::ImageConvertor() { rootDir = "\\\\10.18.23.3\\shuttle-data\\"; }
ImageConvertor::~ImageConvertor() {}

uchar ImageConvertor::CalcByte(long chVal)
{
	if (chVal > 8355840)
		return 255;
	else if (chVal < 0)
		return 0;
	else
		return (char)((chVal + 16384) >> 15);
};

cv::Mat ImageConvertor::yuv_nv21_to_rgb_new(uchar yuv[], int width, int height) {
	int total = width * height;
	int quadro = (width / 2) * (height / 2);
	int yInx = 0;
	int uInx = 0;
	int vInx = 0;
	uchar* yMat = new uchar[total];
	uchar* uMat = new uchar[quadro];
	uchar* vMat = new uchar[quadro];
	//
	uchar* U = new uchar[total];
	uchar* V = new uchar[total];
	long R, G, B;
	long uvR, uvG, uvB;
	uchar r, g, b;
	uchar y, u, v;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			yMat[h * width + w] = yuv[h * width + w];
			if (w % 2 == 0 && h % 2 == 0) {
				uMat[(h / 2) * width / 2 + (w / 2)] = yuv[total + (h / 2) * width / 2 + (w / 2)];
				vMat[(h / 2) * width / 2 + (w / 2)] = yuv[total * 5 / 4 + (h / 2) * width / 2 + (w / 2)];
			}
		}
	}
	/*
	* ---- * ---- *
	*	q1 |  g2  |
	* ---- * ---- *
	*   q3 |  q4  |
	* ---- * ---- *
	*/
	cv::Size half_size(height / 2, width / 2);
	cv::Mat _u1(height / 2, width / 2, CV_8UC1, uMat);
	cv::Mat _v1(height / 2, width / 2, CV_8UC1, vMat);

	cv::Size actual_size(width, height);
	cv::Mat _u(height, width, CV_8UC1);
	cv::Mat _v(height, width, CV_8UC1);
	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width / 2; x++) {
			int i_old = y * width / 2 + x;
			_u.at<uchar>(2 * y, 2 * x) = uMat[i_old];
			_u.at<uchar>(2 * y + 1, 2 * x) = uMat[i_old];
			_u.at<uchar>(2 * y, 2 * x + 1) = uMat[i_old];
			_u.at<uchar>(2 * y + 1, 2 * x + 1) = uMat[i_old];
			//1
			_v.at<uchar>(2 * y, 2 * x) = vMat[i_old];
			_v.at<uchar>(2 * y + 1, 2 * x) = vMat[i_old];
			_v.at<uchar>(2 * y, 2 * x + 1) = vMat[i_old];
			_v.at<uchar>(2 * y + 1, 2 * x + 1) = vMat[i_old];
		}
	}
	cv::Mat _y(height, width, CV_8UC1, yMat);
#ifndef use_cvtColor
	std::vector<cv::Mat> yuv_channels = { _y, _u, _v };
#else // !use_cvtColor
	cv::Mat u_resized, v_resized;
	cv::resize(_u, u_resized, actual_size, 0, 0, cv::INTER_LINEAR); //repeat u values 4 times
	cv::resize(_v, v_resized, actual_size, 0, 0, cv::INTER_LINEAR); //repeat v values 4 times
	std::vector<cv::Mat> yuv_channels = { _y, u_resized, v_resized };
#endif // use
	cv::Mat YUV;
	cv::merge(yuv_channels, YUV);
	cv::Mat bgr(YUV.size(), CV_8UC3);

#ifdef use_cvtColor
	cv::cvtColor(YUV, bgr, cv::COLOR_YUV2BGR);
#else
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			cv::Vec3b raw_yuv = YUV.at<cv::Vec3b>(y, x);
			auto Y = (int)raw_yuv.val[0];
			auto U = (int)raw_yuv.val[1];
			auto V = (int)raw_yuv.val[2];

			uvR = (V - 128) * 48318;
			uvB = (U - 128) * 61467;
			uvG = (U - 128) * (-5392) + (V - 128) * (-18722);

			Y = Y << 15;
			R = Y + uvR;
			r = CalcByte(R);
			G = Y + uvG;
			g = CalcByte(G);
			B = Y + uvB;
			b = CalcByte(B);
			bgr.at<cv::Vec3b>(y, x).val[0] = b;
			bgr.at<cv::Vec3b>(y, x).val[1] = g;
			bgr.at<cv::Vec3b>(y, x).val[2] = r;

		}
	}
#endif // use_cvtColor
	delete[]yMat;
	delete[]uMat;
	delete[]vMat;
	return bgr;
};

cv::Mat ImageConvertor::readImage(FrameOptions _fo) {
	int bufLen = _fo.getWidth() * _fo.getHeight() * 3 / 2;
	std::ifstream file_in(rootDir + _fo.getPath(), std::ios_base::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file_in), {});
	uchar* buf_src = new uchar[bufLen];
	std::copy(buffer.begin(), buffer.end(), buf_src);
	//convert YUV420 (NV21) -> Mat(opencv) 
	auto img = yuv_nv21_to_rgb_new(buf_src, _fo.getWidth(), _fo.getHeight());
	if (file_in.is_open())
		file_in.close();
	delete[]buf_src;
	return img;
};
