#ifndef SENDER_H
#define SENDER_H

#include <QObject>
#include <qDebug>
#include "StellaType.h"
#include <QMutex>
#include <QThread>
#include <winsock2.h>

class sender : public QThread
{
    Q_OBJECT
public:
    explicit sender(QObject *parent = nullptr);
    //    void send_to_struct(stella_type::StellaSocketSendMsg def_send_msg);
    void run();
public slots:
    void send_to_struct(stella_type::StellaSocketSendMsg);
private:
    static bool connect_handle;
    static bool send_tag;
    QMutex m_lock;
    // 发送数据缓冲区
    static stella_type::StellaSocketSendMsg send_msg;

    // socket
    static SOCKET m_sockfd;
    static struct sockaddr_in m_addr;
    static int32_t m_addr_len;
signals:

};

#endif // SENDER_H
