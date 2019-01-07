#include <fstream>
#include <iostream>
#include <regex>
#include "glm/glm.hpp"
#include "global_fun.h"

string gShaderName;
int gMaxUniformVS = 0;
int gMaxUniformFS = 0;
int gMaxVS = 0;
int gMaxFS = 0;

struct shader_type_mapping {
    VkShaderStageFlagBits vkshader_type;
    shaderc_shader_kind   shaderc_type;
};

static const shader_type_mapping shader_map_table[] =
        {
        {
                VK_SHADER_STAGE_VERTEX_BIT,
                shaderc_glsl_vertex_shader
        },
        {
                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                shaderc_glsl_tess_control_shader
        },
        {
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                shaderc_glsl_tess_evaluation_shader
        },
        {
                VK_SHADER_STAGE_GEOMETRY_BIT,
                shaderc_glsl_geometry_shader},
        {
                VK_SHADER_STAGE_FRAGMENT_BIT,
                shaderc_glsl_fragment_shader
        },
        {
                VK_SHADER_STAGE_COMPUTE_BIT,
                shaderc_glsl_compute_shader
        },
};

shaderc_shader_kind MapShadercType(VkShaderStageFlagBits vkShader)
{
    for (auto shader : shader_map_table) {
        if (shader.vkshader_type == vkShader) {
            return shader.shaderc_type;
        }
    }
    assert(false);
    return shaderc_glsl_infer_from_source;
}

std::vector<char> readShaderFile(std::string fileName)
{
    FILE *pFile = fopen(fileName.c_str(), "rb");
    fseek(pFile, 0, SEEK_END);
    int nFileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET );
    std::vector<char> buffer(nFileSize);
    fread(buffer.data(), nFileSize, 1, pFile );
    fclose(pFile);

    return buffer;
}

void getShaderInfo(std::vector<ShaderInfo> &shaderInfo, std::vector<char> &code)
{
    char *pdata = code.data();
    int datalen = code.size();
    char *pTemp = pdata;
    int linebegin = 0;
    int lineend = 0;
    while( lineend < datalen ) {
        while (pTemp[lineend] != '\n' && lineend < datalen) {
            ++lineend;
        }

        std::string line = std::string(pdata + linebegin, lineend - linebegin );
        if( line.find("layout") != string::npos){
            if( line.find("location") != string::npos && line.find(" in ") != string::npos ){
                ShaderInfo shader_info;
                memset(&shader_info, 0, sizeof(ShaderInfo) );
                shader_info.data_flow = DataFlow_in;
                int posb = line.rfind(" ") + 1;
                int pose = line.rfind(";");
                string name = line.substr(posb, pose - posb );
                memcpy( shader_info.loc_name, name.data(), sizeof(shader_info.loc_name) - 1);
                memcpy( shader_info.shaderName, gShaderName.c_str(), gShaderName.size());
                posb = line.find("=") + 1;
                pose = line.find(")");
                string strpos = line.substr(posb, pose - posb );
                shader_info.location = atoi(strpos.data());
                shaderInfo.push_back(shader_info);
            } else if( line.find("location") != string::npos && line.find(" out ") != string::npos ){
                ShaderInfo shader_info;
                memset(&shader_info, 0, sizeof(ShaderInfo) );
                shader_info.data_flow = DataFlow_out;
                int posb = line.rfind(" ") + 1;
                int pose = line.rfind(";");
                string name = line.substr(posb, pose - posb );
                memcpy( shader_info.loc_name, name.data(), sizeof(shader_info.loc_name) - 1);
                memcpy( shader_info.shaderName, gShaderName.c_str(), gShaderName.size());
                posb = line.find("=") + 1;
                pose = line.find(")");
                string strpos = line.substr(posb, pose - posb );
                shader_info.location = atoi(strpos.data());
                shaderInfo.push_back(shader_info);
            }else if( line.find("binding") != string::npos && line.find(" uniform ") != string::npos ){
                ShaderInfo shader_info;
                memset(&shader_info, 0, sizeof(ShaderInfo) );
                shader_info.data_flow = DataFlow_uniform;
                int posb = line.rfind(" ") + 1;
                int pose = line.rfind(";");
                if( pose == string::npos ){
                    pose = line.size() -1;
                }
                string name = line.substr(posb, pose - posb );
                memcpy( shader_info.loc_name, name.data(), sizeof(shader_info.loc_name) - 1);
                memcpy( shader_info.shaderName, gShaderName.c_str(), gShaderName.size());
                posb = line.find("=") + 1;
                pose = line.find(")");
                string strpos = line.substr(posb, pose - posb );
                shader_info.location = atoi(strpos.data());
                shaderInfo.push_back(shader_info);
            }


        }
        linebegin = lineend + 1;
        ++lineend;

    }
}

