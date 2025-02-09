#include "../lib/Config/Debug.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
//#include <libconfig.h> https://habr.com/ru/articles/148948/
//#include "config.h"
#include "../lib/Config/DEV_Config.h"
#include "display.h"
#include "../lib/GUI/GUI_BMPfile.h"

#include <math.h>

#include <stdlib.h>     //exit()
#include <signal.h>     //signal()

#define Enhance false
char  const ImgPath[] = "/media/share/photos/";

UWORD VCOM = 2510;
IT8951_Dev_Info Dev_Info = { 0, 0 };
UWORD Panel_Width;
UWORD Panel_Height;
UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 0;	//0: no rotate, no mirror
//1: no rotate, horizontal mirror, for 10.3inch
//2: no totate, horizontal mirror, for 5.17inch
//3: no rotate, no mirror, isColor, for 6inch color

void  Handler(int signo) {
    Debug("\r\nHandler:exit\r\n");
    if (Refresh_Frame_Buf != NULL) {
        free(Refresh_Frame_Buf);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf = NULL;
    }
    if (Panel_Frame_Buf != NULL) {
        free(Panel_Frame_Buf);
        Debug("free Panel_Frame_Buf\r\n");
        Panel_Frame_Buf = NULL;
    }
    if (Panel_Area_Frame_Buf != NULL) {
        free(Panel_Area_Frame_Buf);
        Debug("free Panel_Area_Frame_Buf\r\n");
        Panel_Area_Frame_Buf = NULL;
    }
    if (bmp_src_buf != NULL) {
        free(bmp_src_buf);
        Debug("free bmp_src_buf\r\n");
        bmp_src_buf = NULL;
    }
    if (bmp_dst_buf != NULL) {
        free(bmp_dst_buf);
        Debug("free bmp_dst_buf\r\n");
        bmp_dst_buf = NULL;
    }
    if (Dev_Info.Panel_W != 0) {
        Debug("Going to sleep\r\n");
        EPD_IT8951_Sleep();
    }
    DEV_Module_Exit();
    exit(0);
}

#define EXT ".bmp"
#define ELEN 2

UBYTE startShow(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr, UBYTE BitsPerPixel) {
    DIR* dp = opendir(ImgPath);
    int i = 0;
    if (!dp) {
        Debug("Can't open current directory!\n");
        return 1;
    }

    struct dirent* de;
    while ((de = readdir(dp))) {

        size_t nlen = strlen(de->d_name);

        if (nlen > ELEN && strcmp((de->d_name) + nlen - 4, EXT) == 0) {
            //Debug("%s\n", de->d_name);
            char fileName[255] = "";
            strcpy(fileName, ImgPath);
            strcat(fileName, de->d_name);
            Display_BMP(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_4, fileName);
            //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, GC16_Mode);
        }
        i++;
    }

    closedir(dp);

    return 0;
}

int main(int argc, char* argv[])
{
    //Exception handling:ctrl + c
    signal(SIGINT, Handler);

    if (argc < 2) {
        Debug("Please input VCOM value on FPC cable!\r\n");
        Debug("Example: sudo ./epd -2.51\r\n");
        exit(1);
    }
    if (argc != 3) {
        Debug("Please input e-Paper display mode!\r\n");
        Debug("Example: sudo ./epd -2.51 0 or sudo ./epd -2.51 1\r\n");
        Debug("Now, 10.3 inch glass panle is mode1, else is mode0\r\n");
        Debug("If you don't know what to type in just type 0 \r\n");
        exit(1);
    }

    //Init the BCM2835 Device
    if (DEV_Module_Init() != 0) {
        return -1;
    }

    double temp;
    sscanf(argv[1], "%lf", &temp);
    VCOM = (UWORD)(fabs(temp) * 1000);
    Debug("VCOM value:%d\r\n", VCOM);
    sscanf(argv[2], "%d", &epd_mode);
    Debug("Display mode:%d\r\n", epd_mode);
    Dev_Info = EPD_IT8951_Init(VCOM);

#if(Enhance)
    Debug("Attention! Enhanced driving ability, only used when the screen is blurred\r\n");
    Enhance_Driving_Capability();
#endif

    //get some important info from Dev_Info structure
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    char* LUT_Version = (char*)Dev_Info.LUT_Version;
    if (strcmp(LUT_Version, "M641") == 0) {
        //6inch e-Paper HAT(800,600), 6inch HD e-Paper HAT(1448,1072), 6inch HD touch e-Paper HAT(1448,1072)
        A2_Mode = 4;
        Four_Byte_Align = true;
    }
    else if (strcmp(LUT_Version, "M841_TFAB512") == 0) {
        //Another firmware version for 6inch HD e-Paper HAT(1448,1072), 6inch HD touch e-Paper HAT(1448,1072)
        A2_Mode = 6;
        Four_Byte_Align = true;
    }
    else if (strcmp(LUT_Version, "M841") == 0) {
        //9.7inch e-Paper HAT(1200,825)
        A2_Mode = 6;
    }
    else if (strcmp(LUT_Version, "M841_TFA2812") == 0) {
        //7.8inch e-Paper HAT(1872,1404)
        A2_Mode = 6;
    }
    else if (strcmp(LUT_Version, "M841_TFA5210") == 0) {
        //10.3inch e-Paper HAT(1872,1404)
        A2_Mode = 6;
    }
    else {
        //default set to 6 as A2 Mode
        A2_Mode = 6;
    }
    Debug("A2 Mode:%d\r\n", A2_Mode);

    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    //Show a bmp file
    //1bp use A2 mode by default, before used it, refresh the screen with WHITE
    //Display_BMP(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_4);

    int ii = 0;
    for (;;) {
        ii++;
        startShow(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_4);
    }

    //We recommended refresh the panel to white color before storing in the warehouse.
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    //EPD_IT8951_Standby();
    EPD_IT8951_Sleep();

    //In case RPI is transmitting image in no hold mode, which requires at most 10s
    DEV_Delay_ms(5000);

    DEV_Module_Exit();
    return 0;
}