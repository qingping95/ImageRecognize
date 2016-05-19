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
#include <queue>
//
//结束时需要释放pColorTable这个指针的内存
//
#include "IMAGE.h"
#include "OCR.h"
#include "Thinning.h"
#include "ContourBaseThin.h"
//
#include "IMAGE_DSU.h"
using namespace std;
#define CHECK(x, y) ((x) >= 0 && (x) < n && (y) >= 0 && (y) < m)

#define ID(x, y) ((x)*m+(y))
#define DEBUG(x) cout<<#x<<" -> "<<x<<endl

//print current image
void printImage(int *data, int n, int m);

//K=2 十字相连
//K=4 星型相连(default)
//合并相邻的所有像素
void runUnite(ImageDsu &dsu, int n, int m, int K = 4);

//得到所有的前景分量
vector<int> getForeground(ImageDsu &Idsu, int Fore, int &mah, int &maw);

// 对24位真彩图像做二值化处理
void binaryzation();

//合并相近的分量
bool uniteCom(ImageDsu &Idsu, vector<int> v, int &mah, int &maw, bool use);

//OCR识别接口
bool runOCR();

void cutImage();

//Zhang Algorithm
int runZhang(unsigned char *data, int height, int width, int b, int f, bool isSave);


//Self Algorithm
void selfThinningDriver(char *input);
void selfThinning(int* data, int n, int m, int d);

//Contour-Based Thinning
void ContourDriver(char *input);

int main()
{
    //runOCR
    //if(runOCR()) return 0;

    //run binaryzation()
    //binaryzation();

    //run cutImage()
    //cutImage();
//    selfThinningDriver("GB1000_R.bmp");
    ContourDriver("bit-a.bmp");
    return 0;
}
typedef pair<int, int> PII;
void ContourDriver(char *input)
{
    int height, width, biBitCount;
    unsigned char *ImageData; //图像数据
    int b, f;
    readBmp(input, ImageData, width, height, biBitCount, b, f);
    int lineByte = calLineByte(width, 1);
    unsigned char *data = new unsigned char[height*width];
    formatToReal(data, ImageData, height, width, 1);

    ContourBaseThin Thin(data, height, width, b, f);
    freopen("ContourBaseResult.txt", "w", stdout);
    Thin.getContourVector(true);
    Thin.getSegment(true);

    delete []pColorTable;
    delete []ImageData;
    delete []data;
}
void selfThinningDriver(char *input)
{
    int height, width, biBitCount;
    unsigned char *ImageData; //图像数据
    int b, f;
    readBmp(input, ImageData, width, height, biBitCount, b, f);
    int lineByte = calLineByte(width, 1);
    unsigned char *data = new unsigned char[height*width];
    formatToReal(data, ImageData, height, width, 1);

//    realToFormat(ImageData, data, height, width, 1);
//    saveBmp("save_test.bmp", ImageData, width, height, 1, pColorTable);
//    return ;

    int d = runZhang(data, height, width, b, f, 1);
    DEBUG(d);
    int *BW = new int[height*width];
    for(int i = 0; i < height*width; i++)
    {
        BW[i] = (data[i] == f);
    }

    selfThinning(BW, height, width, d);

    delete []BW;
    delete []pColorTable;
    delete []ImageData;
    delete []data;
}
void selfThinning(int* data, int n, int m, int d)
{
    freopen("SelfResult.txt", "w", stdout);
    printf("initial :\n");
    printImage(data, n, m);
//    int LOW_LIMIT = 5;
    int LOW_LIMIT = d/2-5;
    cerr<<"LOW_LIMIT -> "<<LOW_LIMIT<<endl;
    priority_queue<PII, vector<PII >, greater<PII > > que;

    ///int -> double
    int *num = new int[n*m];
    bool *vis = new bool[n*m];
    memset(vis, 0, sizeof(bool)*n*m);
    memset(num, 0x3f, sizeof(int)*n*m);

    int dir[][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 1}, {1, 1}, {1, -1}, {-1, -1}};
    for(int i = 0; i < n * m; i++)
    {
        if(data[i]) continue;
        num[i] = 0;
        que.push(PII(0, i));
    }

    while(!que.empty())
    {
        PII cur = que.top();que.pop();
        int cn = cur.first, idx = cur.second;
        if(vis[idx]) continue;
        vis[idx] = 1;
        int x = idx / m, y = idx % m;
        if(num[idx] < LOW_LIMIT && data[idx])  data[idx] = 0;
        for(int i = 0; i < 4; i++)
        {
            int cx = x + dir[i][0];
            int cy = y + dir[i][1];
            if(!CHECK(cx, cy)) continue;
            if(vis[cx*m+cy]) continue;
            int w = data[cx*m+cy];
            if(cn + w < num[cx*m+cy])
            {
                num[cx*m+cy] = cn+w;
                que.push(PII(cn+w, cx*m+cy));
            }
        }
    }
    for(int i = 0; i < n; i++)
        for(int j = 0; j < m; j++)
        {
            if(!data[i*m+j]) continue;
            for(int k = 0 ; k < 4; k++)
            {
                int cx = i + dir[k][0];
                int cy = j + dir[k][1];
                if(!CHECK(cx, cy)) continue;
                //if(num[i*m+j] > num[cx*m+cy]) data[cx*m+cy] = 0;
            }
        }
    printf("After Thinning:\n");
    for(int i = n-1; i >= 0; i--,printf("\n"))
        for(int j = 0; j < m; j++)
            if(num[i*m+j] == 0)
                printf(".");
            else
                printf("%x", num[i*m+j]);
    printf("After Thinning:\n");
    printImage(data, n, m);
    fclose(stdout);
    freopen("CON", "w", stdout);

    delete []vis;
    delete []num;
}

