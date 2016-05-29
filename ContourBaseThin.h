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
    int *segIdx;
    double *theta;
    double *dTheta;
    bool *vis;
    int width, height, n;
    int penWidth;
    int BACK,FORE;
    vector<int > Contour;
    vector<PII > contourSeg;
    vector<PII > segment;
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
        segIdx = new int[h*w];
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
        delete []segIdx;
    }
    int Sign(double x)
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
    double getK(Vector A)
    {
        double ang = angle(A);
        if(ang > PI/2) ang -= PI;
        if(ang < -PI/2) ang += PI;
        if(abs(ang - PI/2) < 1e-4) ang -= PI;
        return ang;
    }
    int getSegmentEndPoint(int stidx)
    {
//        DEBUG(stidx);
//        DEBUG(Contour.size());
        segment.clear();
        const double PI = acos(-1.0);
        int sx = Contour[stidx] / width;
        int sy = Contour[stidx] % width;
        for(int i = stidx+1; i < Contour.size(); i++)
        {
            int cx = Contour[i] / width;
            int cy = Contour[i] % width;
            theta[i] = getK(Vector(cx - sx, cy - sy));
            if(i>stidx+1) dTheta[i] = theta[i] - theta[i-1];
        }
        int st = stidx+1;
        for(int i = st+1; i < Contour.size(); i++)
        {
            if(Sign(dTheta[i]) != Sign(dTheta[i-1]))
            {
                segment.push_back(PII(st, i-1));
                st = i;
            }
        }
        if(st < Contour.size()-1) segment.push_back(PII(st, Contour.size()-1));
        for(int i = 0; i < segment.size(); i++)
        {
            double diff = theta[segment[i].second] - theta[segment[i].first];
            if(abs(diff) > 15.0/180*PI){
                if(i){
                    return segment[i-1].second+1;
                    //contourSeg.push_back(PII(stidx, segment[i-1].second));
                }else{
                    for(int j = segment[i].first; j <= segment[i].second; j++)
                    {
                        double curd = theta[j] - theta[segment[i].first];
                        if(abs(curd) > 15.0/180*PI){
                            return j;
                        }
                    }
                }
            }
        }
        return Contour.size();
    }
    bool getSegmentDriver(bool isPrint)
    {
        if(Contour.size() == 0) return false;
        contourSeg.clear();

        int st = 0;
        while(st < Contour.size())
        {
            int tmp = getSegmentEndPoint(st);
            contourSeg.push_back(PII(st, tmp));
            for(int i = st; i < tmp; i++)
            {
                int cx = Contour[i]/width;
                int cy = Contour[i]%width;
                segIdx[Contour[i]] = contourSeg.size()-1;
            }
            cerr<<st<<" -> "<<tmp<<endl;
            st = tmp;
        }
        if(isPrint)
        {
            memset(pcolor, 0, sizeof(int)*height*width);
            for(int i = 0; i < contourSeg.size(); i++)
                for(int j = contourSeg[i].first; j < contourSeg[i].second; j++)
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
            for(int j = contourSeg[i].first; j < contourSeg[i].second; j++)
                pcolor[Contour[j]] = idx%3+1;
        for(int i = 0; i < n; i++)
            if(pcolor[i] == 0) pcolor[i] = 255;
            else pcolor[i]--;
    }
    //return the angle of pixel position x
    double getAvgD(int x)
    {
        int num = 0;
        double res = 0;
        int sx = Contour[x] / width;
        int sy = Contour[x] % width;
        for(int i = max(0, x - 5); i < min((int)Contour.size(), x+5); i++)
        {
            int cx = Contour[i] / width;
            int cy = Contour[i] % width;
            if(i == x) continue;
            if(segIdx[Contour[i]] == segIdx[Contour[x]]){
                res += getK(Vector(cx - sx, cy - sy)), num++;
            }
        }
        return res / num;
    }
    void getMedialAxis(bool isPrint)
    {
        int getNum = 0;
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
            double stDir;
            int add;
            double dirK;
            if(Sign(stAvg) == 0) add = 0, dirK = 1;
            else if(Sign(stAvg - PI/2) == 0) add = 1, dirK = 0;
            else add = 1, dirK = 1/tan(stAvg);
//                int oriPos = contourSeg[j];
            int sx = Contour[i] / width;
            int sy = Contour[i] % width;
            int cx = sx+add;
            double cy = dirK + sy;
            if(color[cx * width + (int)(cy+0.5)] == 0) {
                add = -add;
                dirK = -dirK;
                cx = sx + add;
                cy = sy + dirK;
            }
            if(sx == 373 && sy == 433)
            {
                DEBUG(add);
                DEBUG(dirK);
            }
            while(cx >= 0 && cx < height && Sign(cy - width) < 0 && Sign(cy) >= 0)
            {
                int ry = cy+0.5;
                if(color[cx*width+ry] == 0) break;
//                if(sx == 373 && sy == 433)
//                {
//                    DEBUG(cx);
//                    DEBUG(ry);
//                    DEBUG(isContour[cx*width+ry]);
//                }
                if(ry < width && ry >= 0)
                {
                    if(isContour[cx*width+ry] > 0)
                    {
                        double peAvg = getAvgD(isContour[cx*width+ry] - 1);
//                        if(true
                        int mx, my;
                        if(abs(peAvg - stAvg) < 20.0*PI/180
                            && Sign(Length(Vector(sx, sy) - Vector(cx, ry)) - 2.0*penWidth) < 0)
                        {
                            mx = (sx+cx)*1.0/2+0.5;
                            my = (sy+cy)*1.0/2+0.5;
                            medial[mx*width+my] = 1;
                            getNum++;
                        }
                        if(mx == 295 && my == 410)
                        {
                            DEBUG(sx);
                            DEBUG(sy);
                            DEBUG(cx);
                            DEBUG(ry);
                        }

                        break;
                    }
                }
                cx += add;
                cy += dirK;
            }
        }
        DEBUG(getNum);
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
