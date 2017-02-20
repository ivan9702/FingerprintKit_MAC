//
//  image.c
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#include "image.h"
#include <stdlib.h>
#include <string.h>

#pragma mark - Graphic Operation


int clip(int src_w, int src_h,
         uint8_t *src, uint8_t *dst,
         unsigned short x_str, unsigned short y_str,
         unsigned short x_disp, unsigned short y_disp)
{
    long count = 0;
    long offset = (long)y_str * src_w + x_str;
    
    if ((x_str + x_disp > src_w) || (y_str + y_disp > src_h))
        return -1;
    
    for (int i = 0; i < y_disp; i++) {
        memcpy(dst + count, src + offset, x_disp);
        
        offset += src_w;
        count += x_disp;
    }
    return 0;
}

int resize_cubic_v6(uint8_t *InImg, uint8_t *OutImg, int src_w, int src_h, int tgt_w, int tgt_h)
{
    int i,j,tmpyi,tmpxi,tmpx_ori,tmpy_ori;
    int tmpy,tmpx;
    int offset,offset_t;
    int rx,ry,rxi,ryi;
    int r0,r1,r2,r3,result;
    int dis;
    int f0_dis[1024],f1_dis[1024],f2_dis[1024],f3_dis[1024];
    int f0_disy[1024],f1_disy[1024],f2_disy[1024],f3_disy[1024];
    int dx0,dx1,dx2,dx3;
    int dy0,dy1,dy2,dy3;
    
    memset(OutImg,0,tgt_w*tgt_h);
    rx=(src_w<<7)/tgt_w;
    ry=(src_h<<7)/tgt_h;
    rxi=(src_w/tgt_w)<<7;
    ryi=(src_h/tgt_h)<<7;
    
    //first 2 row use NN
    offset=0;
    for(j=0;j<2;j++){
        tmpy_ori=j*(src_h/tgt_h);
        offset_t=tmpy_ori*src_w;
        for(i=0;i<tgt_w;i++){
            tmpx_ori=(i*src_w)/tgt_w;
            OutImg[offset]=InImg[(offset_t+tmpx_ori)];
            offset++;
        }
    }
    //
    //precompute dis_value in f0,f1,f2,f3
    for(i=0;i<tgt_w;i++){
        tmpx=i*(src_w<<7)/tgt_w;
        tmpxi=((i*src_w)/tgt_w)<<7;
        dis=tmpx-tmpxi;
        //printf("dis :%d\n",dis);
        
        f0_dis[i]=(-dis)*(128-dis)*(128-dis);
        f1_dis[i]=(((128-dis)<<14)+(dis*(128-dis)*(128-dis)));
        f2_dis[i]=((dis<<14)+(dis*dis*(128-dis)));
        f3_dis[i]=((-dis)*(dis)*(128-dis));
    }
    
    for(j=2;j<tgt_h-1;j++){
        tmpy=j*(src_h<<7)/tgt_h;
        tmpyi=((j*src_h)/tgt_h)<<7;
        dis=tmpy-tmpyi;
        
        f0_disy[j]=(-dis)*(128-dis)*(128-dis);
        f1_disy[j]=(((128-dis)<<14)+(dis*(128-dis)*(128-dis)));
        f2_disy[j]=((dis<<14)+(dis*dis*(128-dis)));
        f3_disy[j]=((-dis)*(dis)*(128-dis));
    }
    //
    
    offset=2*tgt_w;
    for(j=2;j<tgt_h-1;j++){
        tmpy_ori=((j*src_h)/tgt_h);
        offset_t=tmpy_ori*src_w;
        dy0=f0_disy[j];
        dy1=f1_disy[j];
        dy2=f2_disy[j];
        dy3=f3_disy[j];
        for(i=0;i<tgt_w;i++){
            tmpx_ori=(i*src_w)/tgt_w;
            dx0=f0_dis[i];
            dx1=f1_dis[i];
            dx2=f2_dis[i];
            dx3=f3_dis[i];
            
            r0=dx0*InImg[(offset_t+tmpx_ori)-src_w-1]
            +dx1*InImg[(offset_t+tmpx_ori)-src_w]
            +dx2*InImg[(offset_t+tmpx_ori)-src_w+1]
            +dx3*InImg[(offset_t+tmpx_ori)-src_w+2];
            r1=dx0*InImg[(offset_t+tmpx_ori)-1]
            +dx1*InImg[(offset_t+tmpx_ori)]
            +dx2*InImg[(offset_t+tmpx_ori)+1]
            +dx3*InImg[(offset_t+tmpx_ori)+2];
            r2=dx0*InImg[(offset_t+tmpx_ori)+src_w-1]
            +dx1*InImg[(offset_t+tmpx_ori)+src_w]
            +dx2*InImg[(offset_t+tmpx_ori)+src_w+1]
            +dx3*InImg[(offset_t+tmpx_ori)+src_w+2];
            r3=dx0*InImg[(offset_t+tmpx_ori)+src_w+src_w-1]
            +dx1*InImg[(offset_t+tmpx_ori)+src_w+src_w]
            +dx2*InImg[(offset_t+tmpx_ori)+src_w+src_w+1]
            +dx3*InImg[(offset_t+tmpx_ori)+src_w+src_w+2];
            
            //	result=f0(dy,r0)+f1(dy,r1)+f2(dy,r2)+f3(dy,r3);
            result=(dy0>>13)*(r0>>14)+(dy1>>13)*(r1>>14)+(dy2>>13)*(r2>>14)+(dy3>>13)*(r3>>14);  //split divide
            result=(result>>15);
            
            if(result>255)
                OutImg[offset]=255;
            else if(result<0)
                OutImg[offset]=0;
            else
                OutImg[offset]=(uint8_t)result;
            
            offset++;
        }
    }
    
    //last row use NN
    offset=(tgt_h-1)*tgt_w;
    for(j=tgt_h-1;j<tgt_h;j++){
        tmpy_ori=(j*src_h)/tgt_h;
        offset_t=tmpy_ori*src_w;
        for(i=0;i<tgt_w;i++){
            tmpx_ori=(i*src_w)/tgt_w;
            OutImg[offset+i]=InImg[(offset_t+tmpx_ori)];
        }
    }
    return 0;
}

