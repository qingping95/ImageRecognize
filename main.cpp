#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
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

vector<int> getForeground(ImageDsu Idsu, int Fore)
{
    vector<int> v;
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
bool uniteCom(ImageDsu Idsu, vector<int> v)
{
    bool isUpdate = false;
    for(int i = 0; i < v.size(); i++)
    {
        for(int j = i+1; j < v.size(); j++)
        {
            int u = find(v[i]), v = find(v[j]);
            if(u == v) continue;
            if(Idsu.check(u, v))
            {
                if(Idsu.unite(u, v)) isUpdate = true;
            }
        }
    }
    return isUpdate;
}
int main()
{
//    make_bmp(31, 31, 1, "black-white.bmp");
    const int BACK = 0, FORE = 1;
    int height, width, biBitCount;
    char input[] = "test_shouxie.bmp";
    char output[] = "after_denoise4.bmp";

    unsigned char *ImageData; //ͼ������
    readBmp(input, ImageData, width, height, biBitCount);
    int lineByte=calLineByte(width, biBitCount);//�Ҷ�ͼ������ɫ������ɫ�����Ϊ256

    int realWidth = lineByte*8/biBitCount;
    unsigned char* realData = new unsigned char[height*realWidth];

    formatToReal(realData, ImageData, height, lineByte, biBitCount);

    ImageDsu Idsu(height, realWidth);

    //������ɫ���ݵ����鼯
    Idsu.importAttr(realData);

    //�ϲ�������
    runUnite(Idsu, height, realWidth, 4);

    //����
    Idsu.denoise(BACK, FORE, 30);

    //���ϲ�ƫ�Բ���
    vector<int> FG = getForeground(Idsu, FORE);
    int times = 0;
    while(uniteCom(Idsu, FG)){
        for(int i = 0; i < FG.size(); i++)
        {
            FG[i] = Idsu.find(FG[i]);
        }
        sort(FG.begin(), FG.end());
        FG.erase(unique(FG.begin(), FG.end()), FG.end());
        times++;
    }
    cout<<"��������Ϊ��"<<times<<endl;

    //save each word
    for(int i = 0; i < FG.size(); i++)
    {

    }

    //�洢ͼ��
    //������������
    Idsu.exportAttr(realData);

    //ת����ImageData
    realToFormat(ImageData, realData, height, realWidth, biBitCount);

    //��ʼ����ɫ��
    if(pColorTable) delete[] pColorTable;
    pColorTable = new RGBQUAD[2];
    initColorTable(pColorTable);

    saveBmp(output, ImageData, width, height, biBitCount, pColorTable);

    //�����ڴ�
    if(pColorTable) delete []pColorTable;
    if(realData) delete []realData;
    if(ImageData) delete []ImageData;
    //Denoise("test_shouxie.bmp", "after_denoise4.bmp");
    return 0;
}
