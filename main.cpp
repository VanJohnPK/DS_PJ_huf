#include "huf.h"

int main(int argc, char *argv[])
{
    string input, output;
    string todo;
    HufTool hufTool;
    // 处理参数
    int i = 0;
    while (argv[1][i] != '\0')
    {
        todo.push_back(argv[1][i]);
        i++;
    }
    if (todo == "help")
    {
        cout << "the first argument(necessary) can be 'help' 'huf' 'dehuf' and 'preview'" << endl;
        cout << "the second argument(necessary for huf, dehuf and preview) is the name of input" << endl;
        cout << "the third argument(only optional for huf) is the name of output, if you forget to write suffix '.huf', I will do it for you" << endl;
        return 0;
    }
    i = 0;
    while (argv[2][i] != '\0')
    {
        input.push_back(argv[2][i]);
        i++;
    }
    i = 0;
    if (argv[3] != nullptr)
    {
        while (argv[3][i] != '\0')
        {
            output.push_back(argv[3][i]);
            i++;
        }
    }
    // 参数处理结束
    if (todo == "huf")
    {
        hufTool.huf(input, output); // 压缩方法
    }
    else if (todo == "dehuf")
    {
        hufTool.dehuf(input); // 解压缩方法
    }
    else if (todo == "preview")
    {
        hufTool.preview(input); // 压缩包预览
    }
    else
    {
        cout << "I don't know what you want to do." << endl;
    }
    return 0;
}