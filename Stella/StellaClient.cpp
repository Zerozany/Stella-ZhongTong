#include "StellaClient.h"
#include "StellaType.h"

#include <boost/algorithm/string.hpp>

namespace stella_host
{
StellaClient::StellaClient() : m_work(m_io_service), m_session_manager(m_io_service, CLIENT, 10) {}

void StellaClient::connect(session_ptr session)
{
    std::string ip = session->GetSessionIp();
    try
    {
        // 设置关闭回调函数
        session->SetCloseCallback(boost::bind(&StellaClient::close_callback, this, _1));
        // 设置读取数据回调函数
        session->SetReadDataCallback(boost::bind(&StellaClient::read_data_callback, this, _1, _2, _3));

        std::vector<std::string> ip_port;
        boost::split(ip_port, ip, boost::is_any_of(":"));
        if (ip_port.size() < 2)
        {
            qDebug() << "StellaClient connect error: ip_port.size() < 2";
            return;
        }
        boost::asio::ip::tcp::resolver resolver(session->GetSocket().get_io_service());
        boost::asio::ip::tcp::resolver::query query(ip_port[0], ip_port[1]);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        m_session_manager.set_session(session);

        boost::asio::ip::tcp::endpoint endpoint = *iterator;
        session->GetSocket().async_connect(
            endpoint,
            boost::bind(&StellaClient::handle_connect, this, boost::asio::placeholders::error(), iterator, session));
    }
    catch (const std::exception &e)
    {
        qDebug() << "StellaClient exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "StellaClient exception: unknown";
    }
}

void StellaClient::handle_connect(const boost::system::error_code &error,
                                  boost::asio::ip::tcp::resolver::iterator endpoint_iterator, session_ptr session)
{
    try
    {
        if (!error)
        {
            qDebug() << "StellaClient connect success";
            session->start();

            // 注册
            stella_type::StellaSocketSendData data;
            data.command = stella_type::tcp_command::regist;

            session->async_write(data);
        }
        else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
        {
            qDebug() << "StellaClient connect failed, reconnecting...";
            session->GetSocket().close();
            boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
            session->GetSocket().async_connect(endpoint, boost::bind(&StellaClient::handle_connect, this,
                                                                     boost::asio::placeholders::error(),
                                                                     endpoint_iterator, session));
        }
        else
        {
            qDebug() << "StellaClient connect error: " << error.message();
            session->GetSocket().close();
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "StellaClient exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "StellaClient exception: unknown";
    }
}

void StellaClient::read_data_callback(const boost::system::error_code &error, session_ptr session,
                                      stella_type::StellaSocketReceiveData &data)
{
    try
    {
        if (error)
        {
            qDebug() << "StellaClient read_data_callback error: " << error.message();
            return;
        }
        if (data.command == stella_type::tcp_command::heartbeat)
        {
            qDebug() << "StellaClient read_data_callback: heartbeat";
        }
        else if (data.command == stella_type::tcp_command::regist)
        {
            qDebug() << "StellaClient read_data_callback: " << session->GetSessionIp() << " regist done";
            stella_type::StellaSocketSendData send_data;
            send_data.command = stella_type::tcp_command::normal;
            session->async_write(send_data);
        }
        else if (data.command == stella_type::tcp_command::normal)
        {
            // qDebug() << "StellaClient read_data_callback: " << session->GetSessionIp() << " normal";
            stella_type::StellaSocketSendData send_data;
            send_data.command = stella_type::tcp_command::normal;
            session->async_write(send_data);
            // 处理读取数据
            handle_read_data(data, session);
        }
        else
        {
            qDebug() << "StellaClient read_data_callback: unknown command";
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "StellaClient exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "StellaClient exception: unknown";
    }
}

void StellaClient::close_callback(session_ptr session)
{
    try
    {
        std::string ip = session->GetSessionIp();
        std::vector<std::string> ip_port;
        boost::split(ip_port, ip, boost::is_any_of(":"));
        if (ip_port.size() < 2)
        {
            qDebug() << "StellaClient close_callback error: ip_port.size() < 2";
            return;
        }

        boost::asio::ip::tcp::resolver resolver(session->GetSocket().get_io_service());
        boost::asio::ip::tcp::resolver::query query(ip_port[0], ip_port[1]);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        boost::asio::ip::tcp::endpoint endpoint = *iterator;
        session->GetSocket().async_connect(
            endpoint,
            boost::bind(&StellaClient::handle_connect, this, boost::asio::placeholders::error(), iterator, session));
    }
    catch (const std::exception &e)
    {
        qDebug() << "StellaClient exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "StellaClient exception: unknown";
    }
}

void StellaClient::handle_read_data(stella_type::StellaSocketReceiveData &data, session_ptr session)
{
    try
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_receive_data = data;
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "StellaClient exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "StellaClient exception: unknown";
    }
}

} // namespace stella_host
