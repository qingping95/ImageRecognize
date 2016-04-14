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
/*
* n     -> 像素数
* pa    -> 分量的id
* num   -> 分量包含的像素数
* attr  -> 分量的颜色
* BACK  -> 背景色
* FORE  -> 前景色
*/
struct ImageDsu
{
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
            Left[i] = Right[i] = x;
            Up[i] = Bottom[i] = y;
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
    bool isInCom(int x, int y, int v)
    {
        if(Left[v] <= x && x <= Right[v] && Bottom[v] <= y && y <= Up[v]) return true;
        return false;
    }
    double Edist(int x1, int y1, int x2, int y2)
    {
        return sqrt((x1 - x2)*(x1 - x2)+(y1 - y2)*(y1 - y2));
    }
    bool check(int u, int v)
    {
        if(u == v) return true;
        if(isInCom(Left[u], Bottom[u], v) || isInCom(Left[u], Up[u], v) ||isInCom(Right[u], Bottom[u], v) || isInCom(Right[u], Up[u], v)) return true;
        if(isInCom(Left[v], Bottom[v], u) || isInCom(Left[v], Up[v], u) ||isInCom(Right[v], Bottom[v], u) || isInCom(Right[v], Up[v], u)) return true;
        double threshold = max(((double)Left[u]+Right[u])/3, ((double)Up[u]+Bottom[u])/3, ((double)Left[v]+Right[v])/3, ((double)Up[v]+Bottom[v])/3);
        if(Edist((Left[u]+Right[u])/2, (Up[u]+Bottom[u])/2, (Left[v]+Right[v])/2, (Up[v]+Bottom[v])/2) < threshold) return true;;
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
        Right[v] = max(Left[u], Left[v]);
        Up[v] = max(Up[v], Up[u]);
        Bottom[v] = min(Bottom[v], Bottom[u]);
        return true;
    }
    void denoise(int BACK, int FORE, int threshold)
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
        }
        delete []vis;
    }
};

#endif // IMAGE_DSU_H_INCLUDED
