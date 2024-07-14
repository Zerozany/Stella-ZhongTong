#include "receive.h"

bool receive::receive_tag = false;
stella_host::StellaClient receive::m_client;
bool receive::thread_flag = false;
pthread_t receive::thread_handle;
std::mutex receive::data_lock;
stella_type::StellaSocketReceiveData receive::data;

receive::receive(QObject *parent)
    : QThread{parent}
{
    qDebug() <<"Receiver thread start ...";

    if (pthread_create(&thread_handle, NULL, update_data, NULL) != 0)
    {
        qDebug() << "Failed to create update data thread";
        return;
    }
    thread_flag = true;
    receive_tag = true;

    usleep(10000);
}

void *receive::update_data(void *args)
{
    while(thread_flag)
    {
        stella_host::session_ptr session =
            boost::make_shared<stella_host::StellaSocketSession>(m_client.GetIoService());
        session->SetSessionIp("192.168.30.21:23459");

        m_client.connect(session);
        m_client.GetIoService().run();
    }
}

void receive::run()
{
    while(receive_tag)
    {
        stella_type::StellaSocketReceiveMsg msg;
        {
            std::lock_guard<std::mutex> lock(data_lock);
            m_client.GetReceiveData(data);
        }
        msg = data.msg;
        {
            std::lock_guard<std::mutex> lock(data_lock);
            emit receive_data_update(msg);
        }
        usleep(100000);
    }
}