void writeShaderFile(std::string fileName, const std::vector<uint32_t> &code, std::vector<ShaderInfo> &shaderInfo)
{

    int tollen = 2 * sizeof(uint32_t) + shaderInfo.size() * sizeof(shaderInfo) + code.size() * sizeof(uint32_t) + 1;
    char *pdata = new char[tollen];
    memset( pdata, 0, tollen );
    char *pTemp = pdata;

    // Default field 0xD86F06DD
    FILE *pFile = fopen(fileName.c_str(), "wb");
    uint32_t flag = 0xD86F06DD;
    uint32_t len = shaderInfo.size() * sizeof(shaderInfo);

//    fwrite(&flag, sizeof(uint32_t), 1, pFile);
//    fwrite(&len, sizeof(uint32_t), 1, pFile);
//    fwrite(shaderInfo.data(), len, 1, pFile);
//    fwrite(code.data(), code.size() * sizeof(uint32_t), 1, pFile);
//    fclose(pFile);

    memcpy(pTemp, &flag, sizeof(uint32_t));
    pTemp += sizeof(uint32_t);
    memcpy(pTemp, &len, sizeof(uint32_t));
    pTemp += sizeof(uint32_t);
    memcpy(pTemp, shaderInfo.data(), len);
    pTemp += len;
    memcpy(pTemp, code.data(), code.size() * sizeof(uint32_t));

    pTemp = pdata;
    int i = 0;
    for( i = 0; i < tollen; ++i )
    {
        char tmp[8] = {0};
        unsigned char uch = pTemp[i];
        sprintf(tmp, "%02x", uch);
        fwrite(tmp, 2, 1, pFile);
    }

//    fwrite(pdata, tollen, 1, pFile);
    fclose(pFile);
    delete []pdata;

    return;
}

void readShaderSpvFile(std::string fileName)
{
    FILE *pFile = fopen(fileName.c_str(), "rb");
    fseek(pFile, 0, SEEK_END);
    int ilen = ftell(pFile);
    fseek(pFile, 0, SEEK_SET );
    char *pch = new char[ilen + 1];
    char *pbyte = new char[ilen + 1];
    memset(pch, 0, ilen + 1);
    memset(pbyte, 0, ilen + 1 );
    fread(pch, ilen, 1, pFile );
    fclose(pFile);

    StrToHex((unsigned char *)pbyte, (unsigned char *)pch, ilen/2);

    string outFileName = fileName + ".out";
    FILE *pOut = fopen(outFileName.c_str(), "wb");
    fwrite(pbyte, ilen/2, 1, pOut);
    fclose(pOut);

    delete []pbyte;
    delete []pch;
}

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader,
               int length, std::vector<unsigned int> &spirv)
{
    // On Android, use shaderc instead.
    shaderc::Compiler compiler;
    shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(pshader, length,
                                      MapShadercType(shader_type),
                                      "shader");
    if (module.GetCompilationStatus() !=
        shaderc_compilation_status_success) {
        printf("Error: Id=%d, Msg=%s",
             module.GetCompilationStatus(),
             module.GetErrorMessage().c_str());
        return false;
    }
    spirv.assign(module.cbegin(), module.cend());

    return true;
}

/*
// C prototype : void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
// [IN] pbSrc - 字符串
// [IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value:
// remarks : 将字符串转化为16进制数
*/
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char h1,h2;
    unsigned char s1,s2;
    int i;

    for (i=0; i<nLen; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i+1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1*16 + s2;
    }
}

