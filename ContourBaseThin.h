#ifndef CONTOURBASETHIN_H_INCLUDED
#define CONTOURBASETHIN_H_INCLUDED
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

typedef pair<int, int> PII;
int dir[][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 1}, {1, 1}, {1, -1}, {-1, -1}};

struct Point
{
    double x,y;
    Point(double x = 0, double y = 0):x(x),y(y){}
    void read()
    {
        scanf("%lf%lf", &x, &y);
    }
};
const double eps = 1e-10;
const double PI = acos(-1.0);
typedef Point Vector;
int dcmp(double x){if(fabs(x) < eps) return 0;else return x<0?-1:1;}
Vector operator+(Vector A,Vector B){return Vector(A.x+B.x, A.y+B.y);}
Vector operator-(Point A,Point B){return Vector(A.x-B.x, A.y-B.y);}
Vector operator*(Vector A, double p){return Vector(A.x*p, A.y*p);}
Vector operator/(Vector A, double p){return Vector(A.x/p, A.y/p);}
bool operator<(const Point& a, const Point& b){return a.x<b.x || (a.x == b.x && a.y < b.y);}
bool operator==(const Point& a, const Point& b){return dcmp(a.x-b.x) == 0 && dcmp(a.y-b.y) == 0;}
double angle(Vector A){return atan2(A.y,A.x);}//返回A向量的极角
double Dot(Vector A, Vector B){return A.x*B.x+A.y*B.y;}
double Length(Vector A){return sqrt(Dot(A,A));}
double Angle(Vector A,Vector B)
{
    double d=Dot(A,B)/Length(A)/Length(B);
    if(dcmp(d-1)==0) return 0;
    if(dcmp(d+1)==0) return PI;
    return acos(d);
}//A到B的逆时针转的角
class ContourBaseThin{
public:
    int *isContour;
    int *color;
    int *pcolor;
    int *medial;
    double *theta;
    double *dTheta;
    bool *vis;
    int width, height, n;
    int penWidth;
    int BACK,FORE;
    vector<int > Contour;
    vector<PII > contourSeg;
    ContourBaseThin(){}
    ContourBaseThin(unsigned char *data, int h, int w, int b, int f)
    {
        BACK = b, FORE = f;
        width = w, height = h;
        n = h*w;
        isContour = new int[h*w];
        theta = new double[h*w];
        dTheta = new double[h*w];
        pcolor = new int[h*w];
        medial = new int[h*w];
        color = new int[h*w];
        vis = new bool[h*w];
        for(int i = 0; i < n; i++)
            color[i] = (data[i] == FORE);
        //black(FORE) is 1, white(BACK) is 0;
    }
    ~ContourBaseThin()
    {
        delete []vis;
        delete []pcolor;
        delete []theta;
        delete []dTheta;
        delete []medial;
        delete []isContour;
        delete []color;
    }
    int sign(double x)
    {
        double eps = 1e-6;
        return (x > eps) - (x < -eps);
    }

