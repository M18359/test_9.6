#define _CRT_SECURE_NO_WARNINGS


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// uart_test.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "gps_info.h" 				   
#include "stdarg.h"	 
#include "math.h"

//����ͷ����ֵ
GPS_ASCLLINFO Ascll_Info[2] = {
	ASCLL_DATA_GPGGA, 0x06, RCV_GPGGA_HEAD,
	ASCLL_DATA_GPRMC, 0x06, RCV_GPRMC_HEAD

};
//�����˽ṹ��
BlkUartRcv blkUartRcv;
GPGGA_INFO gga_info;
GPRMC_INFO gmc_info;

//��buf����õ���num���������ڵ�λ��
//����ֵ:0~0xFE,����������λ�õ�ƫ��.
//       0xFF,�������ڵ�num������							  
u1 _Find_Pos(u1* buf, u1 u1Num)
{
	u1* p = buf;
	while (u1Num)
	{
		if (*buf == '*' || *buf < ' ' || *buf>'z')return 0xFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�num������
		if (*buf == ',')u1Num--;
		buf++;
	}
	return buf - p;
}

//m^n����
//����ֵ:m^n�η�.
u4 _Pow(u1 m, u1 n)
{
	u4 result = 1;
	while (n--)result *= m;
	return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
int _Str2num(u1* buf, u1* dx)
{
	u1* p = buf;
	u4 ires = 0, fres = 0;
	u1 ilen = 0, flen = 0, i;
	u1 mask = 0;
	double res;
	while (1) //�õ�������С���ĳ���
	{
		if (*p == '-') { mask |= 0x02; p++; }//�Ǹ���
		if (*p == ',' || (*p == '*'))break;//����������
		if (*p == '.') { mask |= 0x01; p++; }//����С������
		else if (*p > '9' || (*p < '0'))	//�зǷ��ַ�
		{
			ilen = 0;
			flen = 0;
			break;
		}
		if (mask & 0x01)flen++;
		else ilen++;
		p++;
	}
	if (mask & 0x02)buf++;		//ȥ������
	for (i = 0; i < ilen; i++)	//�õ�������������
	{
		ires += _Pow(10, ilen - 1 - i) * (buf[i] - '0');
	}
	if (flen > 5)flen = 5;	//���ȡ5λС��
	*dx = flen;	 			//С����λ��
	for (i = 0; i < flen; i++)	//�õ�С����������
	{
		fres += _Pow(10, flen - 1 - i) * (buf[ilen + 1 + i] - '0');
	}
	res = ires * _Pow(10, flen) + fres;
	if (mask & 0x02)res = -res;
	return res;
}

//����GPRMC��Ϣ
//buf:���յ���GPS���ݻ������׵�ַ
void _GPRMC_Analysis(GPRMC_INFO* gps_rmc_info, u1* buf)
{
	u1* p1, dx;
	u1 posx;
	u4 temp;
	float rs;

	p1 = (u1*)strstr((const char*)buf, "$GPRMC");				// "$GPRMC"
	if (p1 == NULL) {
		return;
	}

	posx = _Find_Pos(p1, 1);									// �õ�UTCʱ��
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx) / _Pow(10, dx);			// �õ�UTCʱ��,ȥ��ms	 	
		gps_rmc_info->utc_time.hour = temp / 10000;
		gps_rmc_info->utc_time.min = (temp / 100) % 100;
		gps_rmc_info->utc_time.sec = temp % 100;
		gps_rmc_info->utc_time.ssec = _Str2num(p1 + posx, &dx) % _Pow(10, dx);
		printf("GPS UTC Сʱ��%d ���ӣ�%d �룺%d ���룺%d \n\r",
			gps_rmc_info->utc_time.hour,
			gps_rmc_info->utc_time.min,
			gps_rmc_info->utc_time.sec,
			gps_rmc_info->utc_time.ssec);
	}

	posx = _Find_Pos(p1, 2);									// �õ���λ״̬
	if (posx != 0xFF) {
		gps_rmc_info->gps_state = *(p1 + posx);
		printf("GPS����״̬��%c \n\r", gps_rmc_info->gps_state);
	}

	posx = _Find_Pos(p1, 3);									// �õ�γ��ֵ
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_rmc_info->latitude_value = (f8)temp / (f8)_Pow(10, dx);
		printf("GPSγ��ֵ��%.5f \n\r", gps_rmc_info->latitude_value);
	}

	posx = _Find_Pos(p1, 4);									// ��γ���Ǳ�γ
	if (posx != 0xFF) {
		gps_rmc_info->latitude = *(p1 + posx);
		printf("GPSγ�ȣ�%c \n\r", gps_rmc_info->latitude);
	}

	posx = _Find_Pos(p1, 5);									// �õ�����ֵ
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_rmc_info->longtitude_value = (f8)temp / (f8)_Pow(10, dx);
		printf("GPS����ֵ��%.5f \n\r", gps_rmc_info->longtitude_value);
	}

	posx = _Find_Pos(p1, 6);									// ������������
	if (posx != 0xFF) {
		gps_rmc_info->longtitude = *(p1 + posx);
		printf("GPS���ȣ�%c \n\r", gps_rmc_info->longtitude);
	}

	posx = _Find_Pos(p1, 7);									// �ٶ�
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_rmc_info->speed = (f4)temp / (f4)_Pow(10, dx);
		printf("GPS�ٶ�ֵ��%.3f \n\r", gps_rmc_info->speed);
	}

	posx = _Find_Pos(p1, 8);									// ���溽��
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_rmc_info->azimuth_angle = (f4)temp / (f4)_Pow(10, dx);
		printf("GPS�ٶ�ֵ��%.3f \n\r", gps_rmc_info->azimuth_angle);
	}

	posx = _Find_Pos(p1, 9);									//�õ�UTC����
	if (posx != 0xFF)
	{
		temp = _Str2num(p1 + posx, &dx);		 				//�õ�UTC����
		gps_rmc_info->utc_time.date = temp / 10000;
		gps_rmc_info->utc_time.month = (temp / 100) % 100;
		gps_rmc_info->utc_time.year = 2000 + temp % 100;
		printf("GPS UTC �꣺%d �£�%d �գ�%d \n\r",
			gps_rmc_info->utc_time.year,
			gps_rmc_info->utc_time.month,
			gps_rmc_info->utc_time.date);
	}
}


