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
#include <string>
#include <sstream>
//
//结束时需要释放pColorTable这个指针的内存
//
#include "IMAGE.h"
#include "OCR.h"
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
        //if(v.size() == 0) Idsu.printCom(idx);
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
//                printf("%d -> %d\n", U, V);
//                Idsu.printCom(U);
//                Idsu.printCom(V);
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

bool runOCR()
{
    //    //OCR
    printf("OCR process...\n");
    char fileName[] = "result";
    vector<string> files;
    getFiles(fileName, files);
    int len = files.size();
    int idx = 0;
    for(string file : files)
    {
        string oldName = file;
        DEBUG(oldName);
        string info = oldName;
        info.insert(info.find('\\'), "txt");
        DEBUG(info);
        char unicodeOutput[222];
        strcpy(unicodeOutput, info.c_str());
        OCRAPI(oldName.c_str(), unicodeOutput);
        string uni = getUnicode(unicodeOutput)+".bmp";
        string newName = file.replace(file.rfind('\\')+1, file.length(), uni.c_str());
        DEBUG(newName);
        rename(oldName.c_str(), newName.c_str());
        printf("%d / %d\n", ++idx, files.size());
    }
    return true;
}
void binaryzation()
{
    int height, width, biBitCount;
    char input[] = "gray-test.bmp";
    char output[] = "denoise-random-gray.bmp";
    char *outputPath = new char[111];
    strcpy(outputPath, "random/After-cut-final/");

    int b, f;
    unsigned char *ImageData; //图像数据
    readBmp(input, ImageData, width, height, biBitCount, b, f);
    int lineByte24 = calLineByte(width, 24);
//    unsigned char *real = new unsigned char[height*lineByte24];
//    formatToReal(real, ImageData, height, lineByte24, biBitCount);
//    realToFormat(ImageData, real, height, lineByte24/3, biBitCount);
//    saveBmp(output, ImageData, width, height, biBitCount, pColorTable);

    int lineByte8 = calLineByte(width, 8);
    unsigned char *gray = new unsigned char[height * lineByte8];
    if(biBitCount == 24) rgb2gray(gray, ImageData, height, width);

    realToFormat(ImageData, gray, height, lineByte8, 8);

//    if(pColorTable) delete []pColorTable;
    pColorTable = new RGBQUAD[256];
    for(int i = 0; i < 256; i++)
        pColorTable[i] = (RGBQUAD){i, i, i, 0};
    saveBmp(output, ImageData, width, height, 8, pColorTable);

    delete []pColorTable;
    delete []outputPath;
    delete []ImageData;
    delete []gray;
}
int main()
{
    //runOCR
    //if(runOCR()) return 0;

    //run rgb2gray()
    binaryzation();
    return 0;
//    char O[222] = "F:\\result";
//    cout<<getUnicode(O)<<endl;
//    return 0;
    //freopen("outinfo.txt", "w", stdout);
    int BACK = 0, FORE = 1;
    int height, width, biBitCount;
    char input[] = "bit-random.bmp";
    char output[] = "denoise-random-bit.bmp";
    char *outputPath = new char[111];
    strcpy(outputPath, "random/After-cut-final/");

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
        //if(Idsu.find(FG[i]) != 1131212) continue;
        //Idsu.printCom(Idsu.find(FG[i]));
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
