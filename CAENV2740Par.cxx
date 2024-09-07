#include "CAENV2740Par.hxx"

#include <c4/std/string.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

CAENV2740Par::CAENV2740Par() : filename("") {}

CAENV2740Par::CAENV2740Par(const std::string& yamlFilePath) { loadConfigFile(yamlFilePath); }

void CAENV2740Par::loadConfigFile(const std::string& yamlFilePath) {
    try {
        std::ifstream file(yamlFilePath);
        std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        conf = ryml::parse_in_arena(ryml::to_csubstr(fileContent));
        conf.resolve();
        filename = yamlFilePath;
    } catch (const std::exception& e) {
        throw std::runtime_error("YAML 파일 로드 실패: " + std::string(e.what()));
    }
}

void CAENV2740Par::printYAMLNode(ryml::ConstNodeRef node, int indent) const { std::cout << node << std::endl; }

void CAENV2740Par::printConfig() const {
    std::cout << "YAML 설정:" << std::endl;
    printYAMLNode(conf.crootref(), 0);
}

std::string CAENV2740Par::getGlobalValue(const std::string& key) const {
    std::string value;
    if (conf["global"].has_child(c4::to_csubstr(key))) {
        conf["global"][c4::to_csubstr(key)] >> value;
        return value;
    }
    return "";
}

bool CAENV2740Par::getGlobalValueBool(const std::string& key) const {
    bool value;
    if (conf["global"].has_child(c4::to_csubstr(key))) {
        conf["global"][c4::to_csubstr(key)] >> value;
        return value;
    }
    return false;
}

uint32_t CAENV2740Par::getGlobalValueNumber(const std::string& key) const {
    uint32_t value;
    if (conf["global"].has_child(c4::to_csubstr(key))) {
        conf["global"][c4::to_csubstr(key)] >> value;
        return value;
    }
    return 0;
}

std::string CAENV2740Par::getChannelValue(const int ch, const std::string& key) const {
    std::string value;
    if (conf["channel"][ch].has_child(c4::to_csubstr(key))) {
        conf["channel"][ch][c4::to_csubstr(key)] >> value;
        return value;
    }
    return "";
}

bool CAENV2740Par::getChannelValueBool(const int ch, const std::string& key) const {
    bool value;
    if (conf["channel"][ch].has_child(c4::to_csubstr(key))) {
        conf["channel"][ch][c4::to_csubstr(key)] >> value;
        return value;
    }
    return false;
}

uint32_t CAENV2740Par::getChannelValueNumber(const int ch, const std::string& key) const {
    uint32_t value;
    if (conf["channel"][ch].has_child(c4::to_csubstr(key))) {
        conf["channel"][ch][c4::to_csubstr(key)] >> value;
        return value;
    }
    return 0;
}