#pragma mark - Color Conversion

int m_channel_red=1;
int m_channel_green=1;
int m_channel_blue=1;

// R at (x%2, y%2) = (0, 0)
// B at (x%2, y%2) = (1, 1)
int Get1(unsigned char **p, int x, int y)
{
    int cnt=0;
    int e1=0, e2=0, e3=0, e4=0;
    if (y-1 >=0 && x-1 >=0)
    {
        cnt++;
        e1 = (int) p[y-1][x-1];
    }
    if (y-1>=0 && x+1 <=351)
    {
        cnt++;
        e2 = (int) p[y-1][x+1];
    }
    if (y+1 <=287 && x-1 >=0)
    {
        cnt++;
        e3 = (int) p[y+1][x-1];
    }
    if (y+1 <=287 && x+1 <= 351)
    {
        cnt++;
        e4 = (int) p[y+1][x+1];
    }
    
    return (int)((e1+ e2 + e3 + e4)/(float)cnt + 0.5);
}

// G at (x%2, y%2) = (1, 1)
// G at (x%2, y%2) = (0, 0)
int Get2(unsigned char **p, int x, int y)
{
    int cnt=0;
    int e1=0, e2=0, e3=0, e4=0;
    if (y-1 >=0)
    {
        cnt++;
        e1 = (int) p[y-1][x];
    }
    if (x-1>=0)
    {
        cnt++;
        e2 = (int) p[y][x-1];
    }
    if (y+1<=287)
    {
        cnt++;
        e3 = (int) p[y+1][x];
    }
    if (x+1<=351)
    {
        cnt++;
        e4 = (int) p[y][x+1];
    }
    
    return (int)((e1+ e2 + e3 + e4)/(float)cnt + 0.5);
}

// R at (x%2, y%2) = (1, 0)
// B at (x%2, y%2) = (0, 1)
int Get3(unsigned char **p, int x, int y)
{
    int cnt=0;
    int e1=0, e2=0;
    if (y-1 >=0)
    {
        cnt++;
        e1 = (int) p[y-1][x];
    }
    if (y+1 <=287)
    {
        cnt++;
        e2 = (int) p[y+1][x];
    }
    
    return (int)((e1+ e2)/(float)cnt + 0.5);
}

