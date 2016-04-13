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

    HFILE                      hf; //�ļ����

//ָ��BITMAPINFOHEADER�ṹ��ָ��

    LPBITMAPINFOHEADER    lpImgData;

    LOGPALETTE                           *pPal; //ָ���߼���ɫ��ṹ��ָ��

    LPRGBQUAD                            lpRGB; //ָ��RGBQUAD�ṹ��ָ��

    HPALETTE                               hPrevPalette; //���������豸��ԭ���ĵ�ɫ��

    HDC                                         hDc; //�豸���

    HLOCAL                                   hPal; //�洢��ɫ��ľֲ��ڴ���

    DWORD                                   LineBytes;  //ÿһ�е��ֽ���

    DWORD                                   ImgSize;   //ʵ�ʵ�ͼ������ռ�õ��ֽ���

//ʵ���õ�����ɫ�� ������ɫ�������е���ɫ����

    DWORD                                   NumColors;

    DWORD                                   i;

    if((hf=_lopen(BmpFileName,OF_READ))==HFILE_ERROR)
    {

        MessageBox(hWnd,"File c:\\test.bmp not found!","Error Message",

                   MB_OK|MB_ICONEXCLAMATION);

        return FALSE; //���ļ����󣬷���

    }

//��BITMAPFILEHEADER�ṹ���ļ��ж�������д��bf��

    _lread(hf,(LPSTR)&bf,sizeof(BITMAPFILEHEADER));

//��BITMAPINFOHEADER�ṹ���ļ��ж�������д��bi��

    _lread(hf,(LPSTR)&bi,sizeof(BITMAPINFOHEADER));

//���Ƕ�����һ���� #define WIDTHBYTES(i)    ((i+31)/32*4)��������

//�ᵽ����ÿһ�е��ֽ���������4����������ֻҪ����

//WIDTHBYTES(bi.biWidth*bi.biBitCount)���������һ���㡣��һ����

//�ӣ�����2ɫͼ�����ͼ�����31����ÿһ����Ҫ31λ�洢����3��

//�ֽڼ�7λ����Ϊ�ֽ���������4��������������Ӧ����4������ʱ��

//biWidth=31,biBitCount=1,WIDTHBYTES(31*1)=4�������������һ����

//�پ�һ��256ɫ�����ӣ����ͼ�����31����ÿһ����Ҫ31���ֽڴ�

//������Ϊ�ֽ���������4��������������Ӧ����32������ʱ��

//biWidth=31,biBitCount=8,WIDTHBYTES(31*8)=32�����������һ�������

//�Զ�ټ�����������֤һ��

//LineBytesΪÿһ�е��ֽ���

    LineBytes=(DWORD)WIDTHBYTES(bi.biWidth*bi.biBitCount);

//ImgSizeΪʵ�ʵ�ͼ������ռ�õ��ֽ���

    ImgSize=(DWORD)LineBytes*bi.biHeight;

//NumColorsΪʵ���õ�����ɫ�� ������ɫ�������е���ɫ����

    if(bi.biClrUsed!=0)

//���bi.biClrUsed��Ϊ�㣬��Ϊʵ���õ�����ɫ��

        NumColors=(DWORD)bi.biClrUsed;

    else //�����õ�����ɫ��Ϊ2biBitCount��

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

            NumColors=0; //�������ɫͼ��û�õ���ɫ��

            break;

        default: //��������������ɫ������Ϊ����

            MessageBox(hWnd,"Invalid color numbers!","Error Message",

                       MB_OK|MB_ICONEXCLAMATION);

            _lclose(hf);

            return FALSE; //�ر��ļ�������FALSE

        }

    if(bf.bfOffBits!=(DWORD)(NumColors*sizeof(RGBQUAD)+

                             sizeof(BITMAPFILEHEADER)+

                             sizeof(BITMAPINFOHEADER)))

    {

//�������ƫ������ʵ��ƫ����������һ������ɫ������

        MessageBox(hWnd,"Invalid color numbers!","Error Message",

                   MB_OK|MB_ICONEXCLAMATION);

        _lclose(hf);

        return FALSE; //�ر��ļ�������FALSE

    }

    bf.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+

              NumColors*sizeof(RGBQUAD)+ImgSize;

