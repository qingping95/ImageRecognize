#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
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

vector<int> getForeground(ImageDsu &Idsu, int Fore)
{
    vector<int> v;
    //cout<<Idsu.n<<endl;
    for(int i = 0; i < Idsu.n; i++)
    {
        int idx = Idsu.find(i);
        if(Idsu.color[idx] != Fore) continue;
        v.push_back(idx);
    }
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
    return v;
}
bool uniteCom(ImageDsu &Idsu, vector<int> v)
{
    bool isUpdate = false;
    for(int i = 0; i < v.size(); i++)
    {
        for(int j = i+1; j < v.size(); j++)
        {
            //cout<<"i : "<<v[i]<<", j : "<<v[j]<<endl;
            int U = Idsu.find(v[i]), V = Idsu.find(v[j]);
            if(U == V) continue;
            if(Idsu.check(U, V))
            {
                if(Idsu.unite(U, V)) isUpdate = true;
            }
        }
    }
    return isUpdate;
}
int main()
{
//    char *outputPath =new char[111];
//    strcpy(outputPath, "After-cur/");
//    sprintf(outputPath+strlen(outputPath), "%d", 111);
//    strcat(outputPath, ".bmp");
//    cout<<outputPath<<endl;

//    make_bmp(31, 31, 1, "black-white.bmp");

    const int BACK = 0, FORE = 1;
    int height, width, biBitCount;
    char input[] = "test_shouxie.bmp";
    char output[] = "after_denoise4.bmp";

    unsigned char *ImageData; //图像数据
    readBmp(input, ImageData, width, height, biBitCount);
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
    Idsu.denoise(BACK, FORE, 30);

    //做合并偏旁部首
    vector<int> FG = getForeground(Idsu, FORE);
    int times = 0;
    while(uniteCom(Idsu, FG)){
        //clear useless index
        for(int i = 0; i < FG.size(); i++)
        {
            FG[i] = Idsu.find(FG[i]);
        }
        sort(FG.begin(), FG.end());
        FG.erase(unique(FG.begin(), FG.end()), FG.end());
        times++;
    }
    cout<<"迭代次数为："<<times<<endl;

    for(int i = 0; i < FG.size(); i++)
    {
        FG[i] = Idsu.find(FG[i]);
    }
    sort(FG.begin(), FG.end());
    FG.erase(unique(FG.begin(), FG.end()), FG.end());

    //save each component
    //初始化颜色表
    if(pColorTable) delete[] pColorTable;
    pColorTable = new RGBQUAD[2];
    initColorTable(pColorTable);

    char *outputPath = new char[111];
    strcpy(outputPath, "After-cut3/");
    int len = strlen(outputPath);
    for(int i = 0; i < FG.size(); i++)
    {
        int th, tw;
        unsigned char* comData;
        Idsu.exportCom(FG[i], comData, th, tw);
        realToFormat(ImageData, comData, th, tw, biBitCount);
        sprintf(outputPath+len, "%d", FG[i]);
        strcat(outputPath, ".bmp");
        cout<<"Image save in : "<<outputPath<<endl;

        saveBmp(outputPath, ImageData, tw, th, biBitCount, pColorTable);

        if(comData) delete[]comData;
    }
    //存储图像
    //导出分量数据
//    Idsu.exportAttr(realData);
//
//    //转换给ImageData
//    realToFormat(ImageData, realData, height, realWidth, biBitCount);
//
//
//    saveBmp(output, ImageData, width, height, biBitCount, pColorTable);

    //回收内存
    if(outputPath) delete []outputPath;
    if(pColorTable) delete []pColorTable;
    if(realData) delete []realData;
    if(ImageData) delete []ImageData;
    //Denoise("test_shouxie.bmp", "after_denoise4.bmp");
    return 0;
}