//����GPGGA��Ϣ
//buf:���յ���GPS���ݻ������׵�ַ
void _GPGGA_Analysis(GPGGA_INFO* gps_gga_info, u1* buf)
{
	u1* p1, dx;
	u1 posx;
	u4 temp;
	float rs;

	//����str2��str1���״γ��ַ��صĵ�ַ
	p1 = (u1*)strstr((const char*)buf, "$GPGGA");				// "$GPRMC"
	//��GPGGA����str1���Ӵ�����NULL
	if (p1 == NULL) {
		return;
	}


	//120304


	posx = _Find_Pos(p1, 1);									// �õ�UTCʱ��
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx) / _Pow(10, dx);		// �õ�UTCʱ��,ȥ��ms	 	
		gps_gga_info->utc_time.hour = temp / 10000;
		gps_gga_info->utc_time.min = (temp / 100) % 100;
		gps_gga_info->utc_time.sec = temp % 100;
		gps_gga_info->utc_time.ssec = _Str2num(p1 + posx, &dx) % _Pow(10, dx);
		printf("GPS UTC Сʱ��%d ���ӣ�%d �룺%d ���룺%d \n\r",
			gps_gga_info->utc_time.hour,
			gps_gga_info->utc_time.min,
			gps_gga_info->utc_time.sec,
			gps_gga_info->utc_time.ssec);
	}

	posx = _Find_Pos(p1, 2);									// �õ�γ��ֵ
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_gga_info->latitude_value = (f8)temp / (f8)_Pow(10, dx);
		printf("GPSγ��ֵ��%.5f \n\r", gps_gga_info->latitude_value);
	}

	posx = _Find_Pos(p1, 3);									// ��γ���Ǳ�γ
	if (posx != 0xFF) {
		gps_gga_info->latitude = *(p1 + posx);
		printf("GPSγ�ȣ�%c \n\r", gps_gga_info->latitude);
	}

	posx = _Find_Pos(p1, 4);									// �õ�����ֵ
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_gga_info->longtitude_value = (f8)temp / (f8)_Pow(10, dx);
		printf("GPS����ֵ��%.5f \n\r", gps_gga_info->longtitude_value);
	}

	posx = _Find_Pos(p1, 5);									// ������������
	if (posx != 0xFF) {
		gps_gga_info->longitude = *(p1 + posx);
		printf("GPS���ȣ�%c \n\r", gps_gga_info->longitude);
	}

	posx = _Find_Pos(p1, 6);									// GPS״̬
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_gga_info->gps_state = temp;
		printf("GPS״ֵ̬��%d \n\r", gps_gga_info->gps_state);
	}

	posx = _Find_Pos(p1, 7);									// ��������
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_gga_info->sate_num = temp;
		printf("GPS����������%d \n\r", gps_gga_info->sate_num);
	}

	posx = _Find_Pos(p1, 8);									// HDOPˮƽ��������
	if (posx != 0xFF) {
		gps_gga_info->hdop = (f4)temp / (f4)_Pow(10, dx);
		printf("HDOPˮƽ�������ӣ�%f \n\r", gps_gga_info->hdop);
	}

	posx = _Find_Pos(p1, 9);									// ���θ߶�
	if (posx != 0xFF) {
		temp = _Str2num(p1 + posx, &dx);
		gps_gga_info->altitude = (f4)temp / (f4)_Pow(10, dx);
		printf("���θ߶ȣ�%f \n\r", gps_gga_info->altitude);
	}
}


