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
#include <map>

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
    int *isContour;             //记录某个点是否轮廓，值为点的在Contour中的编号+1
    int *color;                 //记录每个点的颜色，前景or背景
    int *pcolor;                //给每个点赋颜色，便于存储
    int *medial;                //记录某个点是不是提取的骨架
    int *segIdx;                //记录某个点的轮廓线段的编号，即在ContourSeg中的下标
    double *theta;              //
    double *dTheta;             //
    bool *vis;                  //
    int width, height, n;       //
    int penWidth;               //
    int BACK,FORE;              //
    const double segDiff = 10.0/180*PI;
    const double midDiff = 10.0*PI/180;
    vector<int > Contour;       //记录轮廓点
    vector<PII > contourSeg;    //记录切割轮廓后形成的线段
    vector<PII > segment;       //中间值
    vector<int> midPoint;       //记载骨架点
    vector<vector<int> > midSeg;   //记录每一个线段包括的骨架点, 里面存的值是骨架点的下标。
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
        if(A.x == 0) return PI/2;
        return atan(A.y/A.x);
//        double ang = angle(A);
//        if(ang > PI/2) ang -= PI;
//        if(ang < -PI/2) ang += PI;
//        if(abs(ang - PI/2) < 1e-4) ang -= PI;
//        return ang;
    }
    int getSegmentEndPoint(int stidx)
    {
//        DEBUG(stidx);
//        DEBUG(Contour.size());
        segment.clear();
        int sx = Contour[stidx] / width;
        int sy = Contour[stidx] % width;
        for(int i = stidx+1; i < Contour.size(); i++)
        {
            int cx = Contour[i] / width;
            int cy = Contour[i] % width;
            theta[i] = getK(Vector(cx - sx, cy - sy));
            if(theta[i] == PI/2) theta[i] = 0;
            if(i>stidx+1) dTheta[i] = theta[i] - theta[i-1];
        }
        int st = stidx+1;
        int len = 4;
        for(int i = st+1; i < Contour.size(); i++)
        {
            int sx = Contour[i]/width, sy = Contour[i]%width;
            Point pre;
            for(int j = max(stidx, i-len); j < i; j++)
            {
                int cx = Contour[j] / width;
                int cy = Contour[j] % width;
                pre = pre + Vector(cx - sx, cy - sy);
            }
            Point suf;
            for(int j = i+1; j < min((int)Contour.size(), i+len+1); j++)
            {
                int cx = Contour[j] / width;
                int cy = Contour[j] % width;
                suf = suf + Vector(cx - sx, cy - sy);
            }
            if(Angle(pre, suf) < 115.0/180*PI)
            {
                return i;
            }
        }
        for(int i = st+1; i < Contour.size(); i++)
        {
            if(Sign(dTheta[i]) != Sign(dTheta[i-1]))
            {
                //cerr<<"segment.size() -> "<<segment.size()<<endl;
                segment.push_back(PII(st, i-1));
                st = i;
            }
        }
        if(st < Contour.size()-1) segment.push_back(PII(st, Contour.size()-1));
        int averheight = min(height, width)*2/3;
        for(int i = 0; i < segment.size(); i++)
        {
//            double diff = theta[segment[i].second] - theta[segment[max(i-1, 0)].first];
            double diff = theta[segment[i].second] - theta[segment[i].first];
//            double diff = theta[segment[i].second];
            if(abs(diff) > segDiff){//
//            if(abs(diff) > segDiff || segment[i].second - stidx > averheight){//
                if(i){
                    return segment[i-1].second+1;
                    //contourSeg.push_back(PII(stidx, segment[i-1].second));
                }else{
                    for(int j = segment[i].first; j <= segment[i].second; j++)
                    {
                        double curd = theta[j] - theta[segment[i].first];
                        if(abs(curd) > segDiff){//5.0/180*PI
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
        cerr<<"Contour.size() -> "<< (Contour.size())<<endl;
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
//            cerr<<st<<" -> "<<tmp<<endl;
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
        for(int i = 0; i < n; i++){
            if(medial[i]) pcolor[i] = 4;
            else if(pcolor[i] == 0) pcolor[i] = 255;
//            if(pcolor[i] == 0) pcolor[i] = 255;
//            if(isContour[i] == 0 && medial[i]) pcolor[i] = 0;
        }
    }
    //return the angle of pixel position x
    double getAvgD(int x)
    {
        int num = 0;
        double res = 0;
        int sx = Contour[x] / width;
        int sy = Contour[x] % width;
        for(int i = max(0, x - 3); i < min((int)Contour.size(), x+3); i++)
        {
            int cx = Contour[i] / width;
            int cy = Contour[i] % width;
            if(i == x) continue;
//            if(segIdx[Contour[i]] == segIdx[Contour[x]]){
                res += getK(Vector(cx - sx, cy - sy)), num++;
//            }
        }
        return res / num;
    }

    void getMedialAxis(bool isPrint)
    {
        int getNum = 0;
//        memset(vis, 0, sizeof(bool )*height*width);
        memset(medial, 0, sizeof(int)*height*width);
        midSeg.clear();
        midSeg = vector<vector<int> >(contourSeg.size());
        for(int i = 0; i < contourSeg.size(); i ++)
            for(int j = contourSeg[i].first; j < contourSeg[i].second; j++)
            {
                pcolor[Contour[j]] = i+1;
            }
//        for(int i = 0; i < contourSeg.size(); i ++)
//            for(int j = contourSeg[i].first; j <= contourSeg[i].second; j++)
        cerr <<"MARK!!!!!!!!!!"<<endl;
        for(int i = 0; i < Contour.size(); i++)
        {
//            if(vis[i]) continue;
//            vis[i] = 1;
            //返回的是这个位置的平均极角
            double stAvg = getAvgD(i);
            double stDir;
            double add;
            double dirK;
            if(Sign(stAvg) == 0) add = 0, dirK = 1;
            else if(Sign(stAvg + PI/2) == 0) add = 1, dirK = 0;
            else add = 1, dirK = 1/tan(stAvg);
//                int oriPos = contourSeg[j];
            int sx = Contour[i] / width;
            int sy = Contour[i] % width;
            if(fabs(dirK) > 1.1) add /= dirK, dirK /= dirK;
            double cx = sx+add;
            double cy = dirK + sy;
            double ccx = cx+add;
            double ccy = cy + dirK;
            if(color[(int)(cx+0.5) * width + (int)(cy+0.5)] == 0 || color[(int)(ccx+0.5) * width + (int)(ccy+0.5)] == 0) {
                add = -add;
                dirK = -dirK;
                cx = sx + add;
                cy = sy + dirK;
            }

            int num = 0;
            while(Sign(cx) >= 0 && Sign(cx - height) < 0 && Sign(cy - width) < 0 && Sign(cy) >= 0)
            {
                num++;
                int rx = cx+0.5;
                int ry = cy+0.5;
                if(color[rx*width+ry] == 0) break;
                if(ry < width && ry >= 0 && num > 10)
                {
                    if(isContour[rx*width+ry] > 0)
                    {
                        //vis[rx*width+ry] = 1;
                        double peAvg = getAvgD(isContour[rx*width+ry] - 1);
                        int mx, my;
                        if(abs(peAvg - stAvg) < midDiff
                            && Sign(Length(Vector(sx, sy) - Vector(rx, ry)) - 2.0*penWidth) < 0 && segIdx[sx*width+sy] != segIdx[rx*width+ry])
                        {
                            mx = (sx+cx)*1.0/2+0.5;
                            my = (sy+cy)*1.0/2+0.5;
                            if(mx == 155 && my == 195)
                            {
                                DEBUG(mx);
                                DEBUG(my);
                                DEBUG(sx);
                                DEBUG(sy);
                                DEBUG(rx);
                                DEBUG(ry);
                            }
                            //if(medial[mx*width+my] != 1){
                                medial[mx*width+my] = 1;
                                getNum++;
                                midPoint.emplace_back(mx*width+my);
                                midSeg[segIdx[sx*width+sy]].emplace_back(midPoint.size()-1);
                                //midSeg[segIdx[rx*width+ry]].emplace_back(midPoint.size()-1);
                            //}
                        }
                        break;
                    }
                }
                cx += add;
                cy += dirK;
            }
        }
        map<int, int> mp;
        for(int i = 0; i < midSeg.size(); i++)
        {
            mp.clear();
            for(int j = 0; j < midSeg[i].size(); j++)
            {
                int idx = midSeg[i][j];
                idx = midPoint[idx];
                if(mp.find(idx) != mp.end())
                {
                    midSeg[i].erase(midSeg[i].begin()+j);
                    j--;
                }else{
                    mp[idx] = 1;
                }
            }
        }
//        clearMid();
        DEBUG(getNum);
        if(isPrint)
        {
            printf("Medial axis image is :\n");
            for(int i = 0; i < n; i++)
                if(isContour[i]) pcolor[i] = 1;
                else if(medial[i]) pcolor[i] = 2;
                else pcolor[i] = 0;
            for(int i = height-1; i >= 0; i--, printf("\n"))
                for(int j = 0; j < width; j++)
                    printf("%c", ".#$"[pcolor[i*width+j]]);
        }
    }
    ///这里死循环了，记得来debug --- 已修复
    vector<int> linkTwoPoint(int sx, int sy, int ex, int ey)
    {
        vector<int> pt;
        double stAvg = getK(Vector(sx-ex, sy-ey));
        double add;
        double dirK;
        if(Sign(stAvg) == 0) add = 0, dirK = 1;
        else if(Sign(stAvg + PI/2) == 0 || Sign(stAvg - PI/2) == 0) add = 1, dirK = 0;
        else add = 1, dirK = 1.0*(sy-ey)/(sx-ex);

        if(fabs(dirK) > 1.1) add /= dirK, dirK /= dirK;
        if(sx > ex && add > 0) add = -add;
        if(sx < ex && add < 0) add = -add;
        if(sy > ey && dirK > 0) dirK = -dirK;
        if(sy < ey && dirK < 0) dirK = -dirK;
        double cx = sx+add;
        double cy = dirK + sy;
        //cerr<<cx<<" "<<cy<<" "<<stAvg<<endl;
        while(true)
        {
            int tx = cx, ty = cy;
            if(sx <= ex && tx > ex) break;
            if(sx >= ex && tx < ex) break;
            if(sy <= ey && ty > ey) break;
            if(sy >= ey && ty < ey) break;
            //cerr<<tx<<" "<<ty<<" "<<sx<<" "<<sy<<" "<<ex<<" "<<ey<<endl;
            pt.push_back(tx*width+ty);
            if(color[tx*width+ty] == 0){
                pt.clear();
                return pt;
            }
            cx += add;
            cy += dirK;
        }
        return pt;
    }
    vector<int> DDA_DrawLine(int sx, int sy, int ex, int ey)
    {
        long YDis = (ey - sy);
        long XDis = (ex - sx);
        long MaxStep = max(abs(XDis),abs(YDis)); // 步进的步数
        float fXUnitLen = 1.0f;  // X方向的单位步进
        float fYUnitLen = 1.0f;  // Y方向的单位步进
        fYUnitLen = static_cast<float>(YDis)/static_cast<float>(MaxStep);
        fXUnitLen = static_cast<float>(XDis)/static_cast<float>(MaxStep);
        // 设置起点像素颜色
//        pDC->SetPixel(sx,sy,LineCor);
        vector<int> pt;
        pt.push_back(sx*width+sy);
        float x = static_cast<float>(sx);
        float y = static_cast<float>(sy);
        // 循环步进
        for (long i = 1;i<=MaxStep;i++)
        {
            x = x + fXUnitLen;
            y = y + fYUnitLen;
            int xx = x+0.5, yy = y+0.5;
            pt.push_back(xx*width+yy);
//            pDC->SetPixel(FloatToInteger(x),FloatToInteger(y),LineCor);
        }
        return pt;
    }
    void linkOneMidSeg(vector<int>& seg)
    {
        int len = 4;
        for(int i = 1; i < seg.size(); i++)
        {
            int pid = seg[i-1], cid = seg[i];
            pid = midPoint[pid], cid = midPoint[cid];
            int px = pid / width, py = pid % width;
            int cx = cid / width, cy = cid % width;
            if(max(abs(px-cx), abs(cy-py)) > 1)
            {
                Point pre;
                for(int j = max(0, i-1-len); j < i-1; j++)
                {
                    int sx = midPoint[seg[j]]/width, sy = midPoint[seg[j]]%width;
                    pre = pre + Vector(sx-px, sy-py);
                }
                //将两个点用直线连接起来。
                if(i == 1 || Angle(pre, Vector(cx-px, cy-py)) > 120.0/180*PI)
                {
                    vector<int> pt = DDA_DrawLine(px, py, cx, cy);
//                    vector<int> pt = linkTwoPoint(px, py, cx, cy);
                    //seg.insert(seg.begin()+i, pt.begin(), pt.end());
                    for(int j = 0; j < pt.size(); j++)
                    {
                        medial[pt[j]] = 1;
                    }
                }
            }
        }
    }
    void clearOneMidSeg(vector<int>& seg)
    {
        DEBUG(seg.size());
        int len = 4;
        for(int i = 2; i+1 < seg.size(); i++)
        {
            int cid = seg[i];
            int sx = midPoint[cid]/width, sy = midPoint[cid]%width;
            double pre = 0;
            int num = 0;
            for(int j = max(0, i-len); j < i; j++)
            {
                int cx = midPoint[seg[j]] / width;
                int cy = midPoint[seg[j]] % width;
                pre += getK(Vector(cx - sx, cy - sy)), num++;
            }
            pre /= num;
            double suf = 0;
            num = 0;
            for(int j = i+1; j < min((int)seg.size(), i+len+1); j++)
            {
                int cx = midPoint[seg[j]] / width;
                int cy = midPoint[seg[j]] % width;
                suf += getK(Vector(cx - sx, cy - sy)), num++;
            }
            suf /= num;
            if(abs(pre - suf) > midDiff)
            {
                medial[midPoint[cid]] = 0;
            }
        }
        for(int i = 0; i < seg.size(); i++)
        {
            int cid = seg[i];
            if(medial[midPoint[cid]] == 0)
            {
                seg.erase(seg.begin()+i);
                i--;
            }
        }
    }
    void clearMid()
    {
        cout<<midSeg.size()<<endl;
        for(int i = 0; i < midSeg.size(); i++)
        {
            if(midSeg[i].empty()) continue;
            linkOneMidSeg(midSeg[i]);
            clearOneMidSeg(midSeg[i]);
        }
    }
};

#endif // CONTOURBASETHIN_H_INCLUDED
