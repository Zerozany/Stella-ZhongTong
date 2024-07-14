#pragma once

#include "StellaType.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#define MAX_BUFFER_LEN 6 * 1024 * 1024

// 在结构体外部定义一个全局的serialize函数，用boost::serialization命名空间包裹
namespace boost
{
namespace serialization
{
template <class Archive> void serialize(Archive &ar, stella_type::StellaSocketSendData &s, const uint32_t version)
{
    // 指定哪些成员变量需要序列化
    ar &s.command; // 命令
}

template <class Archive> void serialize(Archive &ar, stella_type::StellaSocketReceiveData &s, const uint32_t version)
{
    // 指定哪些成员变量需要序列化
    ar &s.command; // 命令
    ar &s.msg;     // 数据
}

template <class Archive> void serialize(Archive &ar, stella_type::StellaSocketReceiveMsg &s, const uint32_t version)
{
    // 指定哪些成员变量需要序列化
    ar &s.timestamp; // 时间戳
    ar &s.frame_id;  // 帧序列号

    ar &s.maps; // 地图

    ar &s.current_frame_id;     // 当前运行帧序列号
    ar &s.vehicle_position_x;   // 车辆x位置 m
    ar &s.vehicle_position_y;   // 车辆y位置 m
    ar &s.vehicle_theta;        // 车辆方向角 度
    ar &s.vehicle_speed;        // 车辆线速度 km/h
    ar &s.vehicle_torque;       // 车辆扭矩 Nm
    ar &s.vehicle_deceleration; // 车辆减速度 m/s^2
    ar &s.steer_wheel_speed;    // 方向盘转角速度 度/s
    ar &s.steer_wheel_angle;    // 方向盘转角 度
    ar &s.vehicle_yaw_rate;     // 车辆摆转角速度 度/s

    ar &s.canvas_size; // 图像字节长度
    ar &s.data_encode; // 图像编码

    ar &s.stop_size;          // 停靠站数量
    ar &s.stop_passed_status; // 停靠站经过状态
}
template <class Archive> void serialize(Archive &ar, stella_type::StellaMap &m, const uint32_t version)
{
    // 指定哪些属性和数据需要序列化
    ar &m.name;   // 地图名称
    ar &m.routes; // 路线
}
template <class Archive> void serialize(Archive &ar, stella_type::StellaRoute &m, const uint32_t version)
{
    // 指定哪些属性和数据需要序列化
    ar &m.name;          // 路线名称
    ar &m.start_lat;     // 起点纬度
    ar &m.start_lon;     // 起点经度
    ar &m.end_lat;       // 终点纬度
    ar &m.end_lon;       // 终点经度
    ar &m.stop_stations; // 路线点
}
template <class Archive> void serialize(Archive &ar, stella_type::StellaStopStation &m, const uint32_t version)
{
    // 指定哪些属性和数据需要序列化
    ar &m.name; // 停靠站名称
    ar &m.lat;  // 停靠站纬度
    ar &m.lon;  // 停靠站经度
}
template<class Archive> void serialize(Archive &ar, stella_type::StellaSocketSendMsg &s, const uint32_t version)
{
    // 指定哪些成员变量需要序列化
    ar &s.timestamp; // 时间戳
    ar &s.frame_id;  // 帧序列号
    ar &s.scenario_name; // 场景类型
    ar &s.route_name;    // 路径类型
    ar &s.stop_size;          // 停靠站数量
    ar &s.stop_enable_status; // 停靠站启用状态
    ar &s.start_stella_system; // 启动自动驾驶系统
    ar &s.stop_stella_system;  // 停止自动驾驶系统
    ar &s.exit_stella_system;  // 退出自动驾驶系统
    ar &s.enable_auto_drive; // 启用自动驾驶
    ar &s.close_the_door;    // 是否关门
    ar &s.vehicle_scenario_type; // 车辆运行场景 0-正常模式 1-紧急停车 2-紧急停靠
}
} // namespace serialization
} // namespace boost

namespace stella_host
{
inline char* struct_to_char(const stella_type::StellaSocketSendMsg &s, int32_t &size)
{
    // 创建一个ostringstream对象，用来存储序列化后的数据
    std::ostringstream oss;
    {
        // 创建一个binary_oarchive对象，用来将结构体写入ostringstream对象
        boost::archive::text_oarchive oa(oss);
        // 使用<<运算符将结构体写入
        oa << s;
    }
    // 从ostringstream对象中获取std::string对象
    std::string str = oss.str();
    // 将size设置为std::string对象的大小
    size = str.size();
    // 分配内存空间给char*指针，并且使用memcpy函数将std::string对象中的数据拷贝到char*指针中
    char *data = new char[size];
    memcpy(data, str.data(), size);
    // 返回char*指针
    return data;
}

//序列化转化函数
inline stella_type::StellaSocketReceiveMsg char_to_struct(const char *data, size_t size)
{
    // 创建一个MyStruct对象，用来存储反序列化后的数据
    stella_type::StellaSocketReceiveMsg s;
    // 创建一个std::string对象，用来存储char*指针中的数据
    std::string str(data, size);
    // 创建一个istringstream对象，用来读取std::string对象中的数据
    std::istringstream iss(str);
    {
        // 创建一个binary_iarchive对象，用来从istringstream对象中读取结构体
        boost::archive::text_iarchive ia(iss);
        // 使用>>运算符将结构体读取
        ia >> s;
    }
    // 返回MyStruct对象
    return s;
}

inline void str_to_struct(const std::string &str, stella_type::StellaSocketReceiveData &receive_data)
{
    std::stringstream ss;
    ss << str;
    {
        boost::archive::text_iarchive ia(ss);
        ia >> receive_data;
    }
}

inline void struct_to_str(const stella_type::StellaSocketSendData &send_data, std::string &str)
{
    std::stringstream ss;
    {
        boost::archive::text_oarchive oa(ss);
        oa << send_data;
    }
    str = ss.str();
}
} // namespace stella_host
