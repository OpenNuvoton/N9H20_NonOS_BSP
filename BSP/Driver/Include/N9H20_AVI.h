/**************************************************************************//**
 * @file     N9H20_AVI.h
 * @version  V3.00
 * @brief    N9H20 series AVI library header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef _AVI_PLAYER_H_
#define _AVI_PLAYER_H_


/*-------------------------------------------------------------------------*
 *  MFL error code table                                                   *
 *-------------------------------------------------------------------------*/
/* general errors */
#define MFL_ERR_NO_MEMORY		0xFFFF8000	/* no memory */
#define MFL_ERR_HARDWARE		0xFFFF8002	/* hardware general error */
#define MFL_ERR_NO_CALLBACK		0xFFFF8004	/* must provide callback function */
#define MFL_ERR_AU_UNSUPPORT	0xFFFF8006	/* not supported audio type */
#define MFL_ERR_VID_UNSUPPORT	0xFFFF8008	/* not supported video type */
#define MFL_ERR_OP_UNSUPPORT	0xFFFF800C	/* unsupported operation */
#define MFL_ERR_PREV_UNSUPPORT	0xFFFF800E	/* preview of this media type was not supported or not enabled */
#define MFL_ERR_FUN_USAGE		0xFFFF8010	/* incorrect function call parameter */
#define MFL_ERR_RESOURCE_MEM	0xFFFF8012	/* memory is not enough to play/record a media file */
/* stream I/O errors */
#define MFL_ERR_FILE_OPEN		0xFFFF8020	/* cannot open file */
#define MFL_ERR_FILE_TEMP		0xFFFF8022	/* temporary file access failure */
#define MFL_ERR_STREAM_IO		0xFFFF8024	/* stream access error */
#define MFL_ERR_STREAM_INIT		0xFFFF8026	/* stream was not opened */
#define MFL_ERR_STREAM_EOF		0xFFFF8028	/* encounter EOF of file */
#define MFL_ERR_STREAM_SEEK		0xFFFF802A	/* stream seek error */
#define MFL_ERR_STREAM_TYPE		0xFFFF802C	/* incorrect stream type */
#define MFL_ERR_STREAM_METHOD	0xFFFF8030	/* missing stream method */
#define MFL_ERR_STREAM_MEMOUT	0xFFFF8032	/* recorded data has been over the application provided memory buffer */
#define MFL_INVALID_BITSTREAM	0xFFFF8034	/* invalid audio/video bitstream format */
/* AVI errors */
#define MFL_ERR_AVI_FILE		0xFFFF8080	/* Invalid AVI file format */
#define MFL_ERR_AVI_VID_CODEC	0xFFFF8081	/* AVI unsupported video codec type */
#define MFL_ERR_AVI_AU_CODEC	0xFFFF8082  /* AVI unsupported audio codec type */
#define MFL_ERR_AVI_CANNOT_SEEK	0xFFFF8083  /* The AVI file is not fast-seekable */
#define MFL_ERR_AVI_SIZE        0xFFFF8080  /* Exceed estimated size */
/* MP3 errors */
#define MFL_ERR_MP3_FORMAT		0xFFFF80D0	/* incorrect MP3 frame format */
#define MFL_ERR_MP3_DECODE		0xFFFF80D2	/* MP3 decode error */
/* hardware engine errors */
#define MFL_ERR_HW_NOT_READY	0xFFFF8100	/* the picture is the same as the last one */
#define MFL_ERR_SHORT_BUFF		0xFFFF8104	/* buffer size is not enough */
#define MFL_ERR_VID_DEC_ERR		0xFFFF8106	/* video decode error */
#define MFL_ERR_VID_DEC_BUSY	0xFFFF8108	/* video decoder is busy */
#define MFL_ERR_VID_ENC_ERR		0xFFFF810A	/* video encode error */
/* other errors */
#define MFL_ERR_UNKNOWN_MEDIA	0xFFFF81E2	/* unknow media type */


