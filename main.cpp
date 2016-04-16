#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <imagehlp.h>
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

vector<int> getForeground(ImageDsu &Idsu, int Fore, int &mah, int &maw)
{
    vector<int> v;
    //cout<<Idsu.n<<endl;
    for(int i = 0; i < Idsu.n; i++)
    {
        int idx = Idsu.find(i);
        if(Idsu.color[idx] != Fore) continue;
        v.push_back(idx);
        mah = max(mah, Idsu.getHeight(idx));
        maw = max(maw, Idsu.getWidth(idx));
    }
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
    return v;
}
bool uniteCom(ImageDsu &Idsu, vector<int> v, int &mah, int maw)
{
    bool isUpdate = false;
    for(int i = 0; i < v.size(); i++)
    {
        for(int j = i+1; j < v.size(); j++)
        {
            //cout<<"i : "<<v[i]<<", j : "<<v[j]<<endl;
            int U = Idsu.find(v[i]), V = Idsu.find(v[j]);
            if(U == V) continue;
            if(Idsu.check(U, V, mah, maw))
            {
                if(Idsu.unite(U, V)) isUpdate = true;
                int th = Idsu.getHeight(Idsu.find(U));
                if(mah < th) isUpdate = true, mah = th;
//                int tw = Idsu.getWidth(Idsu.find(U));
                //if(maw < tw) isUpdate = true, maw = tw;
            }
        }
    }
    return isUpdate;
}
void Bmp32ToBmp24(char Filename[])
{
    char Filename2[] = "output.bmp";


//注意：如果没有LR_CREATEDIBSECTION，位图颜色将被映射到屏幕DC颜色
//也就是说，如果屏幕是16位颜色，则所有的图像都将映射到16位颜色
    HBITMAP hbmp32 = (HBITMAP) LoadImage(NULL, Filename,
                                         IMAGE_BITMAP, 0, 0,
                                         LR_LOADFROMFILE |
                                         LR_CREATEDIBSECTION);


    BITMAP bmp;//获取位图信息
    GetObject(hbmp32, sizeof(BITMAP), &bmp);


    printf("Image Bit Depth : %dnWidth : %d , Height : %d n",
           bmp.bmBitsPixel, bmp.bmWidth, bmp.bmHeight);//显示位图颜色模式和图像宽高


//计算24位图像每行的字节数
    int BytesPerLine = 3 * bmp.bmWidth;
    while(BytesPerLine % 4 != 0)
        BytesPerLine ++;


    BITMAPINFOHEADER bih = {0};//位图信息头
    bih.biBitCount = 24;//每个像素字节大小
    bih.biCompression = BI_RGB;
    bih.biHeight = bmp.bmHeight;//高度
    bih.biPlanes = 1;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biSizeImage = BytesPerLine * bmp.bmHeight;//图像数据大小
    bih.biWidth = bmp.bmWidth;//宽度

    BITMAPFILEHEADER bfh = {0};//位图文件头
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
    bfh.bfSize = bfh.bfOffBits + bih.biSizeImage;//文件总的大小
    bfh.bfType = (WORD)0x4d42;

    FILE *fp = fopen(Filename2, "w+b");

    fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//写入位图文件头

    fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图信息头

    byte * p = new byte[bih.biSizeImage];

//获取当前32位图像数据
    GetDIBits(GetDC(NULL), hbmp32, 0, bmp.bmHeight, p, (LPBITMAPINFO)&bih, DIB_RGB_COLORS);


//只取rgb值，存入文件
    byte b = 0;//用于填充
    for(int i = 0 ; i < bmp.bmWidth * bmp.bmHeight ; i ++)
    {
        //32位位图图像的格式为：Blue, Green, Red, Alpha
        fwrite(&(p[i * 3]), 1, 3, fp);
        if(i % bmp.bmWidth == bmp.bmWidth - 1)//填充字节
        {
            for(int k = 0 ; k < (BytesPerLine - bmp.bmWidth * 3) ; k ++)
                fwrite(&b, sizeof(byte), 1, fp);
        }
    }

    delete [] p;

    fclose(fp);


    DeleteObject(hbmp32);
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
    Idsu.denoise(BACK, FORE, width, 30);

    //做合并偏旁部首
    int mah = 0, maw = 0;
    vector<int> FG = getForeground(Idsu, FORE, mah, maw);

    DEBUG(mah);
    DEBUG(maw);

    int times = 0;
    while(uniteCom(Idsu, FG, mah, maw))
    {
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
    if(pColorTable) delete[] pColorTable;
    pColorTable = new RGBQUAD[2];
    initColorTable(pColorTable);

    char *outputPath = new char[111];
    strcpy(outputPath, "After-cut-size4/");
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