/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
// [IN] pbSrc - 输入16进制数的起始地址
// [IN] nLen - 16进制数的字节数
// return value:
// remarks : 将16进制数转化为字符串
*/
void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char ddl,ddh;
    int i;

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}

void setShaderType(string &strType, ShaderInfo &shader_info)
{
    if( shader_info.data_flow == DataFlow_uniform ){
        shader_info.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
    if( strType == "vec4")
    {
        shader_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        shader_info.length = sizeof(float) * 4;
        shader_info.varType = GL_FLOAT_VEC4;
    }else if( strType == "vec2"){
        shader_info.format = VK_FORMAT_R32G32_SFLOAT;
        shader_info.length = sizeof(float) * 2;
        shader_info.varType = GL_FLOAT_VEC2;
    }else if( strType == "vec3"){
        shader_info.format = VK_FORMAT_R32G32B32_SFLOAT;
        shader_info.length = sizeof(float) * 3;
        shader_info.varType = GL_FLOAT_VEC3;
    }else if( strType == "float"){
        shader_info.format = VK_FORMAT_R32_SFLOAT;
        shader_info.length = sizeof(float);
        shader_info.varType = GL_FLOAT;
    }else if( strType =="mat4"){
        shader_info.format = VK_FORMAT_UNDEFINED;
        shader_info.length = sizeof(glm::mat4); //4 * 4 * sizeof(float);
        shader_info.varType = GL_FLOAT_MAT4;
    }else if( strType =="mat3"){
        shader_info.format = VK_FORMAT_UNDEFINED;
        shader_info.length = sizeof(glm::mat3); //3 * 3 * sizeof(float);
        shader_info.varType = GL_FLOAT_MAT3;
    }else if( strType == "sampler2D"){
        if( shader_info.data_flow == DataFlow_uniform ) {
            shader_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            shader_info.varType = GL_SAMPLER_2D;
        }
    }
    if( shader_info.length == 0 )
    {
        shader_info.length = 0;
    }
}

std::string trim(string& str)
{
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) str.erase(0, pos);
    }
    else {
        str.erase(str.begin(), str.end());
    }
    return str;
}

std::string trimMark(string& str)
{
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) str.erase(0, pos);
    }
    else {
        str.erase(str.begin(), str.end());
    }
    ///
    pos = str.find_last_not_of(',');
    if(pos != string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(',');
        if(pos != string::npos) str.erase(0, pos);
    }
    else {
        str.erase(str.begin(), str.end());
    }
    //
    pos = str.find_last_not_of('"');
    if(pos != string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of('"');
        if(pos != string::npos) str.erase(0, pos);
    }
    else {
        str.erase(str.begin(), str.end());
    }
    return str;
}