void printImage(int *data, int n, int m)
{
    for(int i = n-1; i>= 0; i --)
    {
        for(int j = 0; j < m; j++)
        {
            printf("%c", ".#"[data[i*m+j]]);
        }
        printf("\n");
    }
}

//利用Zhang算法得到平均的笔画长度。
int runZhang(unsigned char *data, int height, int width, int b, int f, bool isSave)
{
    freopen("ZhangResult.txt", "w", stdout);

    //import data to Thinning object
    Thinning solver(data, height, width, b, f);

    //get the origin pixel number of image
    int originN = solver.numOfFORE();
    printf("initial image:\n");
    solver.printToScreen(data);

    //run algorithm
    solver.runZhangThinning();
    printf("\nafter thinning：\n");
    solver.printToScreen(solver.color);
    if(isSave){
        unsigned char *sData = new unsigned char[height*width];
        int lineByte = calLineByte(width, 1);
        unsigned char *formatData = new unsigned char[height*lineByte];
        for(int i = 0; i < height; i ++)
            for(int j = 0; j < width; j++)
                sData[i*width+j] = solver.color[i*width+j] == 1 ? solver.FORE : solver.BACK;

        realToFormat(formatData, sData, height, width, 1);
        saveBmp("ZhangResult/GB1000_R.bmp", formatData, width, height, 1, pColorTable);
        delete []sData;
        delete []formatData;
    }
    //get the pixel number of image after thinning
    int finalN = solver.numOfFORE();

    fclose(stdout);
    freopen("CON", "w", stdout);
    DEBUG(originN);
    DEBUG(finalN);
    return originN / finalN;
}

void runUnite(ImageDsu &dsu, int n, int m, int K)
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
    char input[] = "self-test.bmp";
    char outputGray[] = "gray-self.bmp";
    char outputBit[] = "bit-self.bmp";

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
    saveBmp(outputGray, ImageData, width, height, 8, pColorTable);

    int lineByte2 = calLineByte(width, 1);
    unsigned char *bmp = new unsigned char[height*lineByte8];
    OTSU(bmp, gray, height, lineByte8);
    realToFormat(ImageData, bmp, height, lineByte8, 1);
    pColorTable[1] = pColorTable[255];
    saveBmp(outputBit, ImageData, width, height, 1, pColorTable);

    delete []pColorTable;
    delete []ImageData;
    delete []gray;
    delete []bmp;
}
void cutImage()
{
    //freopen("outputinfo.txt", "w", stdout);
    int BACK = 0, FORE = 1;
    int height, width, biBitCount;
    char input[] = "bit-self.bmp";
//    char input[] = "4855102.bmp";

    char output[] = "denoise-self-bit.bmp";
    char *outputPath = new char[111];
    strcpy(outputPath, "test/");

    unsigned char *ImageData; //图像数据
    readBmp(input, ImageData, width, height, biBitCount, BACK, FORE);
    int lineByte=calLineByte(width, biBitCount);//灰度图像有颜色表，且颜色表表项为256


    int realWidth = lineByte*8/biBitCount;
    unsigned char* realData = new unsigned char[height*width];

    formatToReal(realData, ImageData, height, width, biBitCount);



    ImageDsu Idsu(height, width);

    //导入颜色数据到并查集
    Idsu.importAttr(realData);

    //合并各分量
    runUnite(Idsu, height, realWidth, 4);

//    for(int i = 0; i < height; i++)
//    {
//        for(int j = 0; j < width; j++)
//        {
//            printf("%c", ".#"[Idsu.color[i*width+j] == FORE]);
//        }
//        printf("\n");
//    }
//    return ;
//    Idsu.printCom(idx);
//    return ;

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

// 输出存储文件信息，并存储单字图片
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
}
