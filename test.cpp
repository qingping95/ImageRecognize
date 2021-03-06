#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

BITMAPFILEHEADER  bf;

BITMAPINFOHEADER bi;

BOOL LoadBmpFile (HWND hWnd,char *BmpFileName)

{

    HFILE                      hf; //文件句柄

//指向BITMAPINFOHEADER结构的指针

    LPBITMAPINFOHEADER    lpImgData;

    LOGPALETTE                           *pPal; //指向逻辑调色板结构的指针

    LPRGBQUAD                            lpRGB; //指向RGBQUAD结构的指针

    HPALETTE                               hPrevPalette; //用来保存设备中原来的调色板

    HDC                                         hDc; //设备句柄

    HLOCAL                                   hPal; //存储调色板的局部内存句柄

    DWORD                                   LineBytes;  //每一行的字节数

    DWORD                                   ImgSize;   //实际的图象数据占用的字节数

//实际用到的颜色数 ，即调色板数组中的颜色个数

    DWORD                                   NumColors;

    DWORD                                   i;

    if((hf=_lopen(BmpFileName,OF_READ))==HFILE_ERROR)
    {

        MessageBox(hWnd,"File c:\\test.bmp not found!","Error Message",

                   MB_OK|MB_ICONEXCLAMATION);

        return FALSE; //打开文件错误，返回

    }

//将BITMAPFILEHEADER结构从文件中读出，填写到bf中

    _lread(hf,(LPSTR)&bf,sizeof(BITMAPFILEHEADER));

//将BITMAPINFOHEADER结构从文件中读出，填写到bi中

    _lread(hf,(LPSTR)&bi,sizeof(BITMAPINFOHEADER));

//我们定义了一个宏 #define WIDTHBYTES(i)    ((i+31)/32*4)上面曾经

//提到过，每一行的字节数必须是4的整倍数，只要调用

//WIDTHBYTES(bi.biWidth*bi.biBitCount)就能完成这一换算。举一个例

//子，对于2色图，如果图象宽是31，则每一行需要31位存储，合3个

//字节加7位，因为字节数必须是4的整倍数，所以应该是4，而此时的

//biWidth=31,biBitCount=1,WIDTHBYTES(31*1)=4，和我们设想的一样。

//再举一个256色的例子，如果图象宽是31，则每一行需要31个字节存

//储，因为字节数必须是4的整倍数，所以应该是32，而此时的

//biWidth=31,biBitCount=8,WIDTHBYTES(31*8)=32，我们设想的一样。你可

//以多举几个例子来验证一下

//LineBytes为每一行的字节数

    LineBytes=(DWORD)WIDTHBYTES(bi.biWidth*bi.biBitCount);

//ImgSize为实际的图象数据占用的字节数

    ImgSize=(DWORD)LineBytes*bi.biHeight;

//NumColors为实际用到的颜色数 ，即调色板数组中的颜色个数

    if(bi.biClrUsed!=0)

//如果bi.biClrUsed不为零，即为实际用到的颜色数

        NumColors=(DWORD)bi.biClrUsed;

    else //否则，用到的颜色数为2biBitCount。

        switch(bi.biBitCount)
        {

        case 1:

            NumColors=2;

            break;

        case 4:

            NumColors=16;

            break;

        case 8:

            NumColors=256;

            break;

        case 24:

            NumColors=0; //对于真彩色图，没用到调色板

            break;

        default: //不处理其它的颜色数，认为出错。

            MessageBox(hWnd,"Invalid color numbers!","Error Message",

                       MB_OK|MB_ICONEXCLAMATION);

            _lclose(hf);

            return FALSE; //关闭文件，返回FALSE

        }

    if(bf.bfOffBits!=(DWORD)(NumColors*sizeof(RGBQUAD)+

                             sizeof(BITMAPFILEHEADER)+

                             sizeof(BITMAPINFOHEADER)))

    {

//计算出的偏移量与实际偏移量不符，一定是颜色数出错

        MessageBox(hWnd,"Invalid color numbers!","Error Message",

                   MB_OK|MB_ICONEXCLAMATION);

        _lclose(hf);

        return FALSE; //关闭文件，返回FALSE

    }

    bf.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+

              NumColors*sizeof(RGBQUAD)+ImgSize;

//分配内存，大小为BITMAPINFOHEADER结构长度加调色板+实际位图

    if((hImgData=GlobalAlloc(GHND,(DWORD)

                             (sizeof(BITMAPINFOHEADER)+

                              NumColors*sizeof(RGBQUAD)+

                              ImgSize)))==NULL)

    {

//分配内存错误

        MessageBox(hWnd,"Error alloc memory!","ErrorMessage",MB_OK|

                   MB_ICONEXCLAMATION);

        _lclose(hf);

        return FALSE; //关闭文件，返回FALSE

    }

//指针lpImgData指向该内存区

    lpImgData=(LPBITMAPINFOHEADER)GlobalLock(hImgData);

//文件指针重新定位到BITMAPINFOHEADER开始处

    _llseek(hf,sizeof(BITMAPFILEHEADER),SEEK_SET);

//将文件内容读入lpImgData

    _hread(hf,(char *)lpImgData,(long)sizeof(BITMAPINFOHEADER)

           +(long)NumColors*sizeof(RGBQUAD)+ImgSize);

    _lclose(hf); //关闭文件

    if(NumColors!=0) //NumColors不为零，说明用到了调色板

    {

//为逻辑调色板分配局部内存，大小为逻辑调色板结构长度加

//NumColors个PALETTENTRY

        hPal=LocalAlloc(LHND,sizeof(LOGPALETTE)+

                        NumColors* sizeof(PALETTEENTRY));

//指针pPal指向该内存区

        pPal =(LOGPALETTE *)LocalLock(hPal);

        //填写逻辑调色板结构的头

        pPal->palNumEntries = NumColors;

        pPal->palVersion = 0x300;

//lpRGB指向的是调色板开始的位置

        lpRGB = (LPRGBQUAD)((LPSTR)lpImgData +

                            (DWORD)sizeof(BITMAPINFOHEADER));

//填写每一项

        for (i = 0; i < NumColors; i++)

        {

            pPal->palPalEntry[i].peRed=lpRGB->rgbRed;

            pPal->palPalEntry[i].peGreen=lpRGB->rgbGreen;

            pPal->palPalEntry[i].peBlue=lpRGB->rgbBlue;

            pPal->palPalEntry[i].peFlags=(BYTE)0;

            lpRGB++; //指针移到下一项

        }

//产生逻辑调色板，hPalette是一个全局变量

        hPalette=CreatePalette(pPal);

//释放局部内存

        LocalUnlock(hPal);

        LocalFree(hPal);

    }

//获得设备上下文句柄

    hDc=GetDC(hWnd);

    if(hPalette) //如果刚才产生了逻辑调色板

    {

//将新的逻辑调色板选入DC，将旧的逻辑调色板句柄保存在//hPrevPalette

        hPrevPalette=SelectPalette(hDc,hPalette,FALSE);

        RealizePalette(hDc);

    }

//产生位图句柄

    hBitmap=CreateDIBitmap(hDc,(LPBITMAPINFOHEADER)lpImgData,

                           (LONG)CBM_INIT,

                           (LPSTR)lpImgData+sizeof(BITMAPINFOHEADER)+NumColors*sizeof(RGBQUAD),

                           (LPBITMAPINFO)lpImgData, DIB_RGB_COLORS);

//将原来的调色板(如果有的话)选入设备上下文句柄

    if(hPalette && hPrevPalette)

    {

        SelectPalette(hDc,hPrevPalette,FALSE);

        RealizePalette(hDc);

    }

    ReleaseDC(hWnd,hDc); //释放设备上下文

    GlobalUnlock(hImgData); //解锁内存区

    return TRUE; //成功返回

}

