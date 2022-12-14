#include "huf.h"

void HufTool::find2Min(int num, int &m1, int &m2) // 找到权重最小的两个根节点
{
    int min1 = INF;               // 权重最小的节点的权
    int min2 = INF;               // 权重次小的节点的权
    for (int i = 0; i < num; i++) // 用i<num而不是i<Max可以减少循环次数,节约时间
    {
        if (HT[i].weight <= min1 && HT[i].parent == -1)
        {
            min1 = HT[i].weight;
            m1 = i;
        }
    }
    for (int i = 0; i < num; i++) // 用i<num而不是i<Max可以减少循环次数,节约时间
    {
        if (HT[i].weight <= min2 && i != m1 && HT[i].parent == -1)
        {
            min2 = HT[i].weight;
            m2 = i;
        }
    }
    // cout << "m1 =" << m1 << " m2 =" <<m2 << endl;测试用
}

void HufTool::makeHT() // 初始化以及造树
{
    // 初始化
    for (int i = 0; i < N; i++)
    {
        HT[i].weight = nodeCnt[i];
        HT[i].parent = -1;
        HT[i].lchild = -1;
        HT[i].rchild = -1;
        HT[i].length = 0;
        HT[i].hufCode = NULL;
    }
    for (int i = N; i < MAX; i++)
    {
        HT[i].weight = 0;
        HT[i].parent = -1;
        HT[i].lchild = -1;
        HT[i].rchild = -1;
        HT[i].length = 0;
        HT[i].hufCode = NULL;
    }

    // 造树
    int m1 = -1; // 权重最小的节点
    int m2 = -1; // 权重次小的节点
    for (int i = N; i < MAX; i++)
    {
        find2Min(i, m1, m2);
        if (m1 < m2)
        {
            HT[i].lchild = m1;
            HT[i].rchild = m2;
        }
        else
        {
            HT[i].lchild = m2;
            HT[i].rchild = m1;
        }
        HT[i].weight = HT[m1].weight + HT[m2].weight;
        HT[m1].parent = i;
        HT[m2].parent = i;
        root = i;
    }
}

void HufTool::encode() // 编码
{
    int now; // 正在编码的节点
    int prt; // 正在编码的节点的父亲节点
    int len;
    for (int i = 0; i < N; i++)
    {
        len = 0;
        now = i;
        prt = HT[now].parent;
        while (prt != -1)
        {
            len++;
            now = prt;
            prt = HT[now].parent;
        }
        HT[i].length = len;
        HT[i].hufCode = new int[len];
        now = i;
        prt = HT[now].parent;
        while (prt != -1)
        {
            len--;
            if (HT[prt].lchild == now)
            {
                HT[i].hufCode[len] = 0;
            }
            else
            {
                HT[i].hufCode[len] = 1;
            }
            now = prt;
            prt = HT[now].parent;
        }
    }
}

void HufTool::nodeCount(string input) // 统计出现次数
{
    ifstream ifs;
    char ch;
    ifs.open(input, ios_base::binary);
    if (!ifs)
    {
        exit(1);
    }
    while (ifs.get(ch))
    {
        nodeCnt[(unsigned char)ch]++;
    }
    ifs.close();
}

void HufTool::writeHT(string input, ofstream &ofs) // 写入文件
{
    ifstream ifs;
    char ch;
    char byte = 0;
    int cnt = 0;
    long long length = 0;
    ifs.open(input, ios_base::binary);
    for (int i = 0; i < 8; i++) // 留给之后写入压缩后文件内容的字节数
    {
        ofs.put(0);
    }
    ofs.put(0);           // 留给之后写入补位数
    ofs.put(root - PMOD); // 写入根节点id
    // 写入树
    for (int i = 0; i < N * 2 - 1; i++)
    {
        if (HT[i].parent != -1)
        {
            ofs.put(HT[i].parent - PMOD);
        }
        else
        {
            ofs.put(0x7F); // 0X7F代表无父节点的节点
        }
    }
    // 写入名字
    unsigned char nameLength = input.size();
    ofs.put(nameLength);
    for (int i = 0; i < nameLength; i++)
    {
        ofs.put(input[i]);
    }
    // 写入压缩后内容
    while (ifs.get(ch))
    {
        for (int i = 0; i < HT[(unsigned char)ch].length; i++)
        {
            if (cnt == 8)
            {
                ofs.put(byte);
                byte = 0;
                cnt = 0;
                length++;
            }
            byte = (byte << 1) | HT[(unsigned char)ch].hufCode[i];
            cnt++;
        }
    }
    // 补0以凑齐一个字节
    bool flag = false;
    if (cnt > 0)
    {
        for (int i = cnt; i < 8; i++)
        {
            byte = byte << 1;
        }
        ofs.put(byte);
        length++;
        ofs.seekp(-length - nameLength - 514, ios_base::cur);
        ofs.put(8 - cnt); // 把补0数写到前面
        flag = true;
    }
    // 调整put的位置以适应文件夹压缩
    if (flag)
    {
        ofs.seekp(-9, ios_base::cur);
    }
    else
    {
        ofs.seekp(-length - nameLength - 522, ios_base::cur);
    }
    for (int i = 0; i < 8; i++) // 把length按小端存入开头
    {
        int temp = length % 256;
        if (temp > 127)
        {
            temp -= 256;
        }
        char ch = temp;
        ofs.put(ch);
        length /= 256;
    }
    ofs.seekp(0, ios_base::end);
    ifs.close();
}

