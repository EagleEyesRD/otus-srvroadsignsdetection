#pragma once
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <fstream>
#include <sstream>

class JSONBase
{
public:
	bool DeserializeFromStr(const std::string& jsontxt);
	bool DeserializeFromFile(const std::string& filePath);
	
	virtual std::string Serialize() const;
	virtual bool Deserialize(const std::string& s);
	virtual bool Deserialize(const rapidjson::Document& obj) = 0;
	virtual bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const = 0;
protected:
	bool InitDocument(const std::string& s, rapidjson::Document& doc);
};



