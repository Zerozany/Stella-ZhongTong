#pragma once

#include "StellaType.h"

#include <QDebug>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace stella_host
{
class StellaSocketSession;
typedef boost::shared_ptr<StellaSocketSession> session_ptr;

class StellaSocketSession : public boost::enable_shared_from_this<StellaSocketSession>, private boost::noncopyable
{
  public:
    typedef boost::function<void(session_ptr)> close_callback; // 关闭回调函数
    typedef boost::function<void(const boost::system::error_code &, session_ptr,
                                 stella_type::StellaSocketReceiveData &)>
        read_data_callback; // 读取数据回调函数

    StellaSocketSession(boost::asio::io_service &io_service);
    ~StellaSocketSession();

    /// \brief 获取会话ID
    /// \return 会话ID
    inline int32_t GetSessionId() const { return m_session_id; }

    /// \brief 获取会话IP
    /// \return 会话IP
    inline std::string GetSessionIp() const { return m_session_ip; }

    /// \brief 设置会话IP
    /// \param[in] ip 会话IP
    /// \return void
    inline void SetSessionIp(std::string ip) { m_session_ip = ip; }

    /// \brief 获取socket
    /// \return socket
    inline boost::asio::ip::tcp::socket &GetSocket() { return m_socket; }

    /// \brief 设置关闭回调函数
    /// \param[in] callback 回调函数
    /// \return void
    inline void SetCloseCallback(close_callback callback) { m_close_callback = callback; }

    /// \brief 设置读取数据回调函数
    /// \param[in] callback 回调函数
    /// \return void
    inline void SetReadDataCallback(read_data_callback callback) { m_read_data_callback = callback; }

    /// \brief 启动
    /// \return void
    void start();

    /// \brief 关闭
    /// \return void
    void close();

    /// \brief 发送数据
    /// \param[in] data 发送数据
    /// \return void
    void async_write(const std::string &data);

    /// \brief 发送数据
    /// \param[in] data 发送数据
    /// \return void
    void async_write(const stella_type::StellaSocketSendData &data);

    /// \brief 判断是否超时
    /// \return 是否超时
    bool is_timeout();

    /// \brief 设置最后操作时间
    /// \return void
    inline void set_last_op_time() { std::time(&m_last_op_time); }

  private:
    /// \brief 处理发送数据
    /// \param[in] error 错误码
    /// \param[in] size 发送数据大小
    /// \param[in] data 发送数据
    /// \return void
    void handle_write(const boost::system::error_code &error, int32_t size, std::string *data);

    /// \brief 处理接收数据长度
    /// \param[in] error 错误码
    /// \return void
    void handle_read_size(const boost::system::error_code &error);

    /// \brief 处理接收数据
    /// \param[in] error 错误码
    /// \return void
    void handle_read_data(const boost::system::error_code &error);

    /// \brief 处理关闭
    /// \return void
    void handle_close();

    int32_t m_session_id;     // 会话ID
    std::string m_session_ip; // 会话IP
    int32_t m_data_size;      // 数据大小
    std::string m_data;       // 数据

    boost::asio::ip::tcp::socket m_socket; // socket
    boost::asio::io_service &m_io_service; // io_service

    std::time_t m_last_op_time; // 最后操作时间

    close_callback m_close_callback;         // 关闭回调函数
    read_data_callback m_read_data_callback; // 读取数据回调函数
};
} // namespace stella_host
