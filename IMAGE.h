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

RGBQUAD *pColorTable;//��ɫ��ָ��

int calLineByte(int width, int biBitCount)
{
    return ((width * biBitCount+7)/8+3)/4*4;
}
//-------------------------------------------------------------------------------------------
//��ͼ���λͼ���ݡ����ߡ���ɫ��ÿ����λ�������ݽ��ڴ棬�������Ӧ��ȫ�ֱ�����
bool readBmp(char *bmpName, unsigned char* &ImageData, int& bmpWidth, int& bmpHeight, int& biBitCount, int &BACK, int &FORE)
{
    FILE *fp=fopen(bmpName,"rb");//�����ƶ���ʽ��ָ����ͼ���ļ�

    if(fp==0) return 0;

    //��ȡλͼ�ļ�ͷ�ṹBITMAPFILEHEADER
    fseek(fp, sizeof(BITMAPFILEHEADER), 0);
    //fread(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);

    //BITMAPINFOHEADER head;
    BITMAPINFOHEADER bmphead;

    fread(&bmphead, sizeof(BITMAPINFOHEADER), 1,fp); //��ȡͼ����ߡ�ÿ������ռλ������Ϣ
    bmpWidth = bmphead.biWidth;
    bmpHeight = bmphead.biHeight;
    biBitCount = bmphead.biBitCount;//�������������ͼ��ÿ��������ռ���ֽ�����������4�ı�����
//    int biClrUsed = bmphead.biClrUsed;

    //���ͼ�����Ϣ

    cout<<"width = "<<bmpWidth<<" height = "<<bmpHeight<<" biBitCount = "<<biBitCount<<endl;
    DEBUG(bmphead.biSizeImage);
    DEBUG(bmphead.biCompression);
    int lineByte=((bmpWidth * biBitCount+7)/8+3)/4*4;//�Ҷ�ͼ������ɫ������ɫ�����Ϊ256
    if(bmphead.biClrUsed != 0)
    {
        //������ɫ������Ҫ�Ŀռ䣬����ɫ����ڴ�
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
    //����λͼ��������Ҫ�Ŀռ䣬��λͼ���ݽ��ڴ�
    ImageData = new unsigned char[bmpHeight*lineByte];
    fread(ImageData, 1, bmpHeight*lineByte, fp);

    fclose(fp);//�ر��ļ�
    return 1;//��ȡ�ļ��ɹ�
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
    biBitCount = bmphead.biBitCount;//�������������ͼ��ÿ��������ռ���ֽ�����������4�ı�����

    //���ͼ�����Ϣ
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

    //����ƽ���Ҷ�ֵ
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
        //����W1, W2, U1, U2
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
//����һ��ͼ��λͼ���ݡ����ߡ���ɫ��ָ�뼰ÿ������ռ��λ������Ϣ,����д��ָ���ļ���
bool saveBmp(char *bmpName, unsigned char *imgBuf, int width, int height, int biBitCount, RGBQUAD *pColorTable)
{
    if(!imgBuf) return 0;

    //��ɫ���С�����ֽ�Ϊ��λ���Ҷ�ͼ����ɫ��Ϊ1024�ֽڣ���ɫͼ����ɫ���СΪ0
    int colorTablesize=0;

    if(biBitCount==8)      colorTablesize=1024;
    else if(biBitCount==4) colorTablesize=16*4;
    else if(biBitCount==1) colorTablesize=2*4;

    //���洢ͼ������ÿ���ֽ���Ϊ4�ı���
    int lineByte=((width * biBitCount+7)/8+3)/4*4;
    cout<<"biSizeImage -> "<<lineByte*height<<endl;
    cout<<"width = "<<width<<" height = "<<height<<" biBitCount = "<<biBitCount<<endl;

    FILE *fp=fopen(bmpName, "wb");//�Զ�����д�ķ�ʽ���ļ�
    if(fp==0) return 0;

    //��д�ļ�ͷ��Ϣ
    BITMAPFILEHEADER fileHead;
    fileHead.bfType = 0x4D42;//bmp����

    //bfSize��ͼ���ļ�4����ɲ���֮��
    fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte*height;
    fileHead.bfReserved1 = fileHead.bfReserved2 = 0;

    //bfOffBits��ͼ���ļ�ǰ3����������ռ�֮��
    fileHead.bfOffBits=54+colorTablesize;

    //д�ļ�ͷ���ļ�
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER),1, fp);

    //��д��Ϣͷ��Ϣ
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

    //дλͼ��Ϣͷ���ڴ�
    fwrite(&bmphead, sizeof(BITMAPINFOHEADER),1, fp);

    //����Ҷ�ͼ������ɫ��д���ļ�
    if(bmphead.biClrUsed != 0) fwrite(pColorTable, sizeof(RGBQUAD), bmphead.biClrUsed, fp);
    else fwrite(pColorTable, sizeof(RGBQUAD), 1<<biBitCount, fp);

    //дλͼ���ݽ��ļ�
    fwrite(imgBuf, 1, lineByte*height, fp);

    //�ر��ļ�
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

    //�洢ͼƬ
    saveBmp(outputPath, pBmpBuf, width, height, biBitCount, pColorTable);

    //�����ڴ�
    if(pBmpBuf) delete []pBmpBuf;
    if(pColorTable) delete []pColorTable;
}


#endif // IMAGE_H_INCLUDED