void HufTool::fileHuf(string input, ofstream &ofs) // 压缩文件
{
    nodeCount(input);    // 统计出现次数
    makeHT();            // 初始化以及造树
    encode();            // 编码
    writeHT(input, ofs); // 写入文件
}

void HufTool::remakeHT(ifstream &ifs) // 重新造树
{
    char ch;
    ifs.get(cover); // 获取补位数量
    ifs.get(ch);    // 获取根节点下标
    root = ch + PMOD;

    // 初始化
    for (int i = 0; i < 2 * N - 1; i++)
    {
        HT[i].lchild = -1;
        HT[i].rchild = -1;
    }

    for (int i = 0; i < 2 * N - 1; i++)
    {
        ifs.get(ch);
        if (ch == 0x7F) // 0X7F代表无父节点的节点
        {
            HT[i].parent = -1;
        }
        else
        {
            HT[i].parent = ch + PMOD;
        }
    }
    for (int i = 0; i < N * 2 - 1; i++)
    {
        if (HT[i].parent != -1)
        {
            // 下标小的是左孩子
            int p = HT[i].parent;
            if (HT[p].lchild == -1)
            {
                HT[p].lchild = i;
            }
            else if (HT[p].rchild == -1)
            {
                HT[p].rchild = i;
            }
        }
    }
}

int HufTool::rwName(ifstream &ifs, string &output) // 读写文件名并返回名字长度
{
    int nameLength = 0;
    char temp;
    ifs.get(temp);
    nameLength = temp;
    if (nameLength < 0)
    {
        nameLength += 256;
    }
    for (int i = 0; i < nameLength; i++)
    {
        ifs.get(temp);
        output.push_back(temp);
    }
    // cout << output << endl;
    return nameLength;
}

long long HufTool::getLength(ifstream &ifs) // 读取并计算返回文件长度
{
    long long length = 0;
    long long factor = 1;
    for (int i = 0; i < 8; i++)
    {
        char ch;
        ifs.get(ch);
        int temp = ch;
        if (temp < 0)
        {
            temp += 256;
        }
        length = length + temp * factor;
        factor *= 256;
    }
    return length;
}

void HufTool::fileDehuf(ifstream &ifs) // 解压缩文件
{
    ofstream ofs;
    string output;
    long long length = getLength(ifs);    // 获取长度
    remakeHT(ifs);                        // 重新造树
    int nameLength = rwName(ifs, output); // 读写文件名并返回名字长度
    if (fileOverwrite(output))
    {
        ofs.open(output, ios_base::binary);
        if (!ofs)
        {
            ifs.close();
            exit(1);
        }
        int now, bit, cnt = 0;
        char byte;
        for (long long i = 0; i < length * 8 - cover;)
        {
            now = root;
            while (HT[now].lchild != -1)
            {
                if (cnt % 8 == 0)
                {
                    ifs.get(byte); // 每次读一个字节
                    cnt = 0;
                }
                bit = byte & 0x80; // 获取当前的比特位,也就是byte的首位
                if (bit == 0)
                {
                    now = HT[now].lchild;
                }
                else
                {
                    now = HT[now].rchild;
                }
                byte = byte << 1; // 字节左移一位，以便下一次获取1bit
                cnt++;
                i++;
            }
            if (now > 127) // 范围控制在-128到127之间,防止输入值超出char型的范围
            {
                now -= 256;
            }
            ofs.put(now);
        }
    }
    ofs.close();
}

