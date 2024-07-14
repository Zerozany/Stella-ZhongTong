#include "StellaSocketSession.h"
#include "StellaSerialization.h"

namespace stella_host
{
StellaSocketSession::StellaSocketSession(boost::asio::io_service &io_service)
    : m_socket(io_service), m_io_service(io_service)
{}

StellaSocketSession::~StellaSocketSession()
{
    m_socket.close();
}

void StellaSocketSession::start()
{
    // 设置socket属性
    m_socket.set_option(boost::asio::ip::tcp::acceptor::linger(true, 0));
    m_socket.set_option(boost::asio::socket_base::keep_alive(true));
    // 设置操作时间
    set_last_op_time();
    // 异步读取数据长度
    const boost::system::error_code error;
    handle_read_size(error);
}

void StellaSocketSession::handle_close()
{
    try
    {
        m_socket.close();
        close_callback(shared_from_this());
    }
    catch (const std::exception &e)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: unknown";
    }
}

void StellaSocketSession::close()
{
    // 由于回调函数中有加锁, 需要post到io_service中执行, 否则会造成死锁
    m_io_service.post(boost::bind(&StellaSocketSession::handle_close, shared_from_this()));
}

static int32_t connection_timeout = 60; // 连接超时时间
bool StellaSocketSession::is_timeout()
{
    std::time_t now;
    std::time(&now);
    return (now - m_last_op_time) > connection_timeout;
}

void StellaSocketSession::handle_read_size(const boost::system::error_code &error)
{
    try
    {
        if (error)
        {
            qDebug() << "Session[" << GetSessionIp() << "] read size error: " << error.message();
            close();
            return;
        }

        std::string data;
        data.swap(m_data);
        boost::asio::async_read(
            m_socket, boost::asio::buffer(&m_data_size, sizeof(m_data_size)),
            boost::bind(&StellaSocketSession::handle_read_data, shared_from_this(), boost::asio::placeholders::error));

        if (data.length() > 0 && data != "")
        {
            stella_type::StellaSocketReceiveData receive_data;
            // std::string转struct
            str_to_struct(data, receive_data);
            m_read_data_callback(error, shared_from_this(), receive_data);
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: " << e.what();
        close();
    }
    catch (...)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: unknown";
        close();
    }
}

void StellaSocketSession::handle_read_data(const boost::system::error_code &error)
{
    try
    {
        if (error)
        {
            qDebug() << "Session[" << GetSessionIp() << "] read data error: " << error.message();
            close();
            return;
        }
        qDebug() << "data size: " << m_data_size;
        m_data.resize(m_data_size);
        boost::asio::async_read(
            m_socket, boost::asio::buffer(&m_data[0], m_data_size),
            boost::bind(&StellaSocketSession::handle_read_size, shared_from_this(), boost::asio::placeholders::error));
    }
    catch (const std::exception &e)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: " << e.what();
        close();
    }
    catch (...)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: unknown";
        close();
    }
}

void StellaSocketSession::handle_write(const boost::system::error_code &error, int32_t size, std::string *data)
{
    try
    {
        if (error)
        {
            qDebug() << "Session[" << GetSessionIp() << "] write error: " << error.message();
            close();
            return;
        }
        // 发送数据后, 释放内存
        if (data != nullptr)
        {
            delete data;
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: " << e.what();
        close();
    }
    catch (...)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: unknown";
        close();
    }
}

void StellaSocketSession::async_write(const std::string &data)
{
    try
    {
        int32_t size = data.length();
        char *size_ptr = reinterpret_cast<char *>(&size);

        std::string *data_ptr = new std::string();
        data_ptr->append(size_ptr, sizeof(size));
        data_ptr->append(data);

        boost::asio::async_write(m_socket, boost::asio::buffer(*data_ptr, data_ptr->length()),
                                 boost::bind(&StellaSocketSession::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred, data_ptr));
    }
    catch (const std::exception &e)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: " << e.what();
        close();
    }
    catch (...)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: unknown";
        close();
    }
}

void StellaSocketSession::async_write(const stella_type::StellaSocketSendData &data)
{
    try
    {
        std::string str;
        // struct转std::string
        struct_to_str(data, str);
        async_write(str);
    }
    catch (const std::exception &e)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: " << e.what();
        close();
    }
    catch (...)
    {
        qDebug() << "Session[" << GetSessionIp() << "] exception: unknown";
        close();
    }
}

} // namespace stella_host
