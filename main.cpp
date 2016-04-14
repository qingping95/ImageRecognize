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
//����ʱ��Ҫ�ͷ�pColorTable���ָ����ڴ�
//
#include "IMAGE.h"
//
#include "IMAGE_DSU.h"
using namespace std;
#define CHECK(x, y) ((x) >= 0 && (x) < n && (y) >= 0 && (y) < m)
#define ID(x, y) ((x)*m+(y))

//K=2 ʮ������
//K=4 ��������(default)
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
void dsuSave(ImageDsu Idsu, int biBitCount, int height, int width, char *output = NULL)
{
    //���output�ǿգ���ʾ�洢
    if(output == NULL) return ;

    int lineByte=((width * biBitCount+7)/8+3)/4*4;//�Ҷ�ͼ������ɫ������ɫ�����Ϊ256

    unsigned char* realData = new unsigned char[Idsu.height*Idsu.width];
    unsigned char* ImageData = new unsigned char[height*lineByte];
    //������������
    Idsu.exportAttr(realData);

    //ת����ImageData
    realToFormat(ImageData, realData, Idsu.height, Idsu.width, biBitCount);

    //��ʼ����ɫ��
    if(pColorTable) delete[] pColorTable;
    pColorTable = new RGBQUAD[2];
    initColorTable(pColorTable);

    saveBmp(output, ImageData, Idsu.width, Idsu.height, biBitCount, pColorTable);
    if(realData) delete []realData;
    if(ImageData) delete []ImageData;
}
int main()
{
//    make_bmp(31, 31, 1, "black-white.bmp");

    int height, width, biBitCount;
    unsigned char *ImageData; //ͼ������
    readBmp("test_shouxie.bmp", ImageData, width, height, biBitCount);
    int lineByte=calLineByte(width, biBitCount);//�Ҷ�ͼ������ɫ������ɫ�����Ϊ256

    int realWidth = lineByte*8/biBitCount;
    unsigned char* realData = new unsigned char[height*realWidth];

    //ȡ��ͼ�����Ϣ
    formatToReal(realData, ImageData, height, lineByte, biBitCount);

    ImageDsu Idsu(height, realWidth);

    //������ɫ���ݵ����鼯
    Idsu.importAttr(realData);

    //�ϲ�������
    runUnite(Idsu, height, realWidth, 4);

    //����
    Idsu.denoise(0, 1, 30);
    dsuSave(Idsu, biBitCount, height, width, "after_noise_now.bmp");

    //�����ڴ�
    if(pColorTable) delete []pColorTable;
    if(realData) delete []realData;
    if(ImageData) delete []ImageData;
    return 0;
}
