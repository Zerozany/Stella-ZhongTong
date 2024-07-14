#ifndef RECEIVE_H
#define RECEIVE_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "StellaClient.h"

class receive : public QThread
{
    Q_OBJECT
public:
    explicit receive(QObject *parent = nullptr);

    virtual void run() override;

signals:
    void receive_data_update(stella_type::StellaSocketReceiveMsg);

private:
    static void *update_data(void *args);
    static bool receive_tag;
    static stella_host::StellaClient m_client;

    static bool thread_flag;
    static pthread_t thread_handle;
    static std::mutex data_lock;

    static stella_type::StellaSocketReceiveData data;
};

#endif // RECEIVE_H