typedef enum jv_mode_e
{
	DIRECT_RGB555,				// JPEG output RGB555 directly to VPOST RGB555 frame buffer
	DIRECT_RGB565,				// JPEG output RGB565 directly to VPOST RGB565 frame buffer
	DIRECT_RGB888,				// JPEG output RGB888 directly to VPOST RGB888 frame buffer
	DIRECT_YUV422,				// JPEG output YUV422 directly to VPOST YUV422 frame buffer	
	PLANAR_YUV420				// JPEG output Planar YUV420
				
	//SW_YUV422_TO_RGB565,		// JPEG output packet YUV422, software convert to RGB565 for VPOST
	//OVERLAY_YUV422_RGB555		// JPEG output packet YUV422 buffer to overlay on a RGB555 buffer
	                            //   finally be output to VPOST YUV buffer
}  JV_MODE_E;


typedef enum au_type_e
{
	AU_CODEC_UNKNOWN,
	AU_CODEC_PCM,
	AU_CODEC_IMA_ADPCM,
	AU_CODEC_MP3
}  AU_TYPE_E;


typedef struct avi_info_t
{
	UINT32		uMovieLength;		/* in 1/100 seconds */
	UINT32		uPlayCurTimePos;	/* for playback, the play time position, in 1/100 seconds */
	
	/* audio */
	AU_TYPE_E   eAuCodec;         
	INT			nAuPlayChnNum;		/* 1:Mono, 2:Stero */
	INT 		nAuPlaySRate;		/* audio playback sampling rate */

	/* video */
	UINT32		uVideoFrameRate;	/* only available in MP4/3GP/ASF/AVI files */
	UINT16		usImageWidth;
	UINT16		usImageHeight;
	UINT32		uVidTotalFrames;	/* For playback, it's the total number of video frames. For recording, it's the currently recorded frame number. */
	UINT32		uVidFramesPlayed;	/* Indicate how many video frames have been played */
	UINT32		uVidFramesSkipped;	/* For audio/video sync, some video frames may be dropped. */
}	AVI_INFO_T;

/*-------------------------------------------------------------------------*
 *  AVI playback control enumeration                                       *
 *-------------------------------------------------------------------------*/
typedef enum play_ctrl_e
{
	PLAY_CTRL_FF = 0,
	PLAY_CTRL_FB,
	PLAY_CTRL_FS,
	PLAY_CTRL_PAUSE,
	PLAY_CTRL_RESUME,
	PLAY_CTRL_STOP,
	PLAY_CTRL_SPEED
} PLAY_CTRL_E;

/*-------------------------------------------------------------------------*
 *  MFL ID3 tag information (stored in movie information)                  *
 *-------------------------------------------------------------------------*/
typedef struct id3_ent_t
{
	INT 	nLength;			/* tag length */
	CHAR 	sTag[128];			/* tag content */
	BOOL 	bIsUnicode;			/* 1: is unicode, 0: is ASCII */
}	ID3_ENT_T; 

typedef struct id3_pic_t
{
	BOOL       bHasPic;			/* 1: has picture, 0: has no picture in ID3 tag */
	CHAR       cType;
	ID3_ENT_T  tTitle;			
	INT        nPicOffset;		/* picture offset in file */
	INT        nPicSize;		/* picture size */
	struct id3_pic_t  *ptNextPic;	/* always NULL in current version */
}	ID3_PIC_T;

typedef struct id3_tag_t
{
	ID3_ENT_T  	tTitle;
	ID3_ENT_T  	tArtist;
	ID3_ENT_T  	tAlbum;
	ID3_ENT_T  	tComment;

	CHAR 		szYear[16];
	CHAR 		szTrack[16];
	CHAR 		szGenre[16];
	CHAR    	cVersion;		/* reversion of ID3v2. ID3v2.2.0 = 0x20, ID3v2.3.0 = 0x30  */
								/* reversion of ID3v1. ID3v1.0 = 0x00, ID3v1.1 = 0x10 */
	ID3_PIC_T  	tPicture;
}	ID3_TAG_T;


/*-------------------------------------------------------------------------*
 *  Audio sampling rate enumeration                                        *
 *-------------------------------------------------------------------------*/
typedef enum au_srate_e
{
	AU_SRATE_8000 = 8000,
	AU_SRATE_11025 = 11025,
	AU_SRATE_12000 = 12000,
	AU_SRATE_16000 = 16000,
	AU_SRATE_22050 = 22050,
	AU_SRATE_24000 = 24000,
	AU_SRATE_32000 = 32000,
	AU_SRATE_44100 = 44100,
	AU_SRATE_48000 = 48000
} AU_SRATE_E;

/*-------------------------------------------------------------------------*
 *  Physical audio record device type enumeration                          *
 *-------------------------------------------------------------------------*/
