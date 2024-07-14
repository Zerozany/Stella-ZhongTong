#pragma once

#include "StellaSessionManager.h"
#include "StellaType.h"
#include <mutex>

namespace stella_host
{
class StellaClient
{
  public:
    StellaClient();
    ~StellaClient() = default;

    /// \brief 连接会话
    /// \param[in] session 会话
    /// \return void
    void connect(session_ptr session);

    /// \brief 获取io_service
    /// \return io_service
    inline boost::asio::io_service &GetIoService() { return m_io_service; }

    /// \brief 获取数据
    /// \return 数据
    inline void GetReceiveData(stella_type::StellaSocketReceiveData &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        data = m_receive_data;
    }

  protected:
    /// \brief 处理读取数据
    /// \param[in] data 数据
    /// \param[in] session 会话
    /// \return void
    void handle_read_data(stella_type::StellaSocketReceiveData &data, session_ptr session);

  private:
    /// \brief 处理连接
    /// \param[in] error 错误码
    /// \param[in] endpoint_iterator 迭代器
    /// \param[in] session 会话
    /// \return void
    void handle_connect(const boost::system::error_code &error,
                        boost::asio::ip::tcp::resolver::iterator endpoint_iterator, session_ptr session);

    /// \brief 关闭回调函数
    /// \param[in] session 会话
    /// \return void
    void close_callback(session_ptr session);

    /// \brief 读取数据回调函数
    /// \param[in] error 错误码
    /// \param[in] session 会话
    /// \param[in] data 数据
    /// \return void
    void read_data_callback(const boost::system::error_code &error, session_ptr session,
                            stella_type::StellaSocketReceiveData &data);

    boost::asio::io_service m_io_service;   // io_service对象
    boost::asio::io_service::work m_work;   // io_service工作对象
    StellaSessionManager m_session_manager; // 会话管理器

    stella_type::StellaSocketReceiveData m_receive_data; // 接收数据

    std::mutex m_mutex; // 互斥锁
};
} // namespace stella_host