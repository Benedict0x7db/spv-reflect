#ifndef __GLOBALFUN_H__
#define __GLOBALFUN_H__

#include "shaderc/shaderc.hpp"
#include <string>
#include "stdio.h"
#include "VkType.h"
using namespace std;



enum DataFlow{
    DataFlow_in = 1,
    DataFlow_uniform = 2,
    DataFlow_out = 3
};


#define GL_FLOAT                          0x1406
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_CUBE                   0x8B60

struct ShaderInfo{
    DataFlow data_flow;
    uint32_t location;
    uint32_t length;
    VkFormat format;
    uint32_t varType;
    VkDescriptorType descriptorType;
    VkShaderStageFlags stageFlags;
    char loc_name[32];
    char shaderName[32];
};

#ifdef __cplusplus
extern "C" {
#endif

std::vector<char> readShaderFile(std::string fileName);
void getShaderInfo(std::vector<ShaderInfo> &shaderInfo, std::vector<char> &code);
void writeShaderFile(std::string fileName, const std::vector<uint32_t> &code, std::vector<ShaderInfo> &shaderInfo);
void readShaderSpvFile(std::string fileName);

shaderc_shader_kind MapShadercType(VkShaderStageFlagBits vkShader);

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, int length,
               std::vector<unsigned int> &spirv);

void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen);
void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen);

void findShaderInfo(std::vector<char> &shaderData, std::vector<ShaderInfo> &shaderInfo,
                    std::string line, ifstream &fin, VkShaderStageFlagBits flagBits);
void obscureShader(std::string shadeFile, std::string obsShaderFile);

void shaderInfoToStr(std::string &strObfus, std::vector<ShaderInfo> &shaderInfo);
void spvToStr(std::string &strObfus, const std::vector<uint32_t> &code);

#ifdef __cplusplus
}
#endif


#endif //__GLOBALFUN_H__