typedef enum au_rec_dev_e
{
	MFL_REC_AC97 = 0,
	MFL_REC_I2S = 1,
	MFL_REC_UDA1345TS = 2,
	MFL_REC_ADC = 3,
	MFL_REC_W5691 = 7,
	MFL_REC_WM8753 = 8,
	MFL_REC_WM8751 = 9,
	MFL_REC_WM8978 = 10,
	MFL_REC_AK4569 = 14,
	MFL_REC_TIAIC31 = 15,
	MFL_REC_WM8731 = 16
} AU_REC_DEV_E;

/*-------------------------------------------------------------------------*
 *  Physical audio playback device type enumeration                        *
 *-------------------------------------------------------------------------*/
typedef enum au_play_dev_e
{
	MFL_PLAY_AC97 = 0,
	MFL_PLAY_I2S = 1,
	MFL_PLAY_UDA1345TS = 2,
	MFL_PLAY_DAC = 4,
	MFL_PLAY_MA3 = 5,
	MFL_PLAY_MA5 = 6,
	MFL_PLAY_W5691 = 7,
	MFL_PLAY_WM8753 = 8,
	MFL_PLAY_WM8751 = 9,
	MFL_PLAY_WM8978 = 10,
	MFL_PLAY_MA5I = 11,
	MFL_PLAY_MA5SI = 12,
	MFL_PLAY_W56964 = 13,
	MFL_PLAY_AK4569 = 14,
	MFL_PLAY_TIAIC31 = 15,
	MFL_PLAY_WM8731 = 16
} AU_PLAY_DEV_E;

/*-------------------------------------------------------------------------*
 *  MFL data stream function set                                           *
 *-------------------------------------------------------------------------*/
struct stream_t;

typedef struct strm_fun
{
	INT (*open)(struct stream_t *ptStream, CHAR *suFileName, CHAR *szAsciiName, 
	             UINT8 *pbMemBase, UINT32 ulMemLen, INT access);
	INT (*is_opened)(struct stream_t *ptStream);
    INT (*seek)(struct stream_t *ptStream, INT nOffset, INT nSeek);
    INT (*get_pos)(struct stream_t *ptStream);
	INT (*close)(struct stream_t *ptStream);
	INT (*read)(struct stream_t *ptStream, UINT8 *pucBuff, INT nCount, BOOL bIsSkip);
	INT (*write)(struct stream_t *ptStream, UINT8 *pucBuff, INT nCount);
	INT (*peek)(struct stream_t *ptStream, UINT8 *pucBuff, INT nCount);
	INT (*get_bsize)(struct stream_t *ptStream);
}	STRM_FUN_T;

/*-------------------------------------------------------------------------*
 *  H/W video codec enumeration                                            *
 *-------------------------------------------------------------------------*/
typedef enum vid_codec_e
{
	MFL_CODEC_H263,
	MFL_CODEC_MPEG4,
	MFL_CODEC_H264,
	MFL_CODEC_JPEG
} VID_CODEC_E;

/*-------------------------------------------------------------------------*
 *  S/W audio codec enumeration                                            *
 *-------------------------------------------------------------------------*/
typedef enum au_codec_e
{
	MFL_CODEC_PCM,
	MFL_CODEC_AMRNB,
	MFL_CODEC_AMRWB,
	MFL_CODEC_AAC,
	MFL_CODEC_AACP,
	MFL_CODEC_EAACP,
	MFL_CODEC_ADPCM,
	MFL_CODEC_MP3,
	MFL_CODEC_UNKNOWN = 1000
} AU_CODEC_E;

/*-------------------------------------------------------------------------*
 *  MFL data stream type enumeration                                       *
 *-------------------------------------------------------------------------*/
typedef enum stream_type_e
{
	MFL_STREAM_MEMORY,
	MFL_STREAM_FILE,
	MFL_STREAM_USER
} STRM_TYPE_E;

/*-------------------------------------------------------------------------*
 *  Media type enumeration                                                 *
 *-------------------------------------------------------------------------*/
