/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>


#include "impfuncs_old.h"

#define TAG "Sample-Common"


struct chn_conf chn[2] = {
        {
                .index = CH0_INDEX,
                .enable = CHN0_EN,
                .fs_chn_attr = {
                        .pixFmt = PIX_FMT_NV12,
                        .outFrmRateNum = SENSOR_FRAME_RATE_NUM,
                        .outFrmRateDen = SENSOR_FRAME_RATE_DEN,
                        .nrVBs = 3,
                        .type = FS_PHY_CHANNEL,

                        .crop.enable = CROP_EN,
                        .crop.top = 0,
                        .crop.left = 0,
                        .crop.width = SENSOR_WIDTH,
                        .crop.height = SENSOR_HEIGHT,

                        .scaler.enable = 0,

                        .picWidth = SENSOR_WIDTH,
                        .picHeight = SENSOR_HEIGHT,
                },
                .framesource_chn =    {DEV_ID_FS, 0, 0},
                .imp_encoder = {DEV_ID_ENC, 0, 0},
        },
        {
                .index = CH1_INDEX,
                .enable = CHN1_EN,
                .fs_chn_attr = {
                        .pixFmt = PIX_FMT_NV12,
                        .outFrmRateNum = SENSOR_FRAME_RATE_NUM,
                        .outFrmRateDen = SENSOR_FRAME_RATE_DEN,
                        .nrVBs = 3,
                        .type = FS_PHY_CHANNEL,

                        .crop.enable = CROP_EN,
                        .crop.top = 0,
                        .crop.left = 0,
                        .crop.width = SENSOR_WIDTH,
                        .crop.height = SENSOR_HEIGHT,

                        .scaler.enable = 1,
                        .scaler.outwidth = SENSOR_WIDTH_SECOND,
                        .scaler.outheight = SENSOR_HEIGHT_SECOND,

                        .picWidth = SENSOR_WIDTH_SECOND,
                        .picHeight = SENSOR_HEIGHT_SECOND,
                },
                .framesource_chn =    {DEV_ID_FS, 1, 0},
                .imp_encoder = {DEV_ID_ENC, 1, 0},
        },
};


IMPSensorInfo sensor_info;

int sample_system_init(impParams params) {

    chn[0].fs_chn_attr.crop.width = width;
    chn[0].fs_chn_attr.crop.height = height;
    chn[0].fs_chn_attr.picWidth = width;
    chn[0].fs_chn_attr.picHeight = height;



    int ret = 0;

    memset(&sensor_info, 0, sizeof(IMPSensorInfo));
    memcpy(sensor_info.name, SENSOR_NAME, sizeof(SENSOR_NAME));
    sensor_info.cbus_type = SENSOR_CUBS_TYPE;
    memcpy(sensor_info.i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
    sensor_info.i2c.addr = SENSOR_I2C_ADDR;

    IMP_LOG_ERR(TAG, "Imp Log %d\n",IMP_Log_Get_Option());
    //IMP_Log_Set_Option()

    ret = IMP_ISP_Open();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to open ISP\n");
        return -1;
    }


    ret = IMP_ISP_AddSensor(&sensor_info);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to AddSensor\n");
        return -1;
    }

    ret = IMP_ISP_EnableSensor();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
        return -1;
    }


    ret = IMP_System_Init();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
        return -1;
    }




    /* enable turning, to debug graphics */

    ret = IMP_ISP_EnableTuning();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
        return -1;
    }


    ret = IMP_ISP_Tuning_SetWDRAttr(IMPISP_TUNING_OPS_MODE_DISABLE);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to set WDR\n");
        return -1;
    }




    //IMP_LOG_ERR(TAG, "ImpSystemInit success\n");

    return 0;
}

