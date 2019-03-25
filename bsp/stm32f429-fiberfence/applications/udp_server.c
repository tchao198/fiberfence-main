/*
 * 代码清单：UDP服务端，接收和处理上位机发送的命令
 */
#include <rtthread.h>
#include <stdio.h>
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */

#define MAX_DATA_LEN 512
#define BASE_PACK_LEN 10

//查询命令
#define CMD_EXIT        0x100f
#define CMD_CPU_USAGE   0x1001
#define CMD_OPTIC_POWER 0x1002
#define CMD_ALARM_COUNT 0x1003

//控制命令
#define CMD_DAC_GAIN_A        0x2001
#define CMD_DAC_GAIN_B        0x2002
#define CMD_ADC_SAMPLE_FRQ    0x2003
#define CMD_ALARM_THRESHOLD_A 0x2004
#define CMD_ALARM_THRESHOLD_B 0x2005
#define CMD_POWER_THRESHOLD   0x2006
#define CMD_ALARM_INTERVAL    0x2007

//响应命令
#define CMD_RESPONSE_OK        0x0000
#define CMD_RESPONSE_UNKNOW    0x1111
#define CMD_RESPONSE_MAGIC_ERR 0x2222

extern float get_cpu(void);
extern int set_dac(rt_uint16_t value, rt_uint16_t chip_id);

struct Cmd_Data{
    uint16_t magic;
    uint16_t cmd;
    float    r1;
    uint16_t r2;
    uint16_t len;
    char data[MAX_DATA_LEN];
};

void response(int sock, int cmd, struct sockaddr *client_addr, int addr_len)
{
	struct Cmd_Data r;
	r.magic = 0x1234;
	r.cmd = cmd;
	r.r1 = 0.25;
	r.r2 = 1024;
	sendto(sock, (char *)&r, BASE_PACK_LEN+2, 0, client_addr, addr_len);
}

/*********************************************************
* 函数名：cmd_process
* 
* 功  能：处理接收到的命令
*********************************************************/
void cmd_process(int sock, struct Cmd_Data *cd, struct sockaddr *client_addr, int addr_len)
{

}

/*********************************************************
* 函数名：udpserv
* 
* 功  能：udp服务，接收上位机发送的命令，并响应上位机
*********************************************************/
void udpserv(void* paramemter)
{
    int sock;
    int bytes_read;
    char *recv_data;
		struct Cmd_Data *cmd_data_t;
    rt_uint32_t addr_len;
    struct sockaddr_in server_addr, client_addr;

    /* 分配接收用的数据缓冲 */
    recv_data = rt_malloc(1024);
    if (recv_data == RT_NULL)
    {
        /* 分配内存失败，返回 */
        rt_kprintf("No memory\n");
        return;
    }

    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");

        /* 释放接收用的数据缓冲 */
        rt_free(recv_data);
        return;
    }

    /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 绑定socket到服务端地址 */
    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr))
            == -1)
    {
        /* 绑定地址失败 */
        rt_kprintf("Bind error\n");

        /* 释放接收用的数据缓冲 */
        rt_free(recv_data);
        return;
    }

    addr_len = sizeof(struct sockaddr);
    rt_kprintf("UDPServer Waiting for client on port 5000...\n");

    while (1)
    {
        /* 从sock中收取最大1024字节数据 */
        bytes_read = recvfrom(sock, recv_data, 1024, 0,
                              (struct sockaddr *) &client_addr, &addr_len);			
			
				if(bytes_read <=0 )
				{
						rt_kprintf("recive error\n");
						continue;
				}

				cmd_data_t=(struct Cmd_Data *)recv_data;
				
				/* 校验帧头 */
				if(cmd_data_t->magic!=0x1234)
				{
						rt_kprintf("magic error\n");
						continue;
				}
				/* 处理接收到命令 */
				cmd_process(sock, cmd_data_t, (struct sockaddr *) &client_addr, addr_len);
    }
		
		lwip_close(sock);
		rt_free(recv_data);

    return;
}