typedef enum media_type_e
{
	MFL_MEDIA_MP4,
	MFL_MEDIA_3GP,
	MFL_MEDIA_M4V,
	MFL_MEDIA_AAC,
	MFL_MEDIA_AMR,
	MFL_MEDIA_MP3,
	MFL_MEDIA_ADPCM,
	MFL_MEDIA_WAV,
	MFL_MEDIA_MIDI,
	MFL_MEDIA_AVI,
	MFL_MEDIA_ASF,
	MFL_MEDIA_WMV,
	MFL_MEDIA_WMA,
	MFL_MEDIA_SMAF,
	MFL_MEDIA_M4A,
	MFL_MEDIA_WBKTV,
	MFL_MEDIA_UNKNOWN = 1000
} MEDIA_TYPE_E;

/*-------------------------------------------------------------------------*
 *  MFL movie configuration (playback, record, and preview)                *
 *-------------------------------------------------------------------------*/
typedef struct mv_cfg_t
{
	/* Media and stream */
	MEDIA_TYPE_E	eInMediaType;		/* PLAY   - indicae the type of media to be played */
	MEDIA_TYPE_E	eOutMediaType;		/* RECORD - indicate the type of media to generate */

	STRM_TYPE_E 	eInStrmType;		/* PLAY   - indicae the input stream method */
	STRM_TYPE_E 	eOutStrmType;		/* RECORD - indicate the output stream method */
	
	AU_CODEC_E		eAuCodecType;		/* RECORD - indicate the audio encode type */
	VID_CODEC_E		eVidCodecType;		/* RECORD - indicate the video encode type */

	STRM_FUN_T		*ptStrmUserFun;		/* BOTH   - user defined streaming method */

	CHAR			*suInMediaFile;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*szIMFAscii;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*suInMetaFile;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*szITFAscii;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*suOutMediaFile;	/* RECORD - if out stream type is MFL_STREAM_FILE */
	CHAR			*szOMFAscii;		/* RECORD - if out stream type is MFL_STREAM_FILE */
	CHAR			*suOutMetaFile;		/* RECORD - if out stream type is MFL_STREAM_FILE */
	CHAR			*szOTFAscii;		/* RECORD - if out stream type is MFL_STREAM_FILE */
	
	UINT32			uInMediaMemAddr;	/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uInMediaMemSize;	/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uInMetaMemAddr;		/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uInMetaMemSize;		/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMediaMemAddr;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMediaMemSize;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMetaMemAddr;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMetaMemSize;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */

	BOOL			bUseTempFile;		/* PLAY   - use temporary file? */
	INT				uStartPlaytimePos;	/* PLAY   - On MP3 playback start, just jump to a 
	                                                specific time offset then start playback. The time position unit is 1/100 seconds. */
	BOOL			bStartAndPause;		/* PLAY   - On MP4/3GP playback, start and pause */
	INT				nClipStartTime;		/* CLIP   - media clipping start time offset (in 1/100 secs) */
	INT				nClipEndTime;		/* CLIP   - media clipping end time offset (in 1/100 secs) */
	BOOL			bDoClipVideo;		/* CLIP   - clip video or not */
	
	/* audio */
	BOOL			bIsRecordAudio;		/* RECORD - 1: recording audio, 0: No */
	INT				nAuABRScanFrameCnt;	/* PLAY   - on playback, ask MFL scan how many leading frames
	                                                to evaluate average bit rate. -1 means scan the whole file */
	INT				nAudioPlayVolume;	/* PLAY   - volume of playback, 0~31, 31 is max. */
	INT				nAudioRecVolume;	/* RECORD - volume of playback, 0~31, 31 is max. */
	UINT8			nAuRecChnNum;		/* RECORD - 1: record single channel, Otherwise: record left and right channels */
	BOOL            bIsSbcMode;			/* PLAY   - work with Bluetooth SBC */
	AU_PLAY_DEV_E	eAudioPlayDevice;	/* PLAY   - specify the audio playback audio device */
	AU_REC_DEV_E	eAudioRecDevice;	/* RECORD - specify the audio record device */
	AU_SRATE_E		eAuRecSRate;		/* RECORD - audio record sampling rate */
	UINT32			uAuRecBitRate;		/* RECORD - audio record initial bit rate */
	UINT32			uAuBuffSizeDB;		/* RECORD - audio decoder required buffer size */
	UINT32			uAuRecAvgBitRate;	/* RECORD - audio record average bit rate */
	UINT32			uAuRecMaxBitRate;	/* RECORD - audio record maxmum bit rate */
	
	/* video */
	CHAR			bIsRecordVideo;		/* RECORD - 1: recording video, 0: No */
	INT				nMP4VidMaxSSize;	/* PLAY   - direct MFL should allocate how many memory for MP4/3GP video sample buffer */
	INT				nVidPlayFrate;		/* PLAY   - video playback frame rate, only used in M4V file */
	INT				nVidRecFrate;		/* RECORD - video record frame rate */
	INT				nVidRecIntraIntval;	/* RECORD - video record intra frame interval, -1: one first intra, 0: all intra */
	UINT16			sVidRecWidth;		/* RECORD - width of record image */
	UINT16			sVidRecHeight;		/* RECORD - height of record image */
	UINT32			uVidBuffSizeDB;		/* RECORD - video decoder required buffer size */
	UINT32			uVidRecAvgBitRate;	/* RECORD - video record average bit rate */
	UINT32			uVidRecMaxBitRate;	/* RECORD - video record maxmum bit rate */
	
	/* callback	 */
	VOID (*ap_time)(struct mv_cfg_t *ptMvCfg);
	INT  (*au_sbc_init)(struct mv_cfg_t *ptMvCfg);
	VOID (*au_sbc_reset_buff)(VOID);
	INT  (*au_sbc_encode)(struct mv_cfg_t *ptMvCfg, UINT8 *pucPcmBuff, INT nPcmDataLen);
	BOOL (*au_is_sbc_ready)(VOID);
	INT  (*vid_init_decode)(VID_CODEC_E eDecoder, UINT8 *pucBitStrmBuff, UINT32 uBitStrmSize, BOOL *bIsShortHeader, 
							UINT16 *usImageWidth, UINT16 *usImageHeight);
	INT  (*vid_init_encode)(UINT8 *pucM4VHeader, UINT32 *puHeaderSize,
							BOOL bIsH263, UINT16 usImageWidth, UINT16 usImageHeight);
	INT  (*vid_enc_frame)(PUINT8 *pucFrameBuff, UINT32 *puFrameSize);
	VOID (*vid_rec_frame_done)(UINT8 *pucFrameBuff);
	INT  (*vid_dec_frame)(VID_CODEC_E eDecoder, BOOL bIsSilent, UINT8 *pucFrameBuff, UINT32 *puFrameSize);
	INT  (*vid_dec_state)(VOID);
	VOID (*au_on_start)(struct mv_cfg_t *ptMvCfg);
	VOID (*au_on_stop)(struct mv_cfg_t *ptMvCfg);
	VOID (*the_end)(struct mv_cfg_t *ptMvCfg);

	/*
	 * others - for MFL internal used
	 */
	VOID			*data_mv;			
	VOID			*data_info;			
	INT				data_play_action;
	INT				data_play_param;
	INT				data_rec_action;
	INT				data_rec_param;

	/* for extra parameters */
	VOID			*param1;
	VOID			*param2;
	VOID			*param3;
	VOID			*param4;
}  MV_CFG_T;