    void dfs(int x, int y)
    {
        //DEBUG(x*width+y);
        Contour.push_back(x*width+y);
        isContour[x*width+y] = Contour.size();
        for(int i = 0; i < 8; i++)
        {
            int cx = x + dir[i][0];
            int cy = y + dir[i][1];
            if(!CHECKWH(cx, cy)) continue;
            if(vis[cx*width+cy]) continue;
            if(isContour[cx*width+cy] == 0) continue;
            vis[cx*width+cy] = 1;
            dfs(cx, cy);
        }
    }
    /*
    *   第一步
    *       得到图像数据的边界。
    *   isPrint: 是否输出边界图像
    */
    void getContourVector(bool isPrint)
    {
        memset(isContour, 0, sizeof(bool)*height*width);
        memset(vis, 0, sizeof(bool)*height*width);
        memset(pcolor, 0, sizeof(int)*height*width);
        Contour.clear();
        int num = 0;
        for(int i = 0; i < n; i++)
        {
            if(color[i] == 0) continue;
            int x = i / width, y = i % width;
            int zeroNum = 0;
            for(int j = 0; j < 8; j++)
            {
                int cx = x + dir[j][0];
                int cy = y + dir[j][1];
                if(!CHECKWH(cx, cy)) continue;
                if(color[cx*width+cy] == 0) zeroNum++;
            }
            //if the number of background pixel that joint this pixel more than 0, this pixel was considered a contour pixel
            if(zeroNum > 0) pcolor[i] = isContour[i] = 1, num++;
        }
        //cerr << "zeroNum ->" << num << endl;
        for(int i = height-1; i >= 0; i--)
            for(int j = 0; j < width; j++)
                if(isContour[i*width+j] > 0 && vis[i*width+j] == 0)
                    vis[i*width+j] = 1, dfs(i, j);

        //cerr << "zeroNum ->" << num << endl;
        DEBUG(num);
        DEBUG(Contour.size());
        if(isPrint)
        {
            printf("this is Contour image:\n");
            for(int i = height-1; i >= 0; i--, printf("\n"))
                for(int j = 0; j < width; j++)
//                    printf("%c", ".#"[pcolor[i*width+j]]);
                    printf("%c", ".#"[isContour[i*width+j] > 0]);
            printf("\n");
        }
    }
    bool getSegment(bool isPrint)
    {
        if(Contour.size() == 0) return false;
        contourSeg.clear();
        vector<PII> segment;
        const double PI = acos(-1.0);
        segment.clear();
        int sx = Contour[0] / width;
        int sy = Contour[0] % width;
        cerr << sx <<" : "<< sy << endl;
        for(int i = 0; i < Contour.size(); i ++)
        {
            int cx = Contour[i] / width;
            int cy = Contour[i] % width;
            if(cx != sx) theta[i] = atan2((cy - sy)*1.0,  (cx - sx));
            else theta[i] = PI/2;
            if(i) dTheta[i] = theta[i] - theta[i-1];
        }
        int st = 0;
        for(int i = 2; i < Contour.size(); i++)
        {
            if(sign(dTheta[i]) != sign(dTheta[i-1]))
            {
                segment.push_back(PII(st, i-1));
                //cerr<<st<<" -> "<<i-1<<endl;
                st = i;
            }
        }
        if(st < Contour.size()-1) segment.push_back(PII(st, Contour.size()-1));
        for(int i = 0; i < segment.size(); i++)
        {
            for(int j = segment[i].first; j <= segment[i].second; j++)
            {
                pcolor[Contour[j]] = i%3+1;
            }
        }
        for(int i = 0; i < n; i++)
            if(pcolor[i] == 0) pcolor[i] = 255;
            else pcolor[i]--;
//        for(int i = height-1; i >= 0; i--, printf("\n"))
//            for(int j = 0; j < width; j++)
//            {
//                if(pcolor[i*width+j] == 0) printf(".");
//                else printf("%d", pcolor[i*width+j]);
//            }
        st = 0;
        for(int i = 0; i < segment.size(); i++)
        {
            double diff = theta[segment[i].second] - theta[segment[st].first];
            if(diff > (10.0/180*PI) || diff < (-10.0/180*PI)) {
                if(i > st){
                    contourSeg.push_back(PII(segment[st].first, segment[i-1].second));
                    st = i;
                    i--;
                }else{
                    for(int j = segment[i].first+1; j <= segment[i].second; j++)
                    {
                        diff = theta[j] - theta[segment[st].first];
                        if(diff > (10.0/180*PI) || diff < (-10.0/180*PI)){
                            contourSeg.push_back(PII(segment[st].first, j-1));
                            segment[i].first = j;
                            st = i;
                            i--;
                            break;
                        }
                    }
                }
            }
        }
        if(segment[st].first < segment[segment.size()-1].second)
            contourSeg.push_back(PII(segment[st].first, segment[segment.size()-1].second));
        if(isPrint)
        {
            memset(pcolor, 0, sizeof(int)*height*width);
            for(int i = 0; i < contourSeg.size(); i++)
                for(int j = contourSeg[i].first; j <= contourSeg[i].second; j++)
                    pcolor[Contour[j]] = i % 7 + 1;

            for(int i = height-1; i >= 0; i--, printf("\n"))
                for(int j = 0; j < width; j++)
                {
                    if(pcolor[i*width+j] == 0) printf(".");
                    else printf("%d", pcolor[i*width+j]);
                }
        }
        return true;
    }

