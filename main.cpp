#include "Utils.h"

void AnalyzeFrame(std::string &jsonraw, 
						AMQP &_amqp ,
					FrameOptions &fo, 
					ImageConvertor &ic, 
					std::unique_ptr<SignDetector> &detector, 
					bool &_IsShowWin, 
					bool &_IsRecogSigns, 
					MapSings &mp ) {
	if (jsonraw.length() > 0) {
		fo.DeserializeFromStr(jsonraw);
		auto img = ic.readImage(fo);
		auto zones = detector->Run(img, _IsShowWin, _IsRecogSigns, mp);
		SendPackets(_amqp, fo, zones, _IsRecogSigns, mp);
		if (_IsShowWin) cv::waitKey(1);
	}
}

int main(int argc, char **argv) {
	ScanVars inputVars;
	Parser(argc, argv, inputVars);
	std::string connection = getConnectionStr(inputVars);
	bool IsShowWin = std::get<5>(inputVars);
	bool IsRecogSings = std::get<6>(inputVars);
	AMQP amqp(connection);
	FrameOptions fo;
	ImageConvertor icvt;
	std::unique_ptr<SignDetector> detector(new SignDetector());
	MapSings mapSigns;
	while (true) {
		try {
			ConsumerRMQ(amqp);
			for (int i = 0; i < InfoFrames.size(); i++) {
				AnalyzeFrame(InfoFrames[i], amqp, fo, icvt, detector, IsShowWin, IsRecogSings, mapSigns);
				InfoFrames.erase(InfoFrames.begin() + i);
			}
			std::cout << "Wait new frames packet" << std::endl;
		} catch (std::exception ex) { std::cout << "error on main thread: " << std::string(ex.what()) << std::endl; }
	}
	
	return 0;
}