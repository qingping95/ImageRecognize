#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
//
//结束时需要释放pColorTable这个指针的内存
//
#include "IMAGE.h"
//
#include "IMAGE_DSU.h"
using namespace std;
#define CHECK(x, y) ((x) >= 0 && (x) < n && (y) >= 0 && (y) < m)
#define ID(x, y) ((x)*m+(y))

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
void Denoise(char *input, char *output)
{
    int height, width, biBitCount;
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
    Idsu.denoise(0, 1, 30);

    //导出分量数据
    Idsu.exportAttr(realData);

    //转换给ImageData
    realToFormat(ImageData, realData, height, realWidth, biBitCount);

    //初始化颜色表
    if(pColorTable) delete[] pColorTable;
    pColorTable = new RGBQUAD[2];
    initColorTable(pColorTable);

    saveBmp(output, ImageData, width, height, biBitCount, pColorTable);

    //回收内存
    if(pColorTable) delete []pColorTable;
    if(realData) delete []realData;
    if(ImageData) delete []ImageData;
}
int main()
{
//    make_bmp(31, 31, 1, "black-white.bmp");
    Denoise("test_shouxie.bmp", "after_denoise4.bmp");
    return 0;
}