    void get256Color()
    {
        memset(pcolor, 0, sizeof(int)*height*width);
        int idx = 0;
        for(int i = 0; i < contourSeg.size(); i++, idx ++)
            for(int j = contourSeg[i].first; j <= contourSeg[i].second; j++)
                pcolor[Contour[j]] = idx%3+1;
        for(int i = 0; i < n; i++)
            if(pcolor[i] == 0) pcolor[i] = 255;
            else pcolor[i]--;
    }
    double getAvgD(int x)
    {
        int num = 0;
        double res = 0;
        int sx = Contour[x] / width;
        int sy = Contour[x] % width;
        for(int i = max(0, x - 5); i < min((int)Contour.size(), x+5); i++, num++)
        {
            int cx = Contour[i] / width;
            int cy = Contour[i] % width;
            res += atan2(1.0*(cy - sy), cx - sx);
        }
        return res / num;
    }
    void getMedialAxis(bool isPrint)
    {
        memset(vis, 0, sizeof(bool )*height*width);
        memset(medial, 0, sizeof(int)*height*width);
        for(int i = 0; i < contourSeg.size(); i ++)
            for(int j = contourSeg[i].first; j <= contourSeg[i].second; j++)
            {
                pcolor[Contour[j]] = i+1;
            }
//        for(int i = 0; i < contourSeg.size(); i ++)
//            for(int j = contourSeg[i].first; j <= contourSeg[i].second; j++)
        for(int i = 0; i < Contour.size(); i++)
        {

            if(vis[i]) continue;
            vis[i] = 1;

            double stAvg = getAvgD(i);
            double dirK = 1/stAvg;
//                int oriPos = contourSeg[j];
            int sx = Contour[i] / width;
            int sy = Contour[i] % width;
            int add = 1;
            int cx = sx+add;
            double cy = dirK + sy;
            if(color[cx * width + (int)(cy+0.5)] == 0) {
                add = -add;
                dirK = -dirK;
                cx = sx + add;
                cy = sy + dirK;
            }

            while(cx >= 0 && cx < width && sign(cy - height) < 0 && sign(cy) >= 0)
            {
                int ry = cy+0.5;
                if(ry < height && ry >= 0)
                {
                    if(isContour[cx*width+ry] > 0)
                    {
                        double peAvg = getAvgD(isContour[cx*width+ry] - 1);
                        if(abs(Angle(Vector(1, peAvg), Vector(1, stAvg))) < 10.0/180*PI
                            && sign(Length(Vector(sx, sy) - Vector(cx, ry)) - 1.4*penWidth) < 0)
                        {
                            int mx = (sx+cx)*1.0/2+0.5;
                            int my = (sy+cy)*1.0/2+0.5;
                            medial[mx*width+my] = 1;
                        }
                        break;
                    }
                }
                cx += add;
                cy += dirK;
            }
        }
        if(isPrint)
        {
            printf("Medial axis image is :\n");
            for(int i = 0; i < n; i++)
                if(medial[i] || isContour[i]) pcolor[i] = 1;
                else pcolor[i] = 0;
            for(int i = height-1; i >= 0; i--, printf("\n"))
                for(int j = 0; j < width; j++)
                    printf("%c", ".#"[pcolor[i*width+j]]);
        }
    }
};

#endif // CONTOURBASETHIN_H_INCLUDED
