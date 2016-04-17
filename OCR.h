#ifndef OCR_H_INCLUDED
#define OCR_H_INCLUDED

#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>
#include <stdio.h>
#include <locale>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
#include <shellapi.h>
#include <io.h>
#include <sstream>


void OCRAPI(const char *fileName, char *result)
{
    string str = string(fileName)+" ";
    str += result;
    str += " -l chi_sim -psm 8";
    printf("%s\n", str.c_str());
//    cout<<str<<endl;
    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = "C:/Program Files (x86)/Tesseract-OCR/tesseract.exe";
    ShExecInfo.lpParameters = str.c_str();
    //ShExecInfo.lpParameters = "F:/506622.bmp F:/result -l chi_sim -psm 8";
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
}

string getUnicode(char *file)
{
    int len = strlen(file);
    strcat(file, ".txt");
    freopen(file, "r", stdin);

    setlocale(LC_ALL,"chs");
    wchar_t cc[222];
    wscanf(L"%s", cc);
//    for(int i = 0; i < len; i++){
//        int UNICODE = cc[i];
//        wprintf(L"%c: %d\n",cc[i],UNICODE);
//    }
    string str;
    stringstream ss;
    ss << hex << cc[0];
    ss >> str;
    file[len] = '\0';
    return str;
    //wprintf(L"%s\n", cc);
}
void getFiles( string path, vector<string>& files )
{
    //文件句柄
    long   hFile   =   0;
    //文件信息
    struct _finddata_t fileinfo;
    string p;
    if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)
    {
        do
        {
            //如果是目录,迭代之
            //如果不是,加入列表
            if((fileinfo.attrib &  _A_SUBDIR))
            {
                if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
                    getFiles( p.assign(path).append("\\").append(fileinfo.name), files );
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
            }
        }while(_findnext(hFile, &fileinfo)  == 0);
        _findclose(hFile);
    }
}
#endif // OCR_H_INCLUDED
