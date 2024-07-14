#pragma once

#include "StellaSocketSession.h"
#include <mutex>

namespace stella_host
{
#define CLIENT 0
#define SERVER 1

class StellaSessionManager
{
  public:
    StellaSessionManager(boost::asio::io_service &io_service, int32_t type, int32_t timeout);
    ~StellaSessionManager() = default;

    /// \brief 设置会话
    /// \param[in] session 会话
    /// \return void
    inline void set_session(session_ptr session)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_session = session;
    }

  private:
    /// \brief 检查连接
    /// \return void
    void check_connection();

    int32_t m_type;                      // 类型
    int32_t m_timeout;                   // 超时时间
    boost::asio::deadline_timer m_timer; // 定时器
    std::mutex m_mutex;                  // 读写锁
    session_ptr m_session;               // 会话
};

} // namespace stella_host