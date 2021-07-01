#include "JSONBase.h"
bool JSONBase::DeserializeFromStr(const std::string& jsonstr)
{
    return Deserialize(jsonstr);
}

bool JSONBase::DeserializeFromFile(const std::string& filePath)
{
    std::ifstream f(filePath);
    std::stringstream buffer;
    buffer << f.rdbuf();
    f.close();

    return Deserialize(buffer.str());
}

std::string JSONBase::Serialize() const
{
    rapidjson::StringBuffer ss;
    rapidjson::Writer<rapidjson::StringBuffer> writer(ss);
    if (Serialize(&writer))
       return ss.GetString();
    return "";
}

bool JSONBase::Deserialize(const std::string& s)
{
    rapidjson::Document doc;
    if (!InitDocument(s, doc))
       return false;

    Deserialize(doc);

    return true;
}

bool JSONBase::InitDocument(const std::string& s, rapidjson::Document& doc)
{
    if (s.empty())
       return false;

    std::string validJson(s);

    return !doc.Parse(validJson.c_str()).HasParseError() ? true : false;
}
