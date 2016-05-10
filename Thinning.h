#ifndef THINNING_H_INCLUDED
#define THINNING_H_INCLUDED

#include <assert.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "IMAGE_DSU.h"
#include "Image.h"

class Thinning
{
public:
    int *color;
    int width, height, n;
    int BACK,FORE;
    Thinning(){}
    Thinning(unsigned char *data, int h, int w, int b, int f){
        BACK = b, FORE = f;
        width = w, height = h;
        n = h*w;
        color = new int[h*w];
        for(int i = 0; i < n; i++)
            color[i] = (data[i]==FORE);
    }
    ~Thinning()
    {
        delete []color;
    }
    void printToScreen(int *pri)
    {
        for(int i = 0; i < height; i++)
        {
            for(int j = 0; j < width; j++)
            {
                printf("%c", ".#"[pri[i*width+j]]);
            }
            printf("\n");
        }
    }
    void printToScreen(unsigned char *pri)
    {
        for(int i = 0; i < height; i++)
        {
            for(int j = 0; j < width; j++)
            {
                printf("%c", ".#"[pri[i*width+j]]);
            }
            printf("\n");
        }
    }
    void runZhangThinning()
    {
        int *tc = new int[height*width];
        memcpy(tc, color, height*width);
        bool *vis = new bool[height*width];
        memset(vis, 0, sizeof(bool)*height*width);

        bool run = true;
        while(run)
        {
            run = false;
            //sprint one
            for(int i = 0; i < height; i++)
                for(int j = 0; j < width; j++)
                    vis[i*width+j] = judge(i, j, tc, 1);
            run |= clearRubbish(vis);

            //sprint two
            for(int i = 0; i < height; i++)
                for(int j = 0; j < width; j++)
                    vis[i*width+j] = judge(i, j, tc, 2);
            run |= clearRubbish(vis);
        }

        delete []vis;
        delete []tc;
    }
    bool clearRubbish(bool *vis)
    {
        bool update = false;
        for(int i = 0; i < n; i++)
            if(vis[i])
            {
                update = true;
                vis[i] = 0;
                color[i] = 0;
            }
        return update;
    }
    #define CHECK1(x, y) (x >= 0 && x < height && y >= 0 && y < width)
    bool judge(int x, int y, int *tc, int type)
    {
        int dir[][2] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}};
        bool vis[222];
        vis[65] = 1; vis[5] = 1; vis[20] = 1;  vis[80] = 1; vis[13] = 1; vis[22] = 1;
        vis[52] = 1; vis[133] = 1; vis[141] = 1; vis[54] = 1;
        int p[11];
        int n = 0, s = 0, prev = -1, b = 0;

        for(int i = 0; i < 8; i++)
        {
            int cx = x + dir[i][0];
            int cy = y + dir[i][1];
            int curv = -1;
            if(!CHECK1(cx, cy)){
                curv = 0;
            }else{
                curv = tc[cx*width+cy];
            }
            n += (curv == 1);
            s += (curv == 1 && prev == 0);
            b += (curv << i);
            prev = curv;
        }
        int cx = x - 1, cy = y;
        int curv = -1;
        if(!CHECK1(cx, cy)) curv = 0;
        else curv = tc[cx*width+cy];
        s += (curv == 1 && prev == 0);
        bool p2 = p[2]*p[4]*p[6];
        bool p4 = p[2]*p[4]*p[8];
        bool p6 = p[2]*p[6]*p[8];
        bool p8 = p[4]*p[6]*p[8];
        if(type == 1 && n >= 2 && n <= 6 && (s == 1 || vis[b]) && p2==0 && p8==0)
            return true;
        if(type == 2 && n >= 2 && n <= 6 && (s == 1 || vis[b]) && p4==0 && p6==0)
            return true;
        else
            return false;
    }
};

#endif // THINNING_H_INCLUDED