void findShaderInfo(std::vector<char> &shaderData, std::vector<ShaderInfo> &shaderInfos,
                    std::string strline, ifstream &fin, VkShaderStageFlagBits flagBits)
{
    if( strline.find("layout") != string::npos){
        if( strline.find("location") != string::npos && strline.find(" in ") != string::npos
                && flagBits == VK_SHADER_STAGE_VERTEX_BIT )
        {
            ShaderInfo shader_info;
            memset(&shader_info, 0, sizeof(ShaderInfo) );
            shader_info.data_flow = DataFlow_in;
            // name
            int posb = strline.rfind(" ") + 1;
            int pose = strline.rfind(";");
            string name = strline.substr(posb, pose - posb );
            memcpy( shader_info.loc_name, name.data(), name.size());
            memcpy( shader_info.shaderName, gShaderName.c_str(), gShaderName.size());

            // type
            pose = posb - 1;
            posb = strline.rfind(" ", pose - 1) + 1;
            string strtype = strline.substr(posb, pose - posb);
            setShaderType(strtype, shader_info);
            //location
            posb = strline.find("=") + 1;
            pose = strline.find(")");
            string strpos = strline.substr(posb, pose - posb );
            shader_info.location = atoi(strpos.data());
            shaderInfos.push_back(shader_info);
        }
//        else if( strline.find("location") != string::npos && strline.find(" out ") != string::npos )
//        {
//            ShaderInfo shader_info;
//            memset(&shader_info, 0, sizeof(ShaderInfo) );
//            shader_info.data_flow = DataFlow_out;
//            int posb = strline.rfind(" ") + 1;
//            int pose = strline.rfind(";");
//            string name = strline.substr(posb, pose - posb );
//            memcpy( shader_info.loc_name, name.data(), name.size());
//            posb = strline.find("=") + 1;
//            pose = strline.find(")");
//            string strpos = strline.substr(posb, pose - posb );
//            shader_info.location = atoi(strpos.data());
//            shaderInfos.push_back(shader_info);
//        }
        else if( strline.find("binding") != string::npos && strline.find(" uniform ") != string::npos ){
            if(flagBits == VK_SHADER_STAGE_VERTEX_BIT){
                ++gMaxUniformVS;
                cout<<strline << ", MaxUniformVS:" <<gMaxUniformVS <<endl;
            }else{
                ++gMaxUniformFS;
                cout<<strline << ", MaxUniformFS:" <<gMaxUniformFS <<endl;
            }
            ShaderInfo shader_info;
            memset(&shader_info, 0, sizeof(ShaderInfo));
            shader_info.data_flow = DataFlow_uniform;
            shader_info.stageFlags = flagBits;
            string strpos;
            if( strline.find("{") != string::npos) {
                int posb = strline.rfind(" ") + 1;
                int pose = strline.rfind(";");
                if (pose == string::npos) {
                    pose = strline.size();
                }
                string name = strline.substr(posb, pose - posb -1);
                memcpy(shader_info.loc_name, name.data(), name.size());
                memcpy( shader_info.shaderName, gShaderName.c_str(), gShaderName.size());
                posb = strline.find("=") + 1;
                pose = strline.find(")");
                strpos  = strline.substr(posb, pose - posb);

                string line;
//                getline(fin, line);
//                shaderData.insert(shaderData.end(), line.begin(), line.end());
//                shaderData.push_back('\r');
                int length = 0;
                while (getline(fin, line)) {
                    shaderData.insert(shaderData.end(), line.begin(), line.end());
                    shaderData.push_back('\r');
                    if (line.find("}") != string::npos) {
                        break;
                    }
                    trim(line);
                    pose = line.find(" ");
                    string strtype = line.substr(0, pose);
                    setShaderType(strtype, shader_info);
                    length += shader_info.length;
                }
                shader_info.length = length;

            }else{
                // name
                int posb = strline.rfind(" ") + 1;
                int pose = strline.rfind(";");
                string name = strline.substr(posb, pose - posb );
                memcpy( shader_info.loc_name, name.data(), name.size());
                memcpy( shader_info.shaderName, gShaderName.c_str(), gShaderName.size());

                // type
                pose = posb - 1;
                posb = strline.rfind(" ", pose - 1) + 1;
                string strtype = strline.substr(posb, pose - posb);
                setShaderType(strtype, shader_info);
                //location
                posb = strline.find("=") + 1;
                pose = strline.find(")");
                strpos = strline.substr(posb, pose - posb );
            }


            shader_info.location = atoi(strpos.data());

            shaderInfos.push_back(shader_info);
        }
    }
    return;
}

void shaderInfoToStr(std::string &strObfus, std::vector<ShaderInfo> &shaderInfos)
{
    //file struct
    //shaderInfo length(4) + shaderInfoData(shaderInfo.size() * sizeof(shaderInfo))

    uint32_t len = shaderInfos.size() * sizeof(ShaderInfo);
    int tollen = sizeof(uint32_t) + len;
    char *pdata = new char[tollen + 1];
    memset( pdata, 0, tollen + 1);
    char *pTemp = pdata;

    memcpy(pTemp, &len, sizeof(uint32_t));
    pTemp += sizeof(uint32_t);
    memcpy(pTemp, shaderInfos.data(), len);

    pTemp = pdata;
    int i = 0;
    for( i = 0; i < tollen; ++i )
    {
        char tmp[3] = {0};
        unsigned char uch = pTemp[i];
        sprintf(tmp, "%02x", uch);
        strObfus += tmp;
    }
    delete []pdata;
    return;
}

