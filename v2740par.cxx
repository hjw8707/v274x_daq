#include "v2740par.hxx"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <stdexcept>
#include <string>

CAENV2740Par::CAENV2740Par() : filename("") {}

CAENV2740Par::CAENV2740Par(const std::string& yamlFilePath) { loadConfigFile(yamlFilePath); }

void CAENV2740Par::loadConfigFile(const std::string& yamlFilePath) {
    try {
        config = YAML::LoadFile(yamlFilePath);
        filename = yamlFilePath;
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML 파일 로드 실패: " + std::string(e.what()));
    }
}

void CAENV2740Par::printYAMLNode(const YAML::Node& node, int indent) const {
    for (const auto& it : node) {
        std::string key = it.first.as<std::string>();
        for (int i = 0; i < indent; ++i) std::cout << "  ";
        std::cout << key << ": ";

        if (it.second.IsScalar()) {
            std::cout << it.second.as<std::string>() << std::endl;
        } else if (it.second.IsSequence()) {
            std::cout << "[";
            for (size_t i = 0; i < it.second.size(); ++i) {
                if (i > 0) std::cout << ", ";
                if (it.second[i].IsScalar()) {
                    std::cout << it.second[i].as<std::string>();
                } else if (it.second[i].IsMap()) {
                    std::cout << std::endl;
                    printYAMLNode(it.second[i], indent + 1);
                } else {
                    std::cout << "복잡한 타입";
                }
            }
            std::cout << "]" << std::endl;
        } else if (it.second.IsMap()) {
            std::cout << std::endl;
            printYAMLNode(it.second, indent + 1);
        }
    }
}

void CAENV2740Par::printConfig() const {
    std::cout << "YAML 설정:" << std::endl;
    printYAMLNode(config, 0);
}

template <>
std::string CAENV2740Par::getGlobalValue(const std::string& key) const {
    if (config["global"][key].IsDefined()) return config["global"][key].as<std::string>();
    for (const auto& it : config["global"])
        if (it.second.IsMap() && it.second[key].IsDefined()) return it.second[key].as<std::string>();
    return "";
}

template <>
uint32_t CAENV2740Par::getGlobalValue(const std::string& key) const {
    if (config["global"][key].IsDefined()) return config["global"][key].as<uint32_t>();
    for (const auto& it : config["global"])
        if (it.second.IsMap() && it.second[key].IsDefined()) return it.second[key].as<uint32_t>();
    return 0;
}

template <>
bool CAENV2740Par::getGlobalValue(const std::string& key) const {
    if (config["global"][key].IsDefined()) return config["global"][key].as<bool>();
    for (const auto& it : config["global"])
        if (it.second.IsMap() && it.second[key].IsDefined()) return it.second[key].as<bool>();
    return false;
}

template <>
std::string CAENV2740Par::getChannelValue(const int ch, const std::string& key) const {
    if (config["channel"][ch][key].IsDefined()) return config["channel"][ch][key].as<std::string>();
    for (const auto& it : config["channel"][ch]) {
        if (it.second.IsMap() && it.second[key].IsDefined()) return it.second[key].as<std::string>();
        if (it.second.IsMap())
            for (const auto& it2 : it.second)
                if (it2.second.IsMap() && it2.second[key].IsDefined()) return it2.second[key].as<std::string>();
    }
    return "";
}

template <>
uint32_t CAENV2740Par::getChannelValue(const int ch, const std::string& key) const {
    if (config["channel"][ch][key].IsDefined()) return config["channel"][ch][key].as<uint32_t>();
    for (const auto& it : config["channel"][ch]) {
        if (it.second.IsMap() && it.second[key].IsDefined()) return it.second[key].as<uint32_t>();
        if (it.second.IsMap())
            for (const auto& it2 : it.second)
                if (it2.second.IsMap() && it2.second[key].IsDefined()) return it2.second[key].as<uint32_t>();
    }
    return 0;
}

template <>
bool CAENV2740Par::getChannelValue(const int ch, const std::string& key) const {
    if (config["channel"][ch][key].IsDefined()) return config["channel"][ch][key].as<bool>();
    for (const auto& it : config["channel"][ch]) {
        if (it.second.IsMap() && it.second[key].IsDefined()) return it.second[key].as<bool>();
        if (it.second.IsMap())
            for (const auto& it2 : it.second)
                if (it2.second.IsMap() && it2.second[key].IsDefined()) return it2.second[key].as<bool>();
    }
    return false;
}

bool CAENV2740Par::hasGlobalValue(const std::string& key) const {
    if (!config["global"].IsDefined()) return false;

    if (config["global"][key].IsDefined()) return true;
    for (const auto& it : config["global"])
        if (it.second.IsMap() && it.second[key].IsDefined()) return true;
    return false;
}

bool CAENV2740Par::hasChannelValue(const int ch, const std::string& key) const {
    if (!config["channel"][ch].IsDefined()) return false;
    if (config["channel"][ch][key].IsDefined()) return true;
    for (const auto& it : config["channel"][ch]) {
        if (it.second.IsMap() && it.second[key].IsDefined()) return true;
        if (it.second.IsMap())
            for (const auto& it2 : it.second)
                if (it2.second.IsMap() && it2.second[key].IsDefined()) return true;
    }
    return false;
}