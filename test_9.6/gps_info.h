#pragma once
#ifndef _GN_GPSINFO_H__
#define _GN_GPSINFO_H__

//�����ֽڶ���
typedef   int				i4;
typedef   unsigned int		u4;
typedef   short             i2;
typedef   unsigned short    u2;
typedef   signed char		i1;
typedef   unsigned char     u1;
typedef   float             f4;
typedef   double			f8;


//ͷ������
#define  ASCLL_DATA_$           '$'
#define  ASCLL_DATA_GPGGA       "$GPGGA"
#define  ASCLL_DATA_GPRMC       "$GPRMC"


#define  MAX_RCV_SIZE			1024*5
#define  GPGGA_HEAD_LEN			0x06
#define  CMP_SUCCESS			0x00
#define  CMP_ERROR				0x01

#define     _switchState(a)         pblkUartRcv->u1State = a


//ö��--����������0��1��2��3
enum
{
	RCV_STATE_IDIE = 0,   //0
	RCV_STATE_START,     //1
	RCV_GPGGA_HEAD,      //2
	RCV_GPRMC_HEAD       //3
};

typedef struct
{
	u1	u1AscllName[10];
	u1	u1sizeof;
	u1	u1Head;
}GPS_ASCLLINFO;


typedef struct UartRcvTag
{
	u1 u1RcvBuff[MAX_RCV_SIZE];
	u1 u1State ;
	u1 u1DataLen;
	u1 u1Len;
}BlkUartRcv;


//UTCʱ����Ϣ
typedef struct
{
	u2 year;	//���
	u1 month;	//�·�
	u1 date;	//����
	u1 hour; 	//Сʱ
	u1 min; 	//����
	u1 sec; 	//����
	u1 ssec;	//����
}gps_utc_time;

typedef struct
{
	gps_utc_time	utc_time;			// UTCʱ��
	f8	latitude_value;					// γ��
	u1	latitude;						// γ�Ȱ���
	f8	longtitude_value;				// ����
	u1	longitude;						// ���Ȱ���
	u1	gps_state;						// GPS״̬ 0=δ��λ��1=�ǲ�ֶ�λ��2=��ֶ�λ��6=���ڹ���
	u1	sate_num;						// ����λ����������
	f4	hdop;							// HDOPˮƽ��������
	f4	altitude;						// ���θ߶�
}GPGGA_INFO;



typedef struct
{
	gps_utc_time utc_time;		// UTCʱ��
	u1 gps_state;				// ��λ״̬
	f8 latitude_value;			// γ��
	u1 latitude;				// γ�Ȱ���
	f8 longtitude_value;		// ����
	u1 longtitude;				// ���Ȱ���
	f4 speed;					// ��������
	f4 azimuth_angle;			// ���溽��

}GPRMC_INFO;

int _Str2num(u1* buf, u1* dx);

void _GPRMC_Analysis(GPRMC_INFO* gps_rmc_info, u1* buf);
void _GPGGA_Analysis(GPGGA_INFO* gps_gga_info, u1* buf);

#endif