/*-------------------------------------------------------------------------*
 *  MFL movie information (run-time and preview)                           *
 *-------------------------------------------------------------------------*/
typedef struct mv_info
{
	UINT32		uInputFileSize;		/* the file size of input media file */
	UINT32		uMovieLength;		/* in 1/100 seconds */
	UINT32		uPlayCurTimePos;	/* for playback, the play time position, in 1/100 seconds */
	UINT32		uCreationTime;
	UINT32		uModificationTime;
	
	/* audio */
	AU_CODEC_E	eAuCodecType;
	UINT32		uAudioLength;		/* in 1/100 seconds */
	INT			nAuRecChnNum;		/* 1: record single channel, Otherwise: record left and right channels */
	AU_SRATE_E 	eAuRecSRate;		/* audio record sampling rate */
	UINT32		uAuRecBitRate;		/* audio record bit rate */
	UINT32		uAuRecMediaLen; 	/* Currently recorded audio data length */
	BOOL		bIsVBR;				/* input audio file is VBR or not */
	INT			nAuPlayChnNum;		/* 1:Mono, 2:Stero */
	AU_SRATE_E 	eAuPlaySRate;		/* audio playback sampling rate */
	UINT32		uAuPlayBitRate;		/* audio playback bit rate */
	UINT32		uAuTotalFrames;		/* For playback, it's the total number of audio frames. For recording, it's the currently recorded frame number. */
	UINT32		uAuFramesPlayed;	/* Indicate how many audio frames have been played */
	UINT32		uAuPlayMediaLen;	/* Indicate how many audio data have been played (bytes) */
	UINT32		uAuMP4BuffSizeDB;	/* MP4 audio decode buffer size (recorded in MP4 file) */
	UINT32		uAuMP4AvgBitRate;	/* MP4 audio average bit rate (recorded in MP4 file) */
	UINT32		uAuMP4MaxBitRate;   /* MP4 audio maximum bit rate (recorded in MP4 file) */

	/* video */
	VID_CODEC_E	eVidCodecType;
	UINT32		uVideoLength;		/* in 1/100 seconds */
	UINT32		uVideoFrameRate;	/* only available in MP4/3GP/ASF/AVI files */
	BOOL		bIsShortHeader;		/* TRUE:H.263, FALSE: MPEG4 */
	UINT16		usImageWidth;
	UINT16		usImageHeight;
	UINT32		uVidTotalFrames;	/* For playback, it's the total number of video frames. For recording, it's the currently recorded frame number. */
	UINT32		uVidFramesPlayed;	/* Indicate how many video frames have been played */
	UINT32		uVidFramesSkipped;  /* For audio/video sync, some video frames may be dropped. */
	UINT32		uVidPlayMediaLen;	/* Indicate how many video data have been played (bytes) */
	UINT32		uVidRecMediaLen;	/* Currently recorded video data length */
	UINT32		uVidMP4BuffSizeDB;	/* MP4 video decode buffer size (recorded in MP4 file) */
	UINT32		uVidMP4AvgBitRate;	/* MP4 video average bit rate (record/playback) */
	UINT32		uVidMP4MaxBitRate;  /* MP4 video maximum bit rate (recorded in MP4 file) */
	INT			nMPEG4HeaderPos;	/* The file offset of MPEG4 video header in the input MP4 file */
	INT			nMPEG4HeaderLen;	/* The length of MPEG4 video header in the input MP4 file */
	INT			n1stVidFramePos;	/* The file offset of first video frame in the input MP4/3GP file */
	INT			n1stVidFrameLen;	/* The lenght of first video frame in the input MP4/3GP file */

	INT			nMediaClipProgress;	/* The progress of media clipping, 0 ~ 100 */
	INT			nRecDataPerSec;		/* Recorder consumed storage space per second (in bytes) */
	INT			nMP4RecMetaReserved;/* MP4/3GP recorder required reserving meta data storage space (in bytes) */
	
	INT			nMP4RecMetaSize;	/* Only available in 3GP/MP4 record to memory */
	INT         nMP4RecMediaSize;	/* Only available in 3GP/MP4 record to memory */
	
	/* ID3 tag */
	ID3_TAG_T	tID3Tag;
	INT			puVisualData[32];	/* value range 0~31 */
	
	/* MP3 lyric */
	INT			nLyricLenInFile;	/* 0: no lyric, otherwise: the length of MP3 lyric in the MP3 file */
	INT			nLyricOffsetInFile; /* 0: no lyric, otherwise: the file offset of lyric in the Mp3 file */
	UINT32		uLyricCurTimeStamp;	/* in 1/100 seconds, time offset of the current lyric from the start of MP3 */
	CHAR		pcLyricCur[256];	/* on playback MP3, it contained the current lyric */
	UINT32		uLyricNextTimeStamp;/* in 1/100 seconds, time offset of the next lyric from the start of MP3 */
	CHAR		pcLyricNext[256];	/* on playback MP3, it contained the current lyric */
	
	BOOL		bIsEncrypted;		/* DRM or not */

	INT			nReserved1;			
	INT			nReserved2;
	INT			nReserved3;
	INT			nReserved4;

}	MV_INFO_T;

typedef VOID (AVI_CB)(AVI_INFO_T *);

extern void aviStopPlayFile(void);
extern void aviPlayControl(PLAY_CTRL_E cmd);
extern int aviPlayFile(char *suFileName, int x, int y, JV_MODE_E mode, AVI_CB *cb);
extern int aviGetFileInfo(char *suFileName, AVI_INFO_T *ptAviInfo);
extern void aviSetPlayVolume(int vol);
extern void aviSetRightChannelVolume(int vol);
extern void aviSetLeftChannelVolume(int vol);
extern char * mfl_get_last_buf(void);

#endif   // _AVI_PLAYER_H_
