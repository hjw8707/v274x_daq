#ifndef V2740_PARAMETER_MANAGER_H
#define V2740_PARAMETER_MANAGER_H

#include <yaml-cpp/yaml.h>

#include <ryml.hpp>
#include <string>

class CAENV2740Par {
   private:
    // YAML::Node config;
    ryml::Tree conf;
    std::string filename;

   public:
    CAENV2740Par();
    CAENV2740Par(const std::string& yamlFilePath);
    void loadConfigFile(const std::string& yamlFilePath);

    inline const std::string& getFilename() const { return filename; }

    void printYAMLNode(ryml::ConstNodeRef node, int indent = 0) const;
    void printConfig() const;

    inline const ryml::ConstNodeRef getConfig() const { return conf.crootref(); };
    // inline const YAML::Node& getConfig() const { return config; };

    std::string getGlobalValue(const std::string& key) const;
    bool getGlobalValueBool(const std::string& key) const;
    uint32_t getGlobalValueNumber(const std::string& key) const;

    std::string getChannelValue(const int ch, const std::string& key) const;
    bool getChannelValueBool(const int ch, const std::string& key) const;
    uint32_t getChannelValueNumber(const int ch, const std::string& key) const;
    // bool hasGlobalValue(const std::string& key) const;
    // bool hasChannelValue(const int ch, const std::string& key) const;
};

#endif  // V2740_PARAMETER_MANAGER_H