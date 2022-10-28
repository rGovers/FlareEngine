#include "Config.h"

#include <assert.h>
#include <string>
#include <tinyxml2.h>

Config::Config(const std::string_view& a_path)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(std::string(a_path).c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement* configEle = doc.FirstChildElement("Config");
        assert(configEle != nullptr);
    }
}