int sample_system_exit() {
    int ret = 0;

    IMP_LOG_ERR(TAG, "sample_system_exit start\n");


    IMP_System_Exit();

    ret = IMP_ISP_DisableSensor();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
        return -1;
    }

    ret = IMP_ISP_DelSensor(&sensor_info);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to AddSensor\n");
        return -1;
    }

    ret = IMP_ISP_DisableTuning();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
        return -1;
    }

    if (IMP_ISP_Close()) {
        IMP_LOG_ERR(TAG, "failed to open ISP\n");
        return -1;
    }

    IMP_LOG_ERR(TAG, " sample_system_exit success\n");

    return 0;
}

int sample_framesource_streamon() {

    int out_pipe[2];
    int saved_stdout;
    saved_stdout = dup(STDOUT_FILENO);
    pipe(out_pipe);
    dup2(out_pipe[1], STDOUT_FILENO);


    int ret = 0, i = 0;
    /* Enable channels */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_FrameSource_EnableChn(chn[i].index);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
                return -1;
            }
        }
    }
    fflush(stdout);

    dup2(saved_stdout, STDOUT_FILENO);
    return 0;
}

int sample_framesource_streamoff() {
    int ret = 0, i = 0;
    /* Enable channels */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_FrameSource_DisableChn(chn[i].index);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
                return -1;
            }
        }
    }
    return 0;
}

int sample_framesource_init() {
    int i, ret;

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
                return -1;
            }

            ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", chn[i].index);
                return -1;
            }
        }
    }

    return 0;
}

int sample_framesource_exit() {
    int ret, i;

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            /*Destroy channel i*/
            ret = IMP_FrameSource_DestroyChn(i);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn() error: %d\n", ret);
                return -1;
            }
        }
    }
    return 0;
}

int sample_jpeg_init() {
    int i, ret;
    IMPEncoderAttr *enc_attr;
    IMPEncoderCHNAttr channel_attr;
    IMPFSChnAttr *imp_chn_attr_tmp;

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
            enc_attr = &channel_attr.encAttr;
            enc_attr->enType = PT_JPEG;
            enc_attr->bufSize = 0;
            enc_attr->profile = 0;
            enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
            enc_attr->picHeight = imp_chn_attr_tmp->picHeight;

            /* Create Channel */
            ret = IMP_Encoder_CreateChn(2 + chn[i].index, &channel_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
                            chn[i].index, ret);
                return -1;
            }

            /* Resigter Channel */
            ret = IMP_Encoder_RegisterChn(i, 2 + chn[i].index);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
                            chn[i].index, ret);
                return -1;
            }
        }
    }

    return 0;
}