//�����ڴ棬��СΪBITMAPINFOHEADER�ṹ���ȼӵ�ɫ��+ʵ��λͼ

    if((hImgData=GlobalAlloc(GHND,(DWORD)

                             (sizeof(BITMAPINFOHEADER)+

                              NumColors*sizeof(RGBQUAD)+

                              ImgSize)))==NULL)

    {

//�����ڴ����

        MessageBox(hWnd,"Error alloc memory!","ErrorMessage",MB_OK|

                   MB_ICONEXCLAMATION);

        _lclose(hf);

        return FALSE; //�ر��ļ�������FALSE

    }

//ָ��lpImgDataָ����ڴ���

    lpImgData=(LPBITMAPINFOHEADER)GlobalLock(hImgData);

//�ļ�ָ�����¶�λ��BITMAPINFOHEADER��ʼ��

    _llseek(hf,sizeof(BITMAPFILEHEADER),SEEK_SET);

//���ļ����ݶ���lpImgData

    _hread(hf,(char *)lpImgData,(long)sizeof(BITMAPINFOHEADER)

           +(long)NumColors*sizeof(RGBQUAD)+ImgSize);

    _lclose(hf); //�ر��ļ�

    if(NumColors!=0) //NumColors��Ϊ�㣬˵���õ��˵�ɫ��

    {

//Ϊ�߼���ɫ�����ֲ��ڴ棬��СΪ�߼���ɫ��ṹ���ȼ�

//NumColors��PALETTENTRY

        hPal=LocalAlloc(LHND,sizeof(LOGPALETTE)+

                        NumColors* sizeof(PALETTEENTRY));

//ָ��pPalָ����ڴ���

        pPal =(LOGPALETTE *)LocalLock(hPal);

        //��д�߼���ɫ��ṹ��ͷ

        pPal->palNumEntries = NumColors;

        pPal->palVersion = 0x300;

//lpRGBָ����ǵ�ɫ�忪ʼ��λ��

        lpRGB = (LPRGBQUAD)((LPSTR)lpImgData +

                            (DWORD)sizeof(BITMAPINFOHEADER));

//��дÿһ��

        for (i = 0; i < NumColors; i++)

        {

            pPal->palPalEntry[i].peRed=lpRGB->rgbRed;

            pPal->palPalEntry[i].peGreen=lpRGB->rgbGreen;

            pPal->palPalEntry[i].peBlue=lpRGB->rgbBlue;

            pPal->palPalEntry[i].peFlags=(BYTE)0;

            lpRGB++; //ָ���Ƶ���һ��

        }

//�����߼���ɫ�壬hPalette��һ��ȫ�ֱ���

        hPalette=CreatePalette(pPal);

//�ͷžֲ��ڴ�

        LocalUnlock(hPal);

        LocalFree(hPal);

    }

//����豸�����ľ��

    hDc=GetDC(hWnd);

    if(hPalette) //����ղŲ������߼���ɫ��

    {

//���µ��߼���ɫ��ѡ��DC�����ɵ��߼���ɫ����������//hPrevPalette

        hPrevPalette=SelectPalette(hDc,hPalette,FALSE);

        RealizePalette(hDc);

    }

//����λͼ���

    hBitmap=CreateDIBitmap(hDc,(LPBITMAPINFOHEADER)lpImgData,

                           (LONG)CBM_INIT,

                           (LPSTR)lpImgData+sizeof(BITMAPINFOHEADER)+NumColors*sizeof(RGBQUAD),

                           (LPBITMAPINFO)lpImgData, DIB_RGB_COLORS);

//��ԭ���ĵ�ɫ��(����еĻ�)ѡ���豸�����ľ��

    if(hPalette && hPrevPalette)

    {

        SelectPalette(hDc,hPrevPalette,FALSE);

        RealizePalette(hDc);

    }

    ReleaseDC(hWnd,hDc); //�ͷ��豸������

    GlobalUnlock(hImgData); //�����ڴ���

    return TRUE; //�ɹ�����

}

