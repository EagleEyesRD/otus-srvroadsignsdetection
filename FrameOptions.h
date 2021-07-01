#ifndef FRAME_OPTIONS_HPP
#define FRAME_OPTIONS_HPP

#include "JSONBase.h"
#include <string>
#include <memory>
#include <tuple>
class FrameOptions : public JSONBase
{
public:
    FrameOptions();
    virtual ~FrameOptions();
    //bool DeserializeFromStr(const std::string& jsontxt);
    virtual std::string Serialize() const;
    //virtual bool Deserialize(const std::string& s);
    virtual bool Deserialize(const rapidjson::Document& obj);// { return false; };
    virtual bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const;


public:
    std::string getPath() const;
    int getWidth() const;
    int getHeight() const;
    //
    std::string getMarkUpShape() const; 
    void setMarkUpShape(std::string);
    int gettypeId() const; 
    void settypeId(int);
    std::string getType() const;
    void setType(std::string);
    float getownBearing() const; 
    void setownBearing(float);
    float getpredictedShiftX() const;
    void setpredictedShiftX(float);
    float getpredictedShiftY() const; 
    void setpredictedShiftY(float);
    float getpredictedU() const; 
    void setpredictedU(float);
private:
    //input fields
    std::string ShutName;
    std::string FileName;
    int64_t epoch;
    int scale;
    std::string path;
    int frameNum;
    int width;
    int height;
    std::string frameDateTime;
    double H;
    double Gx;
    double Gy;
    double Gz;
    double pixelSizeByF;
    double yaw;
    //output fields
    std::string MarkUpShape;  
    int typeId; 
    std::string type; 
    float ownBearing; 
    float predictedShiftX; 
    float predictedShiftY; 
    float predictedU;  
};

#endif // FRAME_OPTIONS_HPP