int sample_encoder_init() {
    int i, ret;
    IMPEncoderAttr *enc_attr;
    IMPEncoderRcAttr *rc_attr;
    IMPFSChnAttr *imp_chn_attr_tmp;
    IMPEncoderCHNAttr channel_attr;

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
            enc_attr = &channel_attr.encAttr;
            enc_attr->enType = PT_H264;
            enc_attr->bufSize = 0;
            enc_attr->profile = 0;
            enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
            enc_attr->picHeight = imp_chn_attr_tmp->picHeight;
            rc_attr = &channel_attr.rcAttr;



			rc_attr->rcMode = ENC_RC_MODE_H264CBR;
			rc_attr->attrH264Cbr.outFrmRate.frmRateNum = imp_chn_attr_tmp->outFrmRateNum;
			rc_attr->attrH264Cbr.outFrmRate.frmRateDen = imp_chn_attr_tmp->outFrmRateDen;
			rc_attr->attrH264Cbr.maxGop = 2 * rc_attr->attrH264Cbr.outFrmRate.frmRateNum / rc_attr->attrH264Cbr.outFrmRate.frmRateDen;
			rc_attr->attrH264Cbr.outBitRate = 2000 * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrH264Cbr.maxQp = 38;
			rc_attr->attrH264Cbr.minQp = 15;
			rc_attr->attrH264Cbr.maxFPS = 100;
			rc_attr->attrH264Cbr.minFPS = 1;
			rc_attr->attrH264Cbr.IBiasLvl = 2;
			rc_attr->attrH264Cbr.FrmQPStep = 3;
			rc_attr->attrH264Cbr.GOPQPStep = 15;
			rc_attr->attrH264Cbr.AdaptiveMode = false;
			rc_attr->attrH264Cbr.GOPRelation = false;

            /*
            rc_attr->attrH264FrmUsed.enable = true;
            rc_attr->attrH264FrmUsed.frmUsedMode = ENC_FRM_REUSED ;
            rc_attr->attrH264FrmUsed.frmUsedTimes = 50;
            */


            /*
            rc_attr->rcMode = ENC_RC_MODE_H264VBR;
            rc_attr->attrH264Vbr.outFrmRate.frmRateNum = imp_chn_attr_tmp->outFrmRateNum;
            rc_attr->attrH264Vbr.outFrmRate.frmRateDen = imp_chn_attr_tmp->outFrmRateDen;
            rc_attr->attrH264Vbr.maxGop =
                    1 * rc_attr->attrH264Vbr.outFrmRate.frmRateNum / rc_attr->attrH264Vbr.outFrmRate.frmRateDen;
            rc_attr->attrH264Vbr.maxQp = 38;
            rc_attr->attrH264Vbr.minQp = 15;
            rc_attr->attrH264Vbr.staticTime = 1;
            rc_attr->attrH264Vbr.maxBitRate =
                    100 * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1920 * 1080);
            rc_attr->attrH264Vbr.changePos = 50;
            rc_attr->attrH264Vbr.FrmQPStep = 3;
            rc_attr->attrH264Vbr.GOPQPStep = 15;
            rc_attr->attrH264FrmUsed.enable = 1;
             */


            ret = IMP_Encoder_CreateChn(chn[i].index, &channel_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", i);
                return -1;
            }

            ret = IMP_Encoder_RegisterChn(chn[i].index, chn[i].index);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n",
                            chn[i].index, chn[i].index, ret);
                return -1;
            }
        }
    }

    return 0;
}

static int encoder_chn_exit(int encChn) {
    int ret;
    IMPEncoderCHNStat chn_stat;
    ret = IMP_Encoder_Query(encChn, &chn_stat);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n",
                    encChn, ret);
        return -1;
    }

    if (chn_stat.registered) {
        ret = IMP_Encoder_UnRegisterChn(encChn);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n",
                        encChn, ret);
            return -1;
        }

        ret = IMP_Encoder_DestroyChn(encChn);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n",
                        encChn, ret);
            return -1;
        }
    }

    return 0;
}

int sample_encoder_exit(void) {
    int ret;

    ret = encoder_chn_exit(ENC_H264_CHANNEL);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Encoder Channel %d exit  error: %d\n",
                    ENC_H264_CHANNEL, ret);
        return -1;
    }

    ret = encoder_chn_exit(ENC_JPEG_CHANNEL);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Encoder Channel %d exit  error: %d\n",
                    ENC_JPEG_CHANNEL, ret);
        return -1;
    }

    ret = IMP_Encoder_DestroyGroup(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyGroup(0) error: %d\n", ret);
        return -1;
    }

    return 0;
}

