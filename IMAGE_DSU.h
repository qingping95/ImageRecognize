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
            pa[i] = i, color[i] = 0, num[i] = 1;
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
