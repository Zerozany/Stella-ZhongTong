#include "StellaSessionManager.h"

namespace stella_host
{
StellaSessionManager::StellaSessionManager(boost::asio::io_service &io_service, int32_t type, int32_t timeout)
    : m_type(type), m_timeout(timeout), m_timer(io_service)
{
    check_connection();
}

void StellaSessionManager::check_connection()
{
    try
    {
        if (m_session == nullptr)
        {
            return;
        }
        if (m_type == CLIENT)
        {
            // 客户端
            if (!m_session->GetSocket().is_open())
            {
                // 连接已断开
                qDebug() << "Session[" << m_session->GetSessionIp() << "] reconnecting...";
                m_session->close(); // 通过关闭会话来重连
            }
            else
            {
                // 连接中, 发送心跳包
                stella_type::StellaSocketSendData send_data;
                send_data.command = stella_type::tcp_command::heartbeat;

                m_session->async_write(send_data);
                m_session->set_last_op_time();
            }
        }
        else if (m_type == SERVER)
        {
            // 服务端
            if (!m_session->GetSocket().is_open())
            {
                // 连接已断开
                qDebug() << "Session[" << m_session->GetSessionIp() << "] closed";
            }
            else
            {
                // 连接中, 检查是否超时
                if (m_session->is_timeout())
                {
                    // 超时, 关闭会话
                    qDebug() << "Session[" << m_session->GetSessionIp() << "] timeout";
                    m_session->close();
                }
            }

            m_session->set_last_op_time();
        }
        else
        {
            qDebug() << "StellaSessionManager check_connection: unknown type";
        }

        // 定时器
        m_timer.expires_from_now(boost::posix_time::seconds(m_timeout));
        m_timer.async_wait(boost::bind(&StellaSessionManager::check_connection, this));
    }
    catch (const std::exception &e)
    {
        qDebug() << "exception: " << e.what();
    }
    catch (...)
    {
        qDebug() << "exception: unknown";
    }
}

} // namespace stella_host