IMPRgnHandle *sample_osd_init(int grpNum) {
    int ret;
    IMPRgnHandle *prHander;
    IMPRgnHandle rHanderFont;
    IMPRgnHandle rHanderLogo;
    IMPRgnHandle rHanderCover;
    IMPRgnHandle rHanderRect;

    prHander = malloc(4 * sizeof(IMPRgnHandle));
    if (prHander <= 0) {
        IMP_LOG_ERR(TAG, "malloc() error !\n");
        return NULL;
    }

    rHanderFont = IMP_OSD_CreateRgn(NULL);
    if (rHanderFont == INVHANDLE) {
        IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn TimeStamp error !\n");
        return NULL;
    }

    rHanderLogo = IMP_OSD_CreateRgn(NULL);
    if (rHanderLogo == INVHANDLE) {
        IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Logo error !\n");
        return NULL;
    }

    rHanderCover = IMP_OSD_CreateRgn(NULL);
    if (rHanderCover == INVHANDLE) {
        IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Cover error !\n");
        return NULL;
    }

    rHanderRect = IMP_OSD_CreateRgn(NULL);
    if (rHanderRect == INVHANDLE) {
        IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Rect error !\n");
        return NULL;
    }


    ret = IMP_OSD_RegisterRgn(rHanderFont, grpNum, NULL);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
        return NULL;
    }

    ret = IMP_OSD_RegisterRgn(rHanderLogo, grpNum, NULL);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
        return NULL;
    }

    ret = IMP_OSD_RegisterRgn(rHanderCover, grpNum, NULL);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
        return NULL;
    }

    ret = IMP_OSD_RegisterRgn(rHanderRect, grpNum, NULL);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
        return NULL;
    }


    IMPOSDRgnAttr rAttrFont;
    memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
    rAttrFont.type = OSD_REG_PIC;
    rAttrFont.rect.p0.x = 10;
    rAttrFont.rect.p0.y = 10;
    rAttrFont.rect.p1.x =
            rAttrFont.rect.p0.x + 20 * OSD_REGION_WIDTH - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
    rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + OSD_REGION_HEIGHT - 1;
#ifdef SUPPORT_RGB555LE
    rAttrFont.fmt = PIX_FMT_RGB555LE;
#else
    rAttrFont.fmt = PIX_FMT_BGRA;
