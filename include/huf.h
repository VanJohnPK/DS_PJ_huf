#ifndef _huf_h_
#define _huf_h_

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

using namespace std;

#define N 256
#define MAX 2 * N - 1
#define INF 999999999 //+infinity
#define PMOD 384      // parent modifier`

class HufTool
{
private:
    struct HTNode // 哈夫曼树节点
    {
        int weight;
        int parent, lchild, rchild;
        int length;   // 编码长度
        int *hufCode; // 指向存放了长度为length的编码的区域
    };
    struct DirNode // 目录节点
    {
        string pathStr;
        unsigned char len; // 名字所占字节数
        unsigned char dep; // 深度
        bool isFile = false;
    };

    int nodeCnt[N];
    HTNode HT[MAX];         // HT是huffmanTree的缩写
    int root = N;           // 用于记录根节点
    char cover;             // 补位数
    vector<DirNode> dirVec; // 目录容器

    void find2Min(int num, int &m1, int &m2); // 找到权重最小的两个根节点

    void makeHT();                             // 初始化以及造树
    void encode();                             // 编码
    void nodeCount(string input);              // 统计出现次数
    void writeHT(string input, ofstream &ofs); // 写入文件
    long long getLength(ifstream &ifs);        // 读取并计算返回文件长度
    void fileHuf(string input, ofstream &ofs); // 压缩文件

    void remakeHT(ifstream &ifs);              // 重新造树
    int rwName(ifstream &ifs, string &output); // 读写文件名并返回名字长度
    void fileDehuf(ifstream &ifs);             // 解压缩文件

    void recursion(filesystem::path filePath, unsigned char &depth); // 递归取得目录信息
    void writeDir(ofstream &ofs);                                    // 写入目录
    void writeFiles(ofstream &ofs);                                  // 写入压缩后文件
    void folderHuf(filesystem::path filePath, ofstream &ofs);        // 文件夹压缩

    void readDir(ifstream &ifs);     // 读取目录
    void folderDehuf(ifstream &ifs); // 文件夹解压缩

    bool fileOverwrite(string str); // 文件覆写判断

public:
    void huf(string input, string output); // 压缩方法
    void dehuf(string input);              // 解压缩方法
    void preview(string input);            // 压缩包预览
};

#endif