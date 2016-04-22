#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED
#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

#define DEBUG(x) cout<<#x<<" -> "<<x<<endl

RGBQUAD *pColorTable;//颜色表指针

int calLineByte(int width, int biBitCount)
{
    return ((width * biBitCount+7)/8+3)/4*4;
}
//-------------------------------------------------------------------------------------------
//读图像的位图数据、宽、高、颜色表及每像素位数等数据进内存，存放在相应的全局变量中
bool readBmp(char *bmpName, unsigned char* &ImageData, int& bmpWidth, int& bmpHeight, int& biBitCount, int &BACK, int &FORE)
{
    FILE *fp=fopen(bmpName,"rb");//二进制读方式打开指定的图像文件

    if(fp==0) return 0;

    //读取位图文件头结构BITMAPFILEHEADER
    fseek(fp, sizeof(BITMAPFILEHEADER), 0);
    //fread(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);

    //BITMAPINFOHEADER head;
    BITMAPINFOHEADER bmphead;

    fread(&bmphead, sizeof(BITMAPINFOHEADER), 1,fp); //获取图像宽、高、每像素所占位数等信息
    bmpWidth = bmphead.biWidth;
    bmpHeight = bmphead.biHeight;
    biBitCount = bmphead.biBitCount;//定义变量，计算图像每行像素所占的字节数（必须是4的倍数）
//    int biClrUsed = bmphead.biClrUsed;

    //输出图像的信息

    cout<<"width = "<<bmpWidth<<" height = "<<bmpHeight<<" biBitCount = "<<biBitCount<<endl;
    DEBUG(bmphead.biSizeImage);
    DEBUG(bmphead.biCompression);
    int lineByte=((bmpWidth * biBitCount+7)/8+3)/4*4;//灰度图像有颜色表，且颜色表表项为256
    if(bmphead.biClrUsed != 0)
    {
        //申请颜色表所需要的空间，读颜色表进内存
        pColorTable = new RGBQUAD[bmphead.biClrUsed];
        fread(pColorTable,sizeof(RGBQUAD),bmphead.biClrUsed, fp);
    }else {
        pColorTable = new RGBQUAD[1<<biBitCount];
        fread(pColorTable,sizeof(RGBQUAD),1<<biBitCount, fp);
    }
    if(pColorTable[0].rgbBlue == 255) BACK = 0, FORE = 1;
    else BACK = 1, FORE = 0;
//    DEBUG((int)pColorTable[0].rgbBlue);
//    DEBUG((int)pColorTable[0].rgbGreen);
//    DEBUG((int)pColorTable[0].rgbRed);
//    DEBUG((int)pColorTable[1].rgbBlue);
//    DEBUG((int)pColorTable[1].rgbGreen);
//    DEBUG((int)pColorTable[1].rgbRed);
    //申请位图数据所需要的空间，读位图数据进内存
    ImageData = new unsigned char[bmpHeight*lineByte];
    fread(ImageData, 1, bmpHeight*lineByte, fp);

    fclose(fp);//关闭文件
    return 1;//读取文件成功
}
bool read32bmp(char *bmpName, unsigned char* &ImageData, int& bmpWidth, int &bmpHeight, int &biBitCount)
{
    FILE *fp = fopen(bmpName, "rb");
    if(fp == 0) return 0;
    fseek(fp, sizeof(BITMAPFILEHEADER), 0);

    BITMAPINFOHEADER bmphead;
    fread(&bmphead, sizeof(BITMAPINFOHEADER), 1, fp);
     bmpWidth = bmphead.biWidth;
    bmpHeight = bmphead.biHeight;
    biBitCount = bmphead.biBitCount;//定义变量，计算图像每行像素所占的字节数（必须是4的倍数）

    //输出图像的信息
    cout<<"width = "<<bmpWidth<<" height = "<<bmpHeight<<" biBitCount = "<<biBitCount<<endl;

    int lineByte = ((bmpWidth * biBitCount)/8+3)/4*4;
    ImageData = new unsigned char[bmpHeight*lineByte];
    fread(ImageData, 1, bmpHeight*lineByte, fp);

    fclose(fp);
    return true;
}
void rgb2gray(unsigned char *gray, unsigned char* rgb, int n, int m)
{
    int lineByte24 = calLineByte(m, 24);
    int lineByte8 = calLineByte(m, 8);
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < lineByte24; j += 3)
        {
            int B = rgb[i*lineByte24+j];
            int G = rgb[i*lineByte24+j+1];
            int R = rgb[i*lineByte24+j+2];
            gray[i*lineByte8+j/3] = (int)(0.229*R + 0.587*G + 0.114*B);
        }
    }
}
#define D(x) ((x)*(x))
void OTSU(unsigned char *bit, unsigned char* gray, int n, int m)
{
    int *num = new int[256];
    memset(num, 0, sizeof(int)*256);
    for(int i = 0; i < n; i++)
        for(int j = 0; j < m; j++)
            num[gray[i*m+j]]++;

    //计算平均灰度值
    double averU = 0;
    for(int i = 0; i < 256; i++)
        averU += i * num[i];
    averU /= n*m;

    int W1 = 0, W2 = 0, U1 = 0, U2 = 0;
    for(int i = 0 ; i < 256; i++)
        W1 += num[i], U1 += i * num[i];

    double ansG = -0x3f3f3f3f;
    int ansT = 0;
    for(int i = 0; i < 256; i++)
    {
        double w1 = W1 / (n*m);
        double u1 = 0;
        if(W1) u1 = U1 / W1;

        double w2 = W2 / (n*m);
        double u2 = 0;
        if(W2) u2 = U2 / W2;

        double curG = w1*D(u1 - averU)+w2*D(u2-averU);
        if(curG > ansG + 1e-8)
        {
            ansG = curG;
            ansT = i;
        }
        //更新W1, W2, U1, U2
        W1 -= num[i];
        W2 += num[i];
        U1 -= i*num[i];
        U2 += i*num[i];
    }

    for(int i = 0; i < n; i++)
        for(int j = 0; j < m; j++)
        {
            if(gray[i*m+j] >= ansT) bit[i*m+j] = 1;
            else bit[i*m+j] = 0;
        }

    delete []num;
}
void formatToReal(unsigned char* real, unsigned char *ImageData, int n, int m, int biBitCount)
{
    if(biBitCount > 8) return ;
    int num = 8/biBitCount;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            int RGBs = ImageData[i*m+j];
            int cover = (1 << biBitCount) - 1;
            for(int k = num-1; k >= 0; k--)
            {
                real[i*m*num+j*num+k] = RGBs & cover;
                RGBs >>= biBitCount;
            }
        }
    }
}

