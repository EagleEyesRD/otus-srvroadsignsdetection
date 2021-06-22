#include "FrameOptions.h"

FrameOptions::FrameOptions()
{
    ShutName = "";
    FileName = "";
    epoch = 1;
    scale = 1;
    path = "";
    frameNum = 1;
    width = 1;
    height = 1;
    frameDateTime = "";
    H = 0.0;
    Gx = 0.0;
    Gy = 0.0;
    Gz = 0.0;
    pixelSizeByF = 1.0;
    yaw = 1.0;
    MarkUpShape = "";
    typeId = 0;
    type = "unknown";
    ownBearing = 0;
    predictedShiftX = 1;
    predictedShiftY = 1;
    predictedU = 1;
}

FrameOptions::~FrameOptions()
{
}

std::string FrameOptions::getPath() const { return path; }
int FrameOptions::getWidth() const { return width; }
int FrameOptions::getHeight() const { return height; }

bool FrameOptions::Deserialize(const rapidjson::Document& d)
{
    ShutName = d["ShutName"].GetString();
    FileName = d["FileName"].GetString();
    epoch = d["epoch"].GetInt64();
    scale = d["scale"].GetInt();
    path = d["path"].GetString();
    frameNum = d["frameNum"].GetInt();
    width = d["width"].GetInt();
    height = d["height"].GetInt();
    frameDateTime = d["frameDateTime"].GetString();
    H = d["H"].GetDouble();
    Gx = d["Gx"].GetDouble();
    Gy = d["Gy"].GetDouble();
    Gz = d["Gz"].GetDouble();
    pixelSizeByF = d["pixelSizeByF"].GetDouble();
    yaw = d["yaw"].GetDouble();
    return true;
}

bool FrameOptions::Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const
{
    writer->StartObject();

    writer->String("ShutName");
    writer->String(ShutName.c_str());
    writer->String("FileName");
    writer->String(FileName.c_str());
    writer->String("epoch");
    writer->Int64(epoch);
    writer->String("frameNum");
    writer->Int(frameNum);
    writer->String("MarkUpShape");
    writer->String(MarkUpShape.c_str());
    writer->String("typeId");
    writer->Int(typeId);
    writer->String("type");
    writer->String(type.c_str());
    writer->String("ownBearing");
    writer->Double(ownBearing);
    writer->String("predictedShiftX");
    writer->Double(predictedShiftX);
    writer->String("predictedShiftY");
    writer->Double(predictedShiftY);
    writer->String("predictedU");
    writer->Double(predictedU);

    writer->EndObject();
    return true;
}

std::string FrameOptions::Serialize() const
{
    rapidjson::StringBuffer ss;
    rapidjson::Writer<rapidjson::StringBuffer> writer(ss);
    if (Serialize(&writer))
        return ss.GetString();

    return "";
}
std::string FrameOptions::getMarkUpShape() const { return MarkUpShape; }
void FrameOptions::setMarkUpShape(std::string m) { MarkUpShape = m; }
int FrameOptions::gettypeId() const { return typeId; }
void FrameOptions::settypeId(int id) { typeId = id; }
std::string FrameOptions::getType() const { return type; }
void FrameOptions::setType(std::string t) { type = t; }
float FrameOptions::getownBearing() const { return ownBearing; }
void FrameOptions::setownBearing(float bearing) { ownBearing = bearing; }
float FrameOptions::getpredictedShiftX() const { return predictedShiftX; }
void FrameOptions::setpredictedShiftX(float x) { predictedShiftX = x; }
float FrameOptions::getpredictedShiftY() const { return predictedShiftY; }
void FrameOptions::setpredictedShiftY(float y) { predictedShiftY = y; }
float FrameOptions::getpredictedU() const { return predictedU; }
void FrameOptions::setpredictedU(float u) { predictedU = u; }