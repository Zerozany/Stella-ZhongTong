#ifndef WIDGET_H
#define WIDGET_H

#include <QtConcurrent>
#include <QFuture>
#include <QObject>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QLCDNumber>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QThread>
#include <QString>
#include <QCheckBox>
#include <QToolButton>
#include <QMenu>
#include <QDataStream>
#include "sender.h"
#include "receive.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void send_msg_signal(stella_type::StellaSocketSendMsg);

public slots:
    void Route_planning_start_slots();
    void Route_planning_end_slots();
    void exit_app_slots();
    void updateUI(stella_type::StellaSocketReceiveMsg);
    void scenario_route();
    void route_station();
    void recv_vehicle_stop();
    void vehicle_scenario_type_1_run();
    void vehicle_scenario_type_2_run();
//    void vehicle_scenario_type_3_run();
    void play_video();
    void play_LCD();


private:
    QTimer *timer;
    QTimer *timer_LCD;
    QTime *time;
    //容器布局
    QGridLayout *Route_planning_groupbox_layout;
    QFormLayout *Vehicle_status_groupbox_formlayout;
    QHBoxLayout *Vehicle_status_groupbox_HBoxLayout;
    QVBoxLayout *Vehicle_status_groupbox_VBoxLayout;
    QGridLayout *stations_infomations_groupbox_gridLayout;
    //框架
    QFrame *stella_introduction_frame;
    QGroupBox *Route_planning_groupbox;
    QGroupBox *stations_infomations_groupbox;
    QGroupBox *Vehicle_status_groupbox;
    QProgressBar *app_Progressbar;
    QLabel *video_label_ground;
    QLabel *video_label;
    QTextEdit *log_TextEdit;
    //组件(stella_introduction_frame)
    QLabel *stella_introduction_chinese_name;
    QLabel *stella_introduction_english_name;
    QLabel *stella_introduction_line;
    QLabel *stella_introduction_logo;
    //组件(Route_planning_groupbox)
    QLabel *Route_planning_scenario_photo;
    QLabel *Route_planning_scenario_tips;
    QComboBox *Route_planning_scenario_QComboBox;
    QLabel *Route_planning_routes_photo;
    QLabel *Route_planning_routes_tips;
    QComboBox *Route_planning_routes_QComboBox;
    QLabel *Route_planning_park_photo;
    QLabel *Route_planning_park_tips;
    QToolButton *checkbutton;
    QMenu *park_menu;
    QAction *parks;
    QCheckBox *checkBox;
    QPushButton *Route_planning_start_button;
    QPushButton *Exit_application_button;
    //组件(stations_infomations_groupbox)
    QLCDNumber *stations_infomations_time;
    //组件(Vehicle_status_groupbox)
    QLabel *Vehicle_status_runframeID_tips;
    QLabel *Vehicle_status_runframeID;
    QLabel *Vehicle_status_position_x_tips;
    QLabel *Vehicle_status_position_y_tips;
    QLabel *Vehicle_status_position_x_value;
    QLabel *Vehicle_status_position_y_value;
    QLabel *Vehicle_status_vehicle_theta_tips;
    QLabel *Vehicle_status_vehicle_theta_value;
    QLabel *Vehicle_status_vehicle_speed_tips;
    QLabel *Vehicle_status_vehicle_speed_value;
    QLabel *Vehicle_status_vehicle_torque_tips;
    QLabel *Vehicle_status_vehicle_torque_value;
    QLabel *Vehicle_status_vehicle_deceleration_tips;
    QLabel *Vehicle_status_vehicle_deceleration_value;
    QLabel *vehicle_Steering_wheel_angular_speed_tips;
    QLabel *vehicle_Steering_wheel_angular_speed_value;
    QLabel *Vehicle_status_Steering_wheel_corners_tips;
    QLabel *Vehicle_status_Steering_wheel_corners_value;
    QLabel *Vehicle_status_Swing_angle_speed_tips;
    QLabel *Vehicle_status_Swing_angle_speed_value;
    QPushButton *Vehicle_status_Emergency_stop_button;
    QPushButton *Vehicle_status_Automatic_parking_button;
    QPushButton *Vehicle_status_door_opening_button;
    QPushButton *Vehicle_status_exit_button;
    // QPushButton *Vehicle_Continue_execution_button;
    class sender senders;
    receive receiver;
    QFuture<void> vehicle_scenario_type_1_future;
    QFuture<void> vehicle_scenario_type_2_future;
//    QFuture<void> vehicle_scenario_type_3_future;

private:
    ///////////////////////UI信息(private)//////////////////////
    inline void window_head();
    inline void window_page_message();
    inline void page_elements();
    inline void page_text_message();
    inline void UIpage_style();
    inline void elements_layout();
    inline void elements_size();
    ///////////////////////UI信息(private)end//////////////////////
    inline void Vehicle_status_hide();      //车辆状态控制按钮消失
    inline void Vehicle_status_show();      //车辆状态控制按钮展开
    void app_Progressbar_moudle(int time);  //进度条
    void cin_logTetx(std::string log);      //追加日志
    void resizeEvent(QResizeEvent *event);  //GUI界面伸缩比
    void VEHICLE_BOOL(int vehicle_control_select);  //车辆状态控制逻辑
    void stations_bool();                   //站点选择统计
    void bind_singlas_slots();              //绑定信号和槽
    void wait_vehicle_stop();

private:
    static stella_type::StellaSocketReceiveMsg update_msg;
    static stella_type::StellaSocketSendMsg send_msg;
    static std::map<QString,QStringList> m1_maps;
    static std::map<QString,QStringList> m2_maps;
    static int park_numbers;    //size停靠点数量
    static std::vector<bool> bool_parks;
    static bool scenario_route_bool;
    static bool route_station_bool;
    static bool station_status_bool;
    static bool is_map_initialized;
    static bool is_started;
    static bool is_station_grid_layout_initialized;
};
#endif // WIDGET_H
