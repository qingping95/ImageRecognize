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
//
//结束时需要释放pColorTable这个指针的内存
//
#include "IMAGE.h"
//
#include "IMAGE_DSU.h"
using namespace std;
#define CHECK(x, y) ((x) >= 0 && (x) < n && (y) >= 0 && (y) < m)
#define ID(x, y) ((x)*m+(y))
#define DEBUG(x) cout<<#x<<" -> "<<x<<endl
//K=2 十字相连
//K=4 星型相连(default)
void runUnite(ImageDsu &dsu, int n, int m, int K = 4)
{
    int dir[4][2] = {0, 1, 1, 0, 1, 1, 1, -1};
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            for(int k = 0; k < K; k++)
            {
                int cx = i + dir[k][0], cy = j + dir[k][1];
                if(!CHECK(cx, cy)) continue;
                if(dsu.color[ID(i, j)] == dsu.color[ID(cx, cy)])
                {
                    dsu.unite(ID(i, j), ID(cx, cy));
                }
            }
        }
    }
}

vector<int> getForeground(ImageDsu &Idsu, int Fore, int &mah, int &maw)
{
    vector<int> v;
    //cout<<Idsu.n<<endl;
    for(int i = 0; i < Idsu.n; i++)
    {
        int idx = Idsu.find(i);
        if(Idsu.color[idx] != Fore) continue;
        if(v.size() == 0) Idsu.printCom(idx);
        v.push_back(idx);
        mah = max(mah, Idsu.getHeight(idx));
        maw = max(maw, Idsu.getWidth(idx));
    }
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
    return v;
}
bool uniteCom(ImageDsu &Idsu, vector<int> v, int &mah, int &maw, bool use)
{
    bool isUpdate = false;
    for(int i = 0; i < v.size(); i++)
    {
        for(int j = i+1; j < v.size(); j++)
        {
            //cout<<"i : "<<v[i]<<", j : "<<v[j]<<endl;
            int U = Idsu.find(v[i]), V = Idsu.find(v[j]);
            if(U == V) continue;
            if(Idsu.check(U, V, mah, maw, use))
            {
                printf("%d -> %d\n", U, V);
                Idsu.printCom(U);
                Idsu.printCom(V);
                if(Idsu.unite(U, V)) isUpdate = true;
                int th = Idsu.getHeight(Idsu.find(U));
                if(mah < th) isUpdate = true, mah = th;
                int tw = Idsu.getWidth(Idsu.find(U));
                if(maw < tw) isUpdate = true, maw = tw;
            }
        }
    }
    return isUpdate;
}
void OCRAPI(char *fileName, char *result)
{
    string str = string(fileName)+" ";
    str += result;
    str += " -l chi_sim -psm 8";
    printf("正在执行OCR文字识别...\n");
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
std::string ws2s(const std::wstring& ws)
{
    std::string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
    setlocale(LC_ALL, "chs");
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = 2 * ws.size() + 1;
    char *_Dest = new char[_Dsize];
    memset(_Dest,0,_Dsize);
    wcstombs(_Dest,_Source,_Dsize);
    std::string result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
bool WStringToString(const std::wstring &wstr,std::string &str)
{
    int nLen = (int)wstr.length();
    str.resize(nLen,' ');

    int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)wstr.c_str(),nLen,(LPSTR)str.c_str(),nLen,NULL,NULL);

    if (nResult == 0)
    {
        return FALSE;
    }

    return TRUE;
}
std::wstring s2ws(const std::string& s)
{
    setlocale(LC_ALL, "chs");
    const char* _Source = s.c_str();
    size_t _Dsize = s.size() + 1;
    wchar_t *_Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    std::wstring result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, "C");
    return result;
}
void printfile(char *file)
{
    strcat(file, ".txt");
    freopen(file, "r", stdin);

    setlocale(LC_ALL,"chs");
    wchar_t cc[222];
    wscanf(L"%s", cc);
    int len = wcslen(cc);
//        cout<<len<<endl;
    for(int i = 0; i < len; i++)
        wprintf(L"%c: %hx\n",cc[i],cc[i]);

    wprintf(L"%s\n", cc);

    string str;
    wstring wscc = cc;
    wcout<<"wstring :"<<wscc<<endl;
    wprintf(L"wchar: %s\n", wscc.c_str());
    WStringToString(wscc, str);

    cout<<"asdf"<<endl;
    cout<<str<<endl;
}
int main()
{
    char fileName[] = "F:/506622.bmp";
    char result[] = "F:/result";
    //OCRAPI(fileName, result);
    printfile(result);
    return 0;
    //freopen("outinfo.txt", "w", stdout);
    int BACK = 0, FORE = 1;
    int height, width, biBitCount;
    char input[] = "bit-biaoge.bmp";
    char output[] = "denoise-biaoge-bit.bmp";
    char *outputPath = new char[111];
    strcpy(outputPath, "biaoge/After-cut-final/");

    unsigned char *ImageData; //图像数据
    readBmp(input, ImageData, width, height, biBitCount, BACK, FORE);
    int lineByte=calLineByte(width, biBitCount);//灰度图像有颜色表，且颜色表表项为256

    int realWidth = lineByte*8/biBitCount;
    unsigned char* realData = new unsigned char[height*realWidth];

    formatToReal(realData, ImageData, height, lineByte, biBitCount);

    ImageDsu Idsu(height, realWidth);

    //导入颜色数据到并查集
    Idsu.importAttr(realData);

    //合并各分量
    runUnite(Idsu, height, realWidth, 4);

    //降噪
    Idsu.denoise(BACK, FORE, width, 30);

//    //转换给ImageData
    Idsu.exportAttr(realData);
    realToFormat(ImageData, realData, height, realWidth, biBitCount);

    saveBmp(output, ImageData, width, height, biBitCount, pColorTable);

    //做合并偏旁部首
    int mah = 0, maw = 0;
    vector<int> FG = getForeground(Idsu, FORE, mah, maw);
    DEBUG(FG.size());

    DEBUG(mah);
    DEBUG(maw);

    int times = 0;
    bool use = false;
    while(true)
    {
        if(!uniteCom(Idsu, FG, mah, maw, use))
        {
            if(use == false) use = true;
            else break;
        }
        //clear useless index
        for(int i = 0; i < FG.size(); i++)
        {
            FG[i] = Idsu.find(FG[i]);
            mah = max(mah, Idsu.getHeight(FG[i]));
            maw = max(maw, Idsu.getWidth(FG[i]));
        }
        sort(FG.begin(), FG.end());
        FG.erase(unique(FG.begin(), FG.end()), FG.end());
        times++;
        DEBUG(times);
        DEBUG(mah);
        DEBUG(maw);
    }
    cout<<"迭代次数为："<<times<<endl;
    DEBUG(mah);
    DEBUG(maw);

    for(int i = 0; i < FG.size(); i++)
    {
        FG[i] = Idsu.find(FG[i]);
    }
    sort(FG.begin(), FG.end());
    FG.erase(unique(FG.begin(), FG.end()), FG.end());

    //save each component
    //初始化颜色表
//    if(pColorTable) delete[] pColorTable;
//    pColorTable = new RGBQUAD[2];
//    initColorTable(pColorTable);



    int len = strlen(outputPath);
    for(int i = 0; i < FG.size(); i++)
    {
        int th, tw;
        unsigned char* comData;
        Idsu.exportCom(FG[i], comData, th, tw);
        realToFormat(ImageData, comData, th, tw, biBitCount);
        sprintf(outputPath+len, "%d", Idsu.find(FG[i]));
        strcat(outputPath, ".bmp");
        cout<<"Image save in : "<<outputPath<<endl;

        saveBmp(outputPath, ImageData, tw, th, biBitCount, pColorTable);

        if(comData) delete[]comData;
    }

    //回收内存
    if(outputPath) delete []outputPath;
    if(pColorTable) delete []pColorTable;
    if(realData) delete []realData;
    if(ImageData) delete []ImageData;
    return 0;
}
