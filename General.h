#include "CommandLine.hpp"
#include "SignDetector/ImageConverter.h"
#include "SignDetector/SignDetector.h"
#include <memory>
#include <string>
#include <vector>

struct ScanVars {
public:
    ScanVars() {}
    ScanVars( std::string _login, std::string _password, std::string _ip, int _port, std::string _chat, bool _IsShowWindows, bool _IsRecognitionSigns) :
       login(_login), password(_password), ip(_ip), port(_port), chat(_chat), IsShowWin(_IsShowWindows), IsRecogSings(_IsRecognitionSigns) {}
    std::string getLogin() const { return login; }
    std::string setPassword() const { return password; }
    std::string getIp() const { return ip; }
    int getPort() const { return port; }
    std::string getChat() const { return chat; }
    bool getIsShowWin() const { return IsShowWin; }
    bool getIsRecogSings() const { return IsRecogSings; }
private:
    std::string login;
    std::string password;
    std::string ip;
    int port;
    std::string chat;
    bool IsShowWin;
    bool IsRecogSings;
};

class ManagerFrames {
public:
    ManagerFrames() {}
    void setNewFrame( std::string raw) { Frames.push_back(raw); }
    std::string getRawFrame( int index) const { return Frames[index]; }
    void clearFrame( int indexFrame) { Frames.erase(Frames.begin() + indexFrame); }
    size_t getSize() const { return Frames.size(); }
private:
    std::vector<std::string> Frames;
};

int Parser( int argc, char** argv, ScanVars& result) {
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
    } catch (std::runtime_error const& e) {
         std::cout << e.what() << std::endl;
         return -1;
    }

    if (oPrintHelp) {
         args.printHelp();
         return 0;
    }

    result = ScanVars (login, password, ip, port, chat, IsShowWindows, IsRecognitionSigns);
    return true;
};

std::string getConnectionStr( ScanVars& s) {
     return s.getLogin() + ":" \
        + s.setPassword() + "@" \
        + s.getIp() + ":" \
        + std::to_string(s.getPort()) \
        + "/" + s.getChat();
}