void _ResetUartRcvBuff()
{
	memset(&blkUartRcv, 0, sizeof(BlkUartRcv));
}

void GN_UartRcvGPSInfo(u1 u1Data)
{
	int check, sum, hour, min, sec;
	BlkUartRcv* pblkUartRcv = &blkUartRcv;
	u1* pu1Buff = pblkUartRcv->u1RcvBuff;
	u1			u1size = 0;
	char			u1utc[20];
	//���״̬Ϊ0����Ϊ1
	if ((u1Data == ASCLL_DATA_$) &&
		(pblkUartRcv->u1State == RCV_STATE_IDIE)) {
		pblkUartRcv->u1State = RCV_STATE_START;
	}
	//�������ָ����С,����Ϊ0
	if (pblkUartRcv->u1DataLen >= MAX_RCV_SIZE) {
		_ResetUartRcvBuff();
	}
	//ѡ��״̬
	switch (pblkUartRcv->u1State) {
	case RCV_STATE_START:           //1
		//��data����buff��
		pu1Buff[pblkUartRcv->u1DataLen++] = u1Data;

		if (u1Data == '\n') {
			for (u1size = 0; u1size < sizeof(Ascll_Info) / sizeof(GPS_ASCLLINFO); u1size++) {
				if (CMP_SUCCESS == memcmp(&pu1Buff[0], Ascll_Info[u1size].u1AscllName, Ascll_Info[u1size].u1sizeof)) {
					_switchState(Ascll_Info[u1size].u1Head);
				}
			}
		}
		break;
	case RCV_GPGGA_HEAD:   //2

		printf("-----GPGGA------ \r\n");
		printf("GPGGA:%s \r\n", pu1Buff);
		_GPGGA_Analysis(&gga_info, pu1Buff);	// GPGGA��������
		_ResetUartRcvBuff();
		break;
	case RCV_GPRMC_HEAD:

		printf("-----GPRMC------ \r\n");
		printf("GPRMC:%s \r\n", pu1Buff);
		_GPRMC_Analysis(&gmc_info, pu1Buff);	// GPRMC��������
		_ResetUartRcvBuff();
		break;
		break;
	default:
		break;
	}
END:
	return;
}

int main()
{
	// $GPGGA,105547.00,3959.99990484,N,11559.73608378,E,1,10,0.9,555.1075,M,-9.2296,M,,*7A\r\n
	// $GPRMC,105546.00,A,3959.99990614,N,11559.73608463,E,0.004,300.7,140622,5.7,W,A*22\r\n
	u1 nema_gga[] = "$GPRMC,105546.000,A,3959.99990614,N,11559.73608463,E,0.004,300.7,140622,5.7,W,A*22\r\n $GPGGA,105547.00,3959.99990484,N,11559.73608378,E,1,10,0.9,555.1075,M,-9.2296,M,,*7A\r\n";

	memset(&gmc_info, 0x00, sizeof(gmc_info));
	int i = sizeof(nema_gga);

	for (i = 0; i <= sizeof(nema_gga); i++) {
		GN_UartRcvGPSInfo(nema_gga[i]);
	}

	getchar();
	return 0x00;
}