// B at (x%2, y%2) = (1, 0)
// R at (x%2, y%2) = (0, 1)
int Get4(unsigned char **p, int x, int y)
{
    int cnt=0;
    int e1=0, e2=0;
    if (x-1 >=0)
    {
        cnt++;
        e1 = (int) p[y][x-1];
    }
    if (x+1 <=351)
    {
        cnt++;
        e2 = (int) p[y][x+1];
    }
    
    return (int)((e1+ e2)/(float)cnt + 0.5);
}
/*  RGB bayer patter:
 
 x-index
 0123456789
 y 0 GRGRGRGRGRGR
 | 1 BGBGBGBGBGBG
 i 2 GRGRGRGRGRGR
 d 3 BGBGBGBGBGBG
 x 4 GRGRGRGRGRGR
 
*/
int rgb_bayer_to_gray(unsigned char *in, unsigned char *out, int width, int height)
{
    int r = 0, g = 0, b = 0;
    int rr, gg, bb, sum;
    
    rr = m_channel_red   ? 299 : 0;
    gg = m_channel_green ? 587 : 0;
    bb = m_channel_blue  ? 114 : 0;
    sum = rr + gg + bb;

    unsigned char ** in2  = (unsigned char **) calloc(height, sizeof(unsigned char *));
    unsigned char ** out2 = (unsigned char **) calloc(height, sizeof(unsigned char *));
    
    for (int y = 0; y < height; y++)
    {
        in2[y]  =  in  + y * width;
        out2[y] =  out + y * width;
    }
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x % 2 == 0) && (y % 2 == 0))
            {
                r = Get4(in2, x, y);
                g = in2[y][x];
                b = in2[y][x];
            }
            else if ((x%2==0) && (y % 2 == 1))		//b
            {
                r = Get1(in2, x, y);
                g = Get2(in2, x, y);
                b = in2[y][x];
            }
            else if ((x%2==1) && (y %2==0))	//r
            {
                r = in2[y][x];
                g = Get2(in2, x, y);
                b = Get1(in2, x, y);
            }
            else if ((x%2==1) && (y %2==1))
            {
                r = Get3(in2, x, y);
                g = in2[y][x];
                b = in2[y][x];
            }
            //out2[y][x] = (int)(0.299*r + 0.587*g + 0.114*b+0.5);
            //out2[y][x] = (int)(299*r + 587*g + 114*b+500)/1000;
            //out2[y][x] = (int)( 587*g + 114*b+350)/701;
            if (sum!=0)
                out2[y][x] = (int)(rr*r + gg*g + bb*b+sum/2)/sum;
            else
                out2[y][x] = (int)(299*r + 587*g + 114*b+ 500)/1000;
            
        }
    }
    free(in2);
    free(out2);
    return 1;
}

#pragma mark - Effect

void gray_enhance(uint8_t *g_lpTempBuf, int w, int h)
{
    long i=0, j=0;
    unsigned char max1=0, max2=0, max3=0, min1=255, min2=255, min3=255;
    int tmp;
    
    double a,b;
    int chk_start_x,chk_end_x,chk_start_y,chk_end_y;
    chk_start_x=(w-200)/2;
    chk_end_x=chk_start_x+200;
    chk_start_y=(h-200)/2;
    chk_end_y=chk_start_y+200;
    
    if(1){
        for(i=chk_start_y;i<chk_end_y;i++)
        {
            for(j=chk_start_x;j<chk_end_x;j++)
            {
                if( g_lpTempBuf[j+(i*w)] > max1 )
                {
                    max3=max2;
                    max2=max1;
                    max1=g_lpTempBuf[j+(i*w)];
                    continue;
                }
                if( g_lpTempBuf[j+(i*w)] > max2 )
                {
                    max3=max2;
                    max2=g_lpTempBuf[j+(i*w)];
                    continue;
                }
                if( g_lpTempBuf[j+ (i*w)] > max3 )
                {
                    max3=g_lpTempBuf[j+ (i*w)];
                    continue;
                }
            }
        }
        for(i=chk_start_y;i<chk_end_y;i++)
        {
            for(j=chk_start_x;j<chk_end_x;j++)
            {
                if( g_lpTempBuf[j+ (i*w)] < min1 )
                {
                    min3=min2;
                    min2=min1;
                    min1=g_lpTempBuf[j+ (i*w)];
                    continue;
                }
                if( g_lpTempBuf[j+ (i*w)] < min2 )
                {
                    min3=min2;
                    min2=g_lpTempBuf[j+ (i*w)];
                    continue;
                }
                if( g_lpTempBuf[j+ (i*w)] < min3 )
                {
                    min3=g_lpTempBuf[j+ (i*w)];
                    continue;
                }
            }
        }
        
        // adjust the contrast
        if(max3-min3>75)
        {
            a=230.0/(max3-min3);
            b=230-a*max3;
        }
        else
        {
            a=1.0;
            b=0.0;
        }
        
        for(i=0;i<w*h;i++)
        {
            tmp=(int)((float)g_lpTempBuf[i]*a+b);
            if(tmp>255)
                g_lpTempBuf[i]=255;
            else if(tmp<0)
                g_lpTempBuf[i]=0;
            else
                g_lpTempBuf[i]=(unsigned char)tmp;
            
            //g_lpTempBuf[i]= (float)g_lpTempBuf[i]*1.67f-104;
        }
    }
}

