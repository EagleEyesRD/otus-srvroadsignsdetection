#include "General.h"
#include "AMQP/AMQPcpp.h"

namespace RMQ {
using MapSings = std::map<cv::Rect, typeSign>;
ManagerFrames frames;

int onCancel(AMQPMessage* message) {
     std::cout << "cancel tag=" << message->getDeliveryTag() << std::endl;
     return 0;
}

int onMessage(AMQPMessage* message) {
    uint32_t j = 0;
    char* data = message->getMessage(&j);
    if (data) {
       frames.setNewFrame(std::string(data));
       std::cout << data << std::endl;
    }
    std::cout << " tag=" << message->getDeliveryTag() << " content-type:" << message->getHeader("Content-type");
    std::cout << " encoding:" << message->getHeader("Content-encoding") << " mode=" << message->getHeader("Delivery-mode") << std::endl;
    try {
         AMQPQueue* q = message->getQueue();
         q->Cancel(message->getConsumerTag());
    } catch (std::exception ex) {
         std::cout << "error on close queue " + std::string(ex.what()) << std::endl;
    }
    return 0;
};

void ConsumerRMQ(std::unique_ptr<AMQP> &_amqp) { 
    try {
         AMQPQueue* qu2 = _amqp->createQueue("rsInput"); 
         qu2->Declare();
         qu2->setConsumerTag("q2");
         qu2->addEvent(AMQP_MESSAGE, onMessage);
         qu2->addEvent(AMQP_CANCEL, onCancel);
         qu2->Consume(AMQP_NOACK);
    } catch (AMQPException e) {
        std::cout << e.getMessage() << std::endl;
    }
}

void ProducerRMQ(std::unique_ptr<AMQP> &_amqp, FrameOptions &_fo, cv::Rect r, typeSign& s) {
    std::string ss = _fo.Serialize();
    std::cout << "rect: " << r << " raw_json:" << ss << " - type:" << s.gettypeId() << " - " << s.getName() << std::endl;

    try {
         AMQPExchange* ex = _amqp->createExchange("");
         AMQPQueue* qu2 = _amqp->createQueue("rsOutput");
         qu2->Declare();
         ex->setHeader("Delivery-mode", 2);
         ex->setHeader("Content-type", "text/text");
         ex->setHeader("Content-encoding", "UTF-8");
         ex->Publish(ss, "rsOutput");
    } catch (AMQPException e) {
         std::cout << e.getMessage() << std::endl;
    }
};

void SendPackets(std::unique_ptr<AMQP> &a, FrameOptions &_fo, std::vector<cv::Rect>& blobs, bool isRecog, MapSings signs) {
    int cnt = 0;
    if (isRecog && signs.size() > 0) {
       for (auto& sign : signs)
          ProducerRMQ(a, _fo, sign.first, sign.second);
    } else {
        for (auto& zone : blobs) {
           auto t = typeSign(0, "unknown");
           ProducerRMQ(a, _fo, zone, t);
        }
    }
};
}

class ProcessorFrame {
public:
ProcessorFrame(ScanVars sv, FrameOptions &_fo) {
    _IsShowWin = sv.getIsShowWin();
    _IsRecogSigns = sv.getIsRecogSings();
    detector = std::make_unique<SignDetector>();
    fo = _fo;
}

void Analyze(
    std::unique_ptr<AMQP>& a,
    int indexFrame,
    ManagerFrames& frame) {
    std::string jsonraw = frame.getRawFrame(indexFrame);
    if (jsonraw.length() > 0) {
        fo.DeserializeFromStr(jsonraw);
        auto img = ic.readImage(fo);
        auto zones = detector->Run(img, _IsShowWin, _IsRecogSigns, mp);
        RMQ::SendPackets(a, fo, zones, _IsRecogSigns, mp);
        frame.clearFrame(indexFrame);
        if (_IsShowWin) cv::waitKey(1);
    }
}

private:
ImageConvertor ic;
std::unique_ptr<SignDetector> detector;
RMQ::MapSings mp;
bool _IsShowWin;
bool _IsRecogSigns;
FrameOptions fo;
};
