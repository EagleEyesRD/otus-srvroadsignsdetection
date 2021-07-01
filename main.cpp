#include "Utils.h"

int main(int argc, char **argv) {
    ScanVars inputVars;
    Parser(argc, argv, inputVars);
    std::unique_ptr<AMQP> amqp(new AMQP(getConnectionStr(inputVars)));
    FrameOptions fo;
    ProcessorFrame pf(inputVars, fo);
    while (true) {
       try {
             RMQ::ConsumerRMQ(amqp);
             for (size_t i = 0; i < RMQ::frames.getSize(); i++)
               pf.Analyze(amqp, i, RMQ::frames);
               std::cout << "Wait new frames packet" << std::endl;
       } catch (std::exception ex) { std::cout << "error on main thread: " << std::string(ex.what()) << std::endl; }
    }
    return 0;
}