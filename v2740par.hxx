#ifndef V2740_PARAMETER_MANAGER_H
#define V2740_PARAMETER_MANAGER_H

#include <yaml-cpp/yaml.h>

#include <string>

class CAENV2740Par {
   private:
    YAML::Node config;
    std::string filename;

   public:
    CAENV2740Par();
    CAENV2740Par(const std::string& yamlFilePath);
    void loadConfigFile(const std::string& yamlFilePath);

    inline const std::string& getFilename() const { return filename; }

    void printYAMLNode(const YAML::Node& node, int indent = 0) const;
    void printConfig() const;

    inline const YAML::Node& getConfig() const { return config; };

    template <typename T>
    T getGlobalValue(const std::string& key) const;
    template <>
    std::string getGlobalValue(const std::string& key) const;
    template <>
    uint32_t getGlobalValue(const std::string& key) const;
    template <>
    bool getGlobalValue(const std::string& key) const;

    template <typename T>
    T getChannelValue(const int ch, const std::string& key) const;
    template <>
    std::string getChannelValue(const int ch, const std::string& key) const;
    template <>
    uint32_t getChannelValue(const int ch, const std::string& key) const;
    template <>
    bool getChannelValue(const int ch, const std::string& key) const;

    bool hasGlobalValue(const std::string& key) const;
    bool hasChannelValue(const int ch, const std::string& key) const;
};

#endif  // V2740_PARAMETER_MANAGER_H