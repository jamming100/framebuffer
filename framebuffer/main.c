#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
unsigned int *frameBuffer = 0;
 
//打印fb驱动中fix结构信息，注：在fb驱动加载后，fix结构不可被修改。
void printFixedInfo ()
{
   printf ("Fixed screen info:\n"
                        "\tid: %s\n"
                        "\tsmem_start:0x%lx\n"
                        "\tsmem_len:%d\n"
                        "\ttype:%d\n"
                        "\ttype_aux:%d\n"
                        "\tvisual:%d\n"
                        "\txpanstep:%d\n"
                        "\typanstep:%d\n"
                        "\tywrapstep:%d\n"
                        "\tline_length: %d\n"
                        "\tmmio_start:0x%lx\n"
                        "\tmmio_len:%d\n"
                        "\taccel:%d\n"
           "\n",
           finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
           finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
           finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
           finfo.mmio_len, finfo.accel);
}
 
//打印fb驱动中var结构信息，注：fb驱动加载后，var结构可根据实际需要被重置
void printVariableInfo ()
{
   printf ("Variable screen info:\n"
                        "\txres:%d\n"
                        "\tyres:%d\n"
                        "\txres_virtual:%d\n"
                        "\tyres_virtual:%d\n"
                        "\tyoffset:%d\n"
                        "\txoffset:%d\n"
                        "\tbits_per_pixel:%d\n"
                        "\tgrayscale:%d\n"
                        "\tred: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\tgreen: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\tblue: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\ttransp: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\tnonstd:%d\n"
                        "\tactivate:%d\n"
                        "\theight:%d\n"
                        "\twidth:%d\n"
                        "\taccel_flags:0x%x\n"
                        "\tpixclock:%d\n"
                        "\tleft_margin:%d\n"
                        "\tright_margin: %d\n"
                        "\tupper_margin:%d\n"
                        "\tlower_margin:%d\n"
                        "\thsync_len:%d\n"
                        "\tvsync_len:%d\n"
                        "\tsync:%d\n"
                       "\tvmode:%d\n"
           "\n",
           vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
           vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel,
           vinfo.grayscale, vinfo.red.offset, vinfo.red.length,
            vinfo.red.msb_right,vinfo.green.offset, vinfo.green.length,
           vinfo.green.msb_right, vinfo.blue.offset, vinfo.blue.length,
           vinfo.blue.msb_right, vinfo.transp.offset, vinfo.transp.length,
           vinfo.transp.msb_right, vinfo.nonstd, vinfo.activate,
           vinfo.height, vinfo.width, vinfo.accel_flags, vinfo.pixclock,
           vinfo.left_margin, vinfo.right_margin, vinfo.upper_margin,
           vinfo.lower_margin, vinfo.hsync_len, vinfo.vsync_len,
           vinfo.sync, vinfo.vmode);
}

//画大小为width*height的同色矩阵，5reds+6greens+5blues
void drawRectRGBA8888 (int x0, int y0, int width,int height, unsigned int color)
{
   const int bytesPerPixel = 4;
   const int stride = finfo.line_length / bytesPerPixel;
   const unsigned int colorRGBA = color;
 
   unsigned int *dest = (unsigned int *) (frameBuffer)+ (y0 + vinfo.yoffset) * stride + (x0 +vinfo.xoffset);
 
   int x, y;
   for (y = 0; y < height; ++y)
    {
       for (x = 0; x < width; ++x)
       {
           dest[x] = colorRGBA;
       }
       dest += stride;
    }
}

void draw_back(unsigned int *pfb, unsigned int width, unsigned int height, unsigned int color)
{
    unsigned int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            *(pfb + y * width + x) = color;
        }
    }
}

int main (int argc, char **argv)
{
   const char *devfile = "/dev/fb0";
   long int screensize = 0;
   int fbFd = 0;
 
 
 
   /* Open the file for reading and writing */
   fbFd = open (devfile, O_RDWR);
   if (fbFd == -1)
    {
       perror ("Error: cannot open framebuffer device");
       exit (1);
    }
 
   //获取finfo信息并显示
   if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
       perror ("Error reading fixed information");
       exit (2);
    }
   printFixedInfo ();
   //获取vinfo信息并显示
   if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
       perror ("Error reading variable information");
       exit (3);
    }

    vinfo.activate = 1;
    vinfo.width = vinfo.xres;
    vinfo.height = vinfo.yres;

   if (ioctl (fbFd, FBIOPUT_VSCREENINFO, &vinfo) == -1)
    {
       perror ("Error set variable information");
       exit (3);
    }

   printVariableInfo ();
 

   screensize = finfo.smem_len;//fb的缓存长度
 

    frameBuffer =(unsigned int *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
          perror ("Error: Failed to map framebuffer device to memory");
           exit (4);
    }
  	
        drawRectRGBA8888(50,50,400,500,0xff0000ff);
        //draw_back(frameBuffer, vinfo.xres_virtual, vinfo.yres_virtual, 0xffff0000);
	//drawRect_rgb16 (vinfo.xres *3 / 8, vinfo.yres * 3 / 8,vinfo.xres / 4, vinfo.yres / 4,0xff00ff00);//实现画矩形
 

    sleep (2);
    printf (" Done.\n");
 
    munmap (frameBuffer, screensize);   //解除内存映射，与mmap对应
 
    close (fbFd);
    return 0;
}

