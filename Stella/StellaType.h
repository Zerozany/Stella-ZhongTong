#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace stella_type
{
struct StellaSocketSendMsg
{
    double timestamp = 0.0; // 时间戳
    uint64_t frame_id = 0;  // 帧序列号

    std::string scenario_name = ""; // 场景类型
    std::string route_name = "";    // 路径类型

    uint32_t stop_size = 0;                    // 停靠站数量
    std::vector<bool> stop_enable_status = {}; // 停靠站启用状态

    bool start_stella_system = false; // 启动自动驾驶系统
    bool stop_stella_system = false;  // 停止自动驾驶系统
    bool exit_stella_system = false;  // 退出自动驾驶系统

    bool enable_auto_drive = false; // 启用自动驾驶
    bool close_the_door = false;    // 是否关门

    uint16_t vehicle_scenario_type = 0; // 车辆运行场景 0-正常模式 1-紧急停车 2-紧急停靠
};

struct StellaStopStation
{
    std::string name = ""; // 停靠站名称
    double lat = 0.0;      // 纬度
    double lon = 0.0;      // 经度
};

struct StellaRoute
{
    std::string name = "";                             // 路线名称
    double start_lat = 0.0;                            // 起点纬度
    double start_lon = 0.0;                            // 起点经度
    double end_lat = 0.0;                              // 终点纬度
    double end_lon = 0.0;                              // 终点经度
    std::vector<StellaStopStation> stop_stations = {}; // 停靠站
};

struct StellaMap
{
    std::string name = "";                // 地图名称
    std::vector<StellaRoute> routes = {}; // 路线信息
};

enum class tcp_command
{
    heartbeat = 0, // 心跳
    regist,        // 注册
    normal         // 正常
};

struct StellaSocketReceiveMsg
{
    double timestamp = 0.0; // 时间戳
    uint64_t frame_id = 0;  // 帧序列号

    std::vector<StellaMap> maps; // 地图信息

    uint64_t current_frame_id = 0;   // 当前运行帧序列号
    double vehicle_position_x = 0.0; // 车辆x位置 m

    double vehicle_position_y = 0.0;   // 车辆y位置 m
    double vehicle_theta = 0.0;        // 车辆方向角 度
    double vehicle_speed = 0.0;        // 车辆线速度 km/h
    double vehicle_torque = 0.0;       // 车辆扭矩 Nm
    double vehicle_deceleration = 0.0; // 车辆减速度 m/s^2
    double steer_wheel_speed = 0.0;    // 方向盘转角速度 度/s
    double steer_wheel_angle = 0.0;    // 方向盘转角 度
    double vehicle_yaw_rate = 0.0;     // 车辆摆转角速度 度/s

    uint32_t canvas_size = 0; // 图像字节长度
    // cv::Mat canvas_img;       // 图像
    std::vector<uchar> data_encode; // 图像编码

    uint32_t stop_size = 0;               // 停靠站数量
    std::vector<bool> stop_passed_status; // 停靠站经过状态
};

struct StellaSocketSendData
{
    tcp_command command = tcp_command::normal; // 命令
};

struct StellaSocketReceiveData
{
    tcp_command command = tcp_command::normal; // 命令
    StellaSocketReceiveMsg msg;                // 消息
};
} // namespace stella_type
