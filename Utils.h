#include "AMQP/AMQPcpp.h"
#include "SignDetector/ImageConverter.h"
#include "SignDetector/CommandLine.hpp"
#include "SignDetector/SignDetector.h"
#include <vector>
#include <memory>

int i = 0;
std::vector<std::string> InfoFrames;
using MapSings = std::map<cv::Rect, typeSign>;
int onCancel(AMQPMessage* message) {
	std::cout << "cancel tag=" << message->getDeliveryTag() << std::endl;
	return 0;
}

int  onMessage(AMQPMessage* message) {
	uint32_t j = 0;
	char* data = message->getMessage(&j);
	if (data) {
		InfoFrames.push_back(std::string(data));
		std::cout << data << std::endl;
	}

	i++;
	std::cout << "#" << i << " tag=" << message->getDeliveryTag() << " content-type:" << message->getHeader("Content-type");
	std::cout << " encoding:" << message->getHeader("Content-encoding") << " mode=" << message->getHeader("Delivery-mode") << std::endl;
	if (i > 10) {
		try {
			AMQPQueue* q = message->getQueue();
			q->Cancel(message->getConsumerTag());
		}
		catch (std::exception ex) {
			std::cout << "error on close queue " + std::string(ex.what()) << std::endl;
		}
	}
	return 0;
};

void ConsumerRMQ(AMQP& _amqp) {
	try {
		AMQPQueue* qu2 = _amqp.createQueue("rsInput");
		qu2->Declare();
		qu2->setConsumerTag("q2");
		qu2->addEvent(AMQP_MESSAGE, onMessage);
		qu2->addEvent(AMQP_CANCEL, onCancel);
		qu2->Consume(AMQP_NOACK);//
	}
	catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}
}

void ProducerRMQ(AMQP& _amqp, FrameOptions &_fo, cv::Rect r, typeSign &s) {
	std::string ss = _fo.Serialize();
	std::cout << "rect: " << r << " raw_json:" << ss << " - type:" << s.gettypeId() << " - " << s.getName() << std::endl;

	try {
		AMQPExchange* ex = _amqp.createExchange("");
		AMQPQueue* qu2 = _amqp.createQueue("rsOutput");
		qu2->Declare();
		ex->setHeader("Delivery-mode", 2);
		ex->setHeader("Content-type", "text/text");
		ex->setHeader("Content-encoding", "UTF-8");
		ex->Publish(ss, "rsOutput");
	}
	catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}
};

void SendPackets(AMQP& _amqp, FrameOptions& _fo, std::vector<cv::Rect>& blobs, bool isRecog, MapSings signs) {
	int cnt = 0;
	if (isRecog && signs.size() > 0) {
		for (auto& sign : signs)
			ProducerRMQ(_amqp, _fo, sign.first, sign.second);
	}
	else {
		for (auto& zone : blobs) {
			auto t = typeSign(0, "unknown");
			ProducerRMQ(_amqp, _fo, zone, t);
		}
	}
};

int Parser(int argc, char** argv, ScanVars& result) {
	std::string login = "";
	std::string password = "";
	std::string ip = "";
	int port = 0;
	std::string chat = "";
	bool IsShowWindows = false;
	bool IsRecognitionSigns = false;
	bool oPrintHelp = false;

	CommandLine args("");
	args.addArgument({ "-s", "--login" }, &login, "login to server RMQ");
	args.addArgument({ "-s", "--password" }, &password, "password");
	args.addArgument({ "-s", "--ip" }, &ip, "host's ip of server RMQ");
	args.addArgument({ "-i", "--port" }, &port, "port of server RMQ");
	args.addArgument({ "-s", "--chat" }, &chat, "Chat on server RMQ for exchange message");
	args.addArgument({ "-b", "--isshowwindows" }, &IsShowWindows, "It's posible to view process and artefactes");
	args.addArgument({ "-b", "--isrecognitionsigns" }, &IsRecognitionSigns, "It's posible to add step for recognition, because it depends on hardware");
	args.addArgument({ "-h", "--help" }, &oPrintHelp,
		"Print this help. This help message is actually so long "
		"that it requires a line break!");
	try {
		args.parse(argc, argv);
	}
	catch (std::runtime_error const& e) {
		std::cout << e.what() << std::endl;
		return -1;
	}

	if (oPrintHelp) {
		args.printHelp();
		return 0;
	}

	result = { login, password, ip, port, chat, IsShowWindows, IsRecognitionSigns };
	return true;
};

std::string getConnectionStr(ScanVars &s) {
	return std::get<0>(s) + ":" \
		+ std::get<1>(s) + "@" \
		+ std::get<2>(s) + ":" \
		+ std::to_string(std::get<3>(s)) \
		+ "/" + std::get<4>(s);
}
