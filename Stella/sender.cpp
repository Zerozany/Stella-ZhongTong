#include "sender.h"
#include "StellaSerialization.h"

using namespace stella_type;

bool sender::connect_handle = true;
stella_type::StellaSocketSendMsg sender::send_msg{};
bool sender::send_tag;

// socket
SOCKET sender::m_sockfd; // socket句柄
struct sockaddr_in sender::m_addr;
int32_t sender::m_addr_len;

sender::sender(QObject *parent)
    : QThread{parent}
{
    DWORD ver;
    WSADATA WSAData;
    ver = MAKEWORD(1,1);
    WSAStartup(ver, &WSAData);

    // 初始化socket
    int32_t port_out = 10001;
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1ull == m_sockfd)
    {
        return;
    }
    m_addr_len = sizeof(m_addr);
    memset(&m_addr, 0, m_addr_len);
    // 设置socket地址
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port_out);
    m_addr.sin_addr.s_addr = inet_addr("192.168.30.21");

    send_tag = true;
    usleep(10000);
}


void sender::run()
{
    // 发送数据缓冲区
    StellaSocketSendMsg send_msg_temp;

    while (send_tag)
    {
        // 获取全局变量中的数据
        m_lock.lock();
        send_msg_temp = send_msg;
        m_lock.unlock();
        // 转char*指针
        int32_t size = 0;
        char *p = stella_host::struct_to_char(send_msg_temp, size);
        // 发送数据
        sendto(m_sockfd, p, size, 0, (struct sockaddr *)&m_addr, m_addr_len);
        // 删除char*指针
        delete[] p;
        // 休眠100ms
        usleep(100000);
    }
}


void sender::send_to_struct(StellaSocketSendMsg def_send_msg)
{
    m_lock.lock();
    send_msg = def_send_msg;
    m_lock.unlock();
}