void spvToStr(std::string &strObfus, const std::vector<uint32_t> &code)
{
    //file struct
    //spvdata(sizeof(shaderInfo) + code.size() * sizeof(uint32_t))
    int tollen = code.size() * sizeof(uint32_t);
    char *pdata = new char[tollen];
    memset( pdata, 0, tollen );
    char *pTemp = pdata;

    memcpy(pTemp, code.data(), code.size() * sizeof(uint32_t));

    pTemp = pdata;
    int i = 0;
    for( i = 0; i < tollen; ++i )
    {
        char tmp[3] = {0};
        unsigned char uch = pTemp[i];
        sprintf(tmp, "%02x", uch);
        strObfus += tmp;
    }

    delete []pdata;
    return;
}

void obscureShader(std::string shadeFile, std::string obsShaderFile)
{

    ifstream fin( shadeFile.c_str() );
    ofstream fout( obsShaderFile.c_str());
    regex regBegin(".*R\\\"\\w{4}\\(");
    regex regEnd(".*\\)\\w{4}\\\"");
    string strline;
    bool bBegin = false;
    bool bEnd =false;
    std::vector<ShaderInfo> shaderInfos;
    std::vector<char> shaderData;
    std::string strObfus;
    VkShaderStageFlagBits flagBits = VK_SHADER_STAGE_VERTEX_BIT;

    string shaderName;

    while ( getline(fin,strline) )
    {
        cout << strline << endl;
        if( strline.find(".vs") != string::npos)
        {
            shaderName = strline;
            trimMark(shaderName);
            gShaderName = shaderName;
            flagBits = VK_SHADER_STAGE_VERTEX_BIT;
        }else if( strline.find(".fs") != string::npos) {
            shaderName = strline;
            trimMark(shaderName);
            gShaderName = shaderName;
            flagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if( shaderName.find("colorOP.fs") != string::npos )
        {
            printf("test\n");
        }

        smatch matchBegin;
        smatch matchEnd;
        bBegin = regex_search(strline, matchBegin, regBegin );
        if( bBegin )
        {
//            fout << strline << endl;
            while( getline(fin, strline))
            {
                bEnd = regex_search(strline, matchEnd, regEnd);
                if (bEnd) {

                    std::vector<unsigned int> vtx_spv;
                    GLSLtoSPV(flagBits, shaderData.data(), shaderData.size(), vtx_spv);
                    FILE *pFile = fopen(shaderName.c_str(), "wb");
                    fwrite(vtx_spv.data(), vtx_spv.size() * 4, 1, pFile);
                    fclose(pFile);

                    if( gMaxUniformFS > gMaxFS ){
                        gMaxFS = gMaxUniformFS;
                    }
                    if(gMaxUniformVS > gMaxVS){
                        gMaxVS = gMaxUniformVS;
                    }

                    gMaxUniformVS = 0;
                    gMaxUniformFS = 0;

                    shaderInfoToStr(strObfus, shaderInfos);
                    fout << "\"" << strObfus << "\"" << "," << endl;
                    strObfus.clear();

                    spvToStr(strObfus, vtx_spv);
                    fout << "\"" << strObfus << "\"" << "," << endl;

                    cout << matchEnd.str() << endl;

                    shaderInfos.clear();
                    shaderData.clear();
                    strObfus.clear();

                    bBegin = false;
                    bEnd = false;
                    break;
                } else {
                    shaderData.insert(shaderData.end(), strline.begin(), strline.end());
                    shaderData.push_back('\r');
                    findShaderInfo(shaderData, shaderInfos, strline, fin, flagBits);
//                    strline.push_back('\r');

                }// end of else
            }// end of while
        }
        else
        {
            fout << strline << endl;
        }

    } // end of while

    cout << "MaxVS:" <<gMaxVS << ", MaxFS:" << gMaxFS << endl;

    fin.close();
    fout.close();


}