#endif
    rAttrFont.data.picData.pData = NULL;
    ret = IMP_OSD_SetRgnAttr(rHanderFont, &rAttrFont);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr TimeStamp error !\n");
        return NULL;
    }

    IMPOSDGrpRgnAttr grAttrFont;

    if (IMP_OSD_GetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
        return NULL;

    }
    memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
    grAttrFont.show = 0;

    /* Disable Font global alpha, only use pixel alpha. */
    grAttrFont.gAlphaEn = 1;
    grAttrFont.fgAlhpa = 0xff;
    grAttrFont.layer = 3;
    if (IMP_OSD_SetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
        return NULL;
    }

    IMPOSDRgnAttr rAttrLogo;
    memset(&rAttrLogo, 0, sizeof(IMPOSDRgnAttr));
    int picw = 100;
    int pich = 100;
    rAttrLogo.type = OSD_REG_PIC;
    rAttrLogo.rect.p0.x = SENSOR_WIDTH - 100;
    rAttrLogo.rect.p0.y = SENSOR_HEIGHT - 100;
    rAttrLogo.rect.p1.x = rAttrLogo.rect.p0.x + picw - 1;     //p0 is start，and p1 well be epual p0+width(or heigth)-1
    rAttrLogo.rect.p1.y = rAttrLogo.rect.p0.y + pich - 1;
    rAttrLogo.fmt = PIX_FMT_BGRA;
    //rAttrLogo.data.picData.pData = logodata_100x100_bgra;
    ret = IMP_OSD_SetRgnAttr(rHanderLogo, &rAttrLogo);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Logo error !\n");
        return NULL;
    }
    IMPOSDGrpRgnAttr grAttrLogo;

    if (IMP_OSD_GetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
        return NULL;

    }
    memset(&grAttrLogo, 0, sizeof(IMPOSDGrpRgnAttr));
    grAttrLogo.show = 0;

    /* Set Logo global alpha to 0x7f, it is semi-transparent. */
    grAttrLogo.gAlphaEn = 1;
    grAttrLogo.fgAlhpa = 0x7f;
    grAttrLogo.layer = 2;

    if (IMP_OSD_SetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
        return NULL;
    }

    IMPOSDRgnAttr rAttrCover;
    memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));
    rAttrCover.type = OSD_REG_COVER;
    rAttrCover.rect.p0.x = SENSOR_WIDTH / 2 - 100;
    rAttrCover.rect.p0.y = SENSOR_HEIGHT / 2 - 100;
    rAttrCover.rect.p1.x = rAttrCover.rect.p0.x + SENSOR_WIDTH / 2 - 1 +
                           50;     //p0 is start，and p1 well be epual p0+width(or heigth)-1
    rAttrCover.rect.p1.y = rAttrCover.rect.p0.y + SENSOR_HEIGHT / 2 - 1 + 50;
    rAttrCover.fmt = PIX_FMT_BGRA;
    rAttrCover.data.coverData.color = OSD_BLACK;
    ret = IMP_OSD_SetRgnAttr(rHanderCover, &rAttrCover);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Cover error !\n");
        return NULL;
    }
    IMPOSDGrpRgnAttr grAttrCover;

    if (IMP_OSD_GetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Cover error !\n");
        return NULL;

    }
    memset(&grAttrCover, 0, sizeof(IMPOSDGrpRgnAttr));
    grAttrCover.show = 0;

    /* Disable Cover global alpha, it is absolutely no transparent. */
    grAttrCover.gAlphaEn = 1;
    grAttrCover.fgAlhpa = 0x7f;
    grAttrCover.layer = 2;
    if (IMP_OSD_SetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
        return NULL;
    }

    IMPOSDRgnAttr rAttrRect;
    memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));

    rAttrRect.type = OSD_REG_RECT;
    rAttrRect.rect.p0.x = 300;
    rAttrRect.rect.p0.y = 300;
    rAttrRect.rect.p1.x = rAttrRect.rect.p0.x + 600 - 1;
    rAttrRect.rect.p1.y = rAttrRect.rect.p0.y + 300 - 1;
    rAttrRect.fmt = PIX_FMT_MONOWHITE;
    rAttrRect.data.lineRectData.color = OSD_GREEN;
    rAttrRect.data.lineRectData.linewidth = 5;
    ret = IMP_OSD_SetRgnAttr(rHanderRect, &rAttrRect);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Rect error !\n");
        return NULL;
    }
    IMPOSDGrpRgnAttr grAttrRect;

    if (IMP_OSD_GetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Rect error !\n");
        return NULL;

    }
    memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
    grAttrRect.show = 0;
    grAttrRect.layer = 1;
    grAttrRect.scalex = 1;
    grAttrRect.scaley = 1;
    if (IMP_OSD_SetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Rect error !\n");
        return NULL;
    }


    ret = IMP_OSD_Start(grpNum);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
        return NULL;
    }

    prHander[0] = rHanderFont;
    prHander[1] = rHanderLogo;
    prHander[2] = rHanderCover;
    prHander[3] = rHanderRect;
    return prHander;
}

int sample_osd_exit(IMPRgnHandle *prHander, int grpNum) {
    int ret;

    ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
    }

    ret = IMP_OSD_ShowRgn(prHander[1], grpNum, 0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Logo error\n");
    }

    ret = IMP_OSD_ShowRgn(prHander[2], grpNum, 0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close cover error\n");
    }

    ret = IMP_OSD_ShowRgn(prHander[3], grpNum, 0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Rect error\n");
    }


    ret = IMP_OSD_UnRegisterRgn(prHander[0], grpNum);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
    }

    ret = IMP_OSD_UnRegisterRgn(prHander[1], grpNum);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn logo error\n");
    }

    ret = IMP_OSD_UnRegisterRgn(prHander[2], grpNum);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Cover error\n");
    }

    ret = IMP_OSD_UnRegisterRgn(prHander[3], grpNum);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Rect error\n");
    }


    IMP_OSD_DestroyRgn(prHander[0]);
    IMP_OSD_DestroyRgn(prHander[1]);
    IMP_OSD_DestroyRgn(prHander[2]);
    IMP_OSD_DestroyRgn(prHander[3]);

    ret = IMP_OSD_DestroyGroup(grpNum);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(0) error\n");
        return -1;
    }
    free(prHander);
    prHander = NULL;

    return 0;
}