#pragma mark - File Manipulation
void write_bmp(char *filename, uint8_t *red_buf, uint8_t *green_buf, uint8_t *blue_buf, int WIDTH, int HEIGHT)
{
    unsigned int headers[13];
    FILE * outfile;
    int extrabytes;
    int paddedsize;
    int x; int y; int n;
    int red, green, blue;
    
    extrabytes = 4 - ((WIDTH * 3) % 4);                 // How many bytes of padding to add to each
    // horizontal line - the size of which must
    // be a multiple of 4 bytes.
    if (extrabytes == 4)
        extrabytes = 0;
    
    paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;
    
    // Headers...
    // Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".
    
    headers[0]  = paddedsize + 54;      // bfSize (whole file size)
    headers[1]  = 0;                    // bfReserved (both)
    headers[2]  = 54;                   // bfOffbits
    headers[3]  = 40;                   // biSize
    headers[4]  = WIDTH;  // biWidth
    headers[5]  = HEIGHT; // biHeight
    
    // Would have biPlanes and biBitCount in position 6, but they're shorts.
    // It's easier to write them out separately (see below) than pretend
    // they're a single int, especially with endian issues...
    
    headers[7]  = 0;                    // biCompression
    headers[8]  = paddedsize;           // biSizeImage
    headers[9]  = 0;                    // biXPelsPerMeter
    headers[10] = 0;                    // biYPelsPerMeter
    headers[11] = 0;                    // biClrUsed
    headers[12] = 0;                    // biClrImportant
    
    outfile = fopen(filename, "wb");
    
    //
    // Headers begin...
    // When printing ints and shorts, we write out 1 character at a time to avoid endian issues.
    //
    
    fprintf(outfile, "BM");
    
    for (n = 0; n <= 5; n++)
    {
        fprintf(outfile, "%c", headers[n] & 0x000000FF);
        fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
        fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
        fprintf(outfile, "%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
    }
    
    // These next 4 characters are for the biPlanes and biBitCount fields.
    
    fprintf(outfile, "%c", 1);
    fprintf(outfile, "%c", 0);
    fprintf(outfile, "%c", 24);
    fprintf(outfile, "%c", 0);
    
    for (n = 7; n <= 12; n++)
    {
        fprintf(outfile, "%c", headers[n] & 0x000000FF);
        fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
        fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
        fprintf(outfile, "%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
    }
    
    //
    // Headers done, now write the data...
    //
    
    for (y = HEIGHT - 1; y >= 0; y--)     // BMP image format is written from bottom to top...
    {
        for (x = 0; x <= WIDTH - 1; x++)
        {
            red   = red_buf[y * WIDTH + x];
            green = green_buf[y * WIDTH + x];
            blue  = blue_buf[y * WIDTH + x];
            
            fprintf(outfile, "%c", blue);
            fprintf(outfile, "%c", green);
            fprintf(outfile, "%c", red);
        }
        if (extrabytes)      // See above - BMP lines must be of lengths divisible by 4.
        {
            for (n = 1; n <= extrabytes; n++)
            {
                fprintf(outfile, "%c", 0);
            }
        }
    }
    
    fclose(outfile);
    return;
}