void HufTool::recursion(filesystem::path filePath, unsigned char &depth) // 递归取得目录信息
{
    DirNode temp;
    temp.pathStr = filePath.string();
    temp.len = filePath.string().size();
    temp.dep = depth;
    dirVec.push_back(temp);
    for (const auto &entry : filesystem::directory_iterator(filePath))
    {
        if (entry.is_directory()) // 若是文件夹
        {
            filesystem::path subPath = entry;
            depth++;
            recursion(subPath, depth); // 递归
            depth--;
        }
        else // 若是文件
        {
            temp.pathStr = entry.path().string();
            temp.len = entry.path().string().size();
            temp.dep = depth + 1;
            temp.isFile = true;
            dirVec.push_back(temp);
        }
    }
}

void HufTool::writeDir(ofstream &ofs) // 写入目录
{
    int num = dirVec.size();
    for (int i = 0; i < 4; i++) // 把num按小端存入开头
    {
        int temp = num % 256;
        if (temp > 127)
        {
            temp -= 256;
        }
        char ch = temp;
        ofs.put(ch);
        num /= 256;
    }
    for (int i = 0; i < dirVec.size(); i++)
    {
        ofs.put(dirVec[i].len);                            // 1字节
        ofs.put(dirVec[i].dep);                            // 1字节
        ofs.put(dirVec[i].isFile);                         // 1字节
        for (int j = 0; j < dirVec[i].pathStr.size(); j++) // 开头的.\属于冗余信息,可以不记录
        {
            ofs.put(dirVec[i].pathStr[j]);
        }
    }
}

void HufTool::writeFiles(ofstream &ofs) // 写入压缩后文件
{
    for (int i = 0; i < dirVec.size(); i++)
    {
        if (dirVec[i].isFile)
        {
            fileHuf(dirVec[i].pathStr, ofs); // 文件压缩
        }
    }
}

void HufTool::folderHuf(filesystem::path filePath, ofstream &ofs) // 文件夹压缩
{
    unsigned char depth = 0;
    recursion(filePath, depth); // 递归取得目录信息
    writeDir(ofs);              // 写入目录
    writeFiles(ofs);            // 写入压缩后文件
}

void HufTool::readDir(ifstream &ifs) // 读取目录
{
    int num = 0;
    int factor = 1;
    for (int i = 0; i < 4; i++)
    {
        char ch;
        ifs.get(ch);
        int temp = ch;
        if (temp < 0)
        {
            temp += 256;
        }
        num = num + temp * factor;
        factor *= 256;
    }
    // 重建目录
    for (int i = 0; i < num; i++)
    {
        DirNode temp;
        char ch;
        ifs.get(ch);
        temp.len = ch;
        ifs.get(ch);
        temp.dep = ch;
        ifs.get(ch);
        temp.isFile = ch;
        for (int j = 0; j < temp.len; j++)
        {
            char ch;
            ifs.get(ch);
            temp.pathStr.push_back(ch);
        }
        dirVec.push_back(temp);
    }
}

void HufTool::folderDehuf(ifstream &ifs) // 文件夹解压缩
{
    readDir(ifs); // 读取目录
    for (int i = 0; i < dirVec.size(); i++)
    {
        filesystem::path filePath = dirVec[i].pathStr;
        if (dirVec[i].isFile)
        {
            fileDehuf(ifs); // 文件解压
        }
        else
        {
            filesystem::create_directory(filePath); // 创建目录
        }
    }
}

bool HufTool::fileOverwrite(string str) // 文件覆写判断
{
    bool flag = true;
    filesystem::path filePath = ".\\";
    filePath.append(str);
    if (filesystem::is_regular_file(filePath))
    { // 压缩后的文件已存在
        cout << str << " already exists. Overwrite it?" << endl;
        cout << "type 'y' or 'yes' if you want; type anything else like 'n' or 'no' if you don't want" << endl;
        string ans;
        cin >> ans;
        if (ans != "y" && ans != "yes")
        {
            flag = false;
        }
    }
    return flag;
}

