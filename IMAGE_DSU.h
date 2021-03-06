#ifndef IMAGE_DSU_H_INCLUDED
#define IMAGE_DSU_H_INCLUDED

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
/*
* n     -> 像素数
* pa    -> 分量的id
* num   -> 分量包含的像素数
* attr  -> 分量的颜色
* BACK  -> 背景色
* FORE  -> 前景色
*/
class ImageDsu
{
public:
    int n;
    int width, height;
    int *pa;
    int *num;
    int *color;
    int *Left, *Right, *Up, *Bottom;
    int BACK;
    int FORE;
    ImageDsu(){}
    ImageDsu(int height, int width)
    {
        this -> height = height;
        this -> width = width;
        this -> n = height * width;
        pa = new int[n];
        num = new int[n];
        color = new int[n];
        Left = new int[n];
        Right = new int[n];
        Up = new int[n];
        Bottom = new int[n];
        for(int i = 0; i < n; i++)
        {
            pa[i] = i, color[i] = 0, num[i] = 1;
            int x = i/width, y = i%width;
            Left[i] = Right[i] = y;
            Up[i] = Bottom[i] = x;
        }
    }
    ~ImageDsu()
    {
        if(pa)      delete []pa;
        if(num)     delete []num;
        if(color)   delete []color;
        if(Left)    delete []Left;
        if(Right)   delete []Right;
        if(Up)      delete []Up;
        if(Bottom)  delete []Bottom;
    }
    int getHeight(int idx)
    {
        idx = find(idx);
        return Up[idx] - Bottom[idx]+1;
    }
    int getWidth(int idx)
    {
        idx = find(idx);
        return Right[idx] - Left[idx]+1;
    }
    void importAttr(unsigned char* data)
    {
        for(int i = 0; i < n; i++)
            color[i] = data[i];
    }
    void exportAttr(unsigned char* data)
    {
        for(int i = 0; i < n; i++){
            int idx = find(i);
            //if(idx == 267732) cout<<idx<<endl;
            data[i] = color[idx];
        }
    }
    void exportCom(int u, unsigned char* &data, int &th, int &tw)
    {
//        if(data)
//            delete []data;
        u = find(u);
        th = Up[u] - Bottom[u]+1; tw = Right[u] - Left[u]+1;
//        DEBUG(Bottom[u]);
//        DEBUG(Up[u]);
//        DEBUG(Left[u]);
//        DEBUG(Right[u]);
        data = new unsigned char[th * tw];
        for(int i = 0; i < th; i++)
            for(int j = 0; j < tw; j++)
            {
                data[i*tw+j] = color[find((i+Bottom[u])*width+(j+Left[u]))];
            }
    }

    bool isInCom(int x, int y, int v)
    {
        if(Left[v] <= x && x <= Right[v] && Bottom[v] <= y && y <= Up[v]) return true;
        return false;
    }
    double Edist(int x1, int y1, int x2, int y2)
    {
        return sqrt((double)(x1 - x2)*(x1 - x2)+(y1 - y2)*(y1 - y2));
    }
    #define MAX4(a, b, c, d) max(a, max(b, max(c, d)))
    #define MIN4(a, b, c, d) min(a, min(b, min(c, d)))
    bool check(int u, int v, int mah, int maw, bool use)
    {
        u = find(u), v = find(v);
        if(u == v) return true;

        //check two component whether intersected
        if(isInCom(Left[u], Bottom[u], v) || isInCom(Left[u], Up[u], v) ||isInCom(Right[u], Bottom[u], v) || isInCom(Right[u], Up[u], v))
            return true;
        if(isInCom(Left[v], Bottom[v], u) || isInCom(Left[v], Up[v], u) ||isInCom(Right[v], Bottom[v], u) || isInCom(Right[v], Up[v], u))
            return true;

        //check the distance between two component
        int n = 2, nn = n+1;
        double threshold = MAX4(((double)Right[u]-Left[u])*n/nn, ((double)Up[u]-Bottom[u])*n/nn, ((double)Right[v]-Left[v])*n/nn, ((double)Up[v]-Bottom[v])*n/nn);
        if(Edist((Left[u]+Right[u])/2, (Up[u]+Bottom[u])/2, (Left[v]+Right[v])/2, (Up[v]+Bottom[v])/2) < threshold)
            return true;

//        //check the size of word
        int ll = min(Left[u], Left[v]);
        int rr = max(Right[u], Right[v]);
        int uu = max(Up[v], Up[u]);
        int bb = min(Bottom[v], Bottom[u]);

        /*--------------------------------------this is a cut line---------------------------------------------------------------*/
        //there are code of unite all disconnected and correlative component, run it after unite all connective component
        if(!use) return false;
        //Vertical merger
        if(uu - bb + 1 <= mah && rr - ll + 1 <= Right[u]-Left[u]+Right[v]-Left[v])
            return true;
//
//        int base = Up[u]-Bottom[u]+Up[v]-Bottom[v];
        if(rr - ll <= maw && uu - bb <= mah)
            return true;
        //Horizontal merger
        int base = max(Up[u]-Bottom[u], Up[v]-Bottom[v]);
        double error = base*1.0/5;
        if(rr - ll + 1 <= maw && uu - bb <= base+error)
            return true;
        //int base = max(Up[u]-Bottom[u], Up[v]-Bottom[v]);
        //double error = base*1.0/5;
        return false;
    }
    int find(int x)
    {
        return pa[x] == x ? x : pa[x] = find(pa[x]);
    }
    bool unite(int u, int v)
    {
        u = find(u), v = find(v);
        if(u == v || color[u] != color[v]) return false;
        pa[u] = v;
        num[v] += num[u];
        Left[v] = min(Left[u], Left[v]);
        Right[v] = max(Right[u], Right[v]);
        Up[v] = max(Up[v], Up[u]);
        Bottom[v] = min(Bottom[v], Bottom[u]);
        return true;
    }
    void denoise(int BACK, int FORE, int WIDTH, int threshold)
    {
        bool *vis = new bool[n];
        memset(vis, 0, sizeof(bool)*n);
        for(int i = 0; i < n; i++)
        {
            int u = find(i);
            if(vis[u]) continue;
            vis[u] = 1;
            if(color[u] == BACK) continue;
            if(num[u] < threshold) color[u] = BACK;
            if(Right[u] >= WIDTH) color[u] = BACK;
            if(Right[u] - Left[u] >= width / 3) color[u] = BACK;
            if(Up[u] - Bottom[u] >= height / 3) color[u] = BACK;
        }
        delete []vis;
    }
    void printCom(int idx)
    {
        printf("\n");
        idx = find(idx);
        for(int i = Up[idx]; i >= Bottom[idx]; i--)
        {
            for(int j = Left[idx]; j <= Right[idx]; j++)
            {
                printf("%c", ".#"[color[i*width+j]]);
            }
            printf("\n");
        }
        printf("\n");
    }
};

#endif // IMAGE_DSU_H_INCLUDED