void realToFormat(unsigned char* format, unsigned char* realData, int n, int m, int biBitCount)
{
    if(biBitCount > 8) return ;
    int lineByte = calLineByte(m, biBitCount);
    int num = 8/biBitCount;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j += num)
        {
            int save = 0;
            for(int k = 0; k < num; k++)
            {
                save <<= biBitCount;
                //if there are some data, then save it
                if(j + k < m)
                {
                    save += realData[i*m+j+k];
                }
            }
            format[i*lineByte+j/num] = save;
//            format[i] = save;
        }
    }
}
//-----------------------------------------------------------------------------------------
//给定一个图像位图数据、宽、高、颜色表指针及每像素所占的位数等信息,将其写到指定文件中
bool saveBmp(char *bmpName, unsigned char *imgBuf, int width, int height, int biBitCount, RGBQUAD *pColorTable)
{
    if(!imgBuf) return 0;

    //颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0
    int colorTablesize=0;

    if(biBitCount==8)      colorTablesize=1024;
    else if(biBitCount==4) colorTablesize=16*4;
    else if(biBitCount==1) colorTablesize=2*4;

    //待存储图像数据每行字节数为4的倍数
    int lineByte=((width * biBitCount+7)/8+3)/4*4;
    cout<<"biSizeImage -> "<<lineByte*height<<endl;
    cout<<"width = "<<width<<" height = "<<height<<" biBitCount = "<<biBitCount<<endl;

    FILE *fp=fopen(bmpName, "wb");//以二进制写的方式打开文件
    if(fp==0) return 0;

    //填写文件头信息
    BITMAPFILEHEADER fileHead;
    fileHead.bfType = 0x4D42;//bmp类型

    //bfSize是图像文件4个组成部分之和
    fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte*height;
    fileHead.bfReserved1 = fileHead.bfReserved2 = 0;

    //bfOffBits是图像文件前3个部分所需空间之和
    fileHead.bfOffBits=54+colorTablesize;

    //写文件头进文件
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER),1, fp);

    //填写信息头信息
    BITMAPINFOHEADER bmphead;
    bmphead.biBitCount=biBitCount;
    bmphead.biClrImportant=0;
    bmphead.biClrUsed=0;
    bmphead.biCompression=0;
    bmphead.biHeight=height;
    bmphead.biWidth=width;
    bmphead.biSizeImage=lineByte*height;
    bmphead.biPlanes=1;
    bmphead.biSize=40;
    bmphead.biXPelsPerMeter=0;
    bmphead.biYPelsPerMeter=0;

    //写位图信息头进内存
    fwrite(&bmphead, sizeof(BITMAPINFOHEADER),1, fp);

    //如果灰度图像，有颜色表，写入文件
    if(bmphead.biClrUsed != 0) fwrite(pColorTable, sizeof(RGBQUAD), bmphead.biClrUsed, fp);
    else fwrite(pColorTable, sizeof(RGBQUAD), 1<<biBitCount, fp);

    //写位图数据进文件
    fwrite(imgBuf, 1, lineByte*height, fp);

    //关闭文件
    fclose(fp);
    return 1;

}

void initColorTable(RGBQUAD* pColorTable)
{
    pColorTable[0] = RGBQUAD{255, 255, 255, 0};
    pColorTable[1] = RGBQUAD{0, 0, 0, 0};
}
void make_bmp(int width, int height, int biBitCount, char* outputPath)
{
    int lineByte=((width * biBitCount+7)/8+3)/4*4;

    unsigned char* pBmpBuf = new unsigned char[height*lineByte];

    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < lineByte; j++)  pBmpBuf[i*lineByte+j] = 255 * (i&1);
        for(int j = 0; j < lineByte/4; j++) pBmpBuf[i*lineByte+j] = 255*(!(i&1));
    }

//    if(pColorTable) delete[] pColorTable;
//    pColorTable = new RGBQUAD[2];
//    initColorTable(pColorTable);

    //存储图片
    saveBmp(outputPath, pBmpBuf, width, height, biBitCount, pColorTable);

    //回收内存
    if(pBmpBuf) delete []pBmpBuf;
    if(pColorTable) delete []pColorTable;
}


#endif // IMAGE_H_INCLUDED