void HufTool::huf(string input, string output) // 压缩方法
{
    filesystem::path filePath = ".\\";
    filePath.append(input);
    if (output.size() == 0) // 默认的名字
    {
        output += filePath.stem().string();
        output += ".huf";
    }
    else // 保证后缀为.huf,如果输入中已经给了.huf则不作任何改动
    {
        string suffix;
        int size = output.size();
        if (output.size() < 4)
        {
            output += ".huf";
            size += 4;
        }
        for (int i = size - 4; i < size; i++)
        {
            suffix.push_back(output[i]);
        }
        if (suffix != ".huf")
        {
            output += ".huf";
        }
    }
    if (fileOverwrite(output))
    {
        if (filesystem::is_directory(filePath)) // 是文件夹
        {
            ofstream ofs;
            ofs.open(output, ios_base::binary);
            ofs.put(true);
            folderHuf(filePath, ofs);
            ofs.close();
        }
        else if (filesystem::is_regular_file(filePath))
        {
            ofstream ofs;
            ofs.open(output, ios_base::binary);
            ofs.put(false);
            fileHuf(input, ofs);
            ofs.close();
        }
        else
        {
            cout << "input doesm't exist" << endl;
        }
    }
    // else // 测试用
    // {
    //     cout << "exist as you wish" << endl;
    // }
}

void HufTool::dehuf(string input) // 解压缩方法
{
    string suffix;
    int size = input.size();
    for (int i = size - 4; i < size; i++)
    {
        suffix.push_back(input[i]);
    }
    // 判断后缀,后缀不同,肯定是不同来源
    if (suffix == ".huf")
    {
        ifstream ifs;
        ifs.open(input, ios_base::binary);
        if (!ifs)
        {
            exit(1);
        }
        char ch;
        ifs.get(ch);
        // 所有huf文件开头都有占1个字节的标志,用来区分是文件压缩还是文件夹压缩,它的值只可能是0或1,可以应对绝大部分其他来源却有相同后缀的情况
        if (ch == 0 || ch == 1)
        {
            bool isFolder;
            isFolder = ch;
            if (isFolder)
            {
                folderDehuf(ifs);
            }
            else
            {
                fileDehuf(ifs);
            }
        }
        else
        {
            cout << "This is not created by me, unable to dehuf(decompress)" << endl;
        }
    }
    else
    {
        cout << "This is not created by me, unable to dehuf(decompress)" << endl;
    }
}

void HufTool::preview(string input) // 压缩包预览
{
    string suffix;
    int size = input.size();
    for (int i = size - 4; i < size; i++)
    {
        suffix.push_back(input[i]);
    }
    // 判断后缀,后缀不同,肯定是不同来源
    if (suffix == ".huf")
    {
        ifstream ifs;
        ifs.open(input, ios_base::binary);
        if (!ifs)
        {
            exit(1);
        }
        char ch;
        ifs.get(ch);
        // 所有huf文件开头都有占1个字节的标志,用来区分是文件压缩还是文件夹压缩,它的值只可能是0或1,可以应对绝大部分其他来源却有相同后缀的情况
        if (ch == 0 || ch == 1)
        {
            bool isFolder;
            isFolder = ch;

            if (isFolder)
            {
                readDir(ifs); // 读取目录
                vector<string> strVec;
                for (int i = 0; i < dirVec.size(); i++)
                {
                    string str;
                    int dep = dirVec[i].dep;
                    if (dep > 0)
                    {
                        for (int j = 1; j < dep; j++)
                        {
                            str += "    ";
                        }
                        str += "|__ ";
                    }
                    filesystem::path filePath = dirVec[i].pathStr;
                    str += filePath.filename().string();
                    strVec.push_back(str);
                }
                for (int row = strVec.size() - 1; row > 0; row--)
                {
                    int size = strVec[row].size();
                    for (int column = 0; column < size; column += 4)
                    {
                        if (strVec[row][column] == '|' && strVec[row - 1][column] == ' ')
                        {
                            strVec[row - 1][column] = '|';
                        }
                    }
                }
                for (int i = 0; i < strVec.size(); i++)
                {
                    cout << strVec[i] << endl;
                }
            }
            else
            {
                string name;
                ifs.seekg(521, ios_base::cur);
                rwName(ifs, name); // 读写文件名
                cout << "name: '" << name << "'" << endl;
            }
        }
        else
        {
            cout << "This is not created by me, unable to preview" << endl;
        }
    }
    else
    {
        cout << "This is not created by me, unable to preview" << endl;
    }
}