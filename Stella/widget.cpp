#include "widget.h"
#include "qapplication.h"

stella_type::StellaSocketReceiveMsg Widget::update_msg{};
stella_type::StellaSocketSendMsg Widget::send_msg;
std::map<QString,QStringList> Widget::m1_maps;
std::map<QString,QStringList> Widget::m2_maps;
int Widget::park_numbers = 0;
std::vector<bool> Widget::bool_parks;
bool Widget::scenario_route_bool = true;
bool Widget::route_station_bool = true;
bool Widget::station_status_bool = true;
bool Widget::is_map_initialized = false;
bool Widget::is_started = false;
bool Widget::is_station_grid_layout_initialized = false;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    //////////////////////////////////////UI控件布局.start/////////////////////////////////////////////
    //窗口头信息
    window_head();
    //窗口页面信息
    window_page_message();
    //控件信息
    page_elements();
    //页面文本
    page_text_message();
    //页面组件风格
    UIpage_style();
    //页面布局
    elements_layout();
    //设置部分控件初始化大小
    elements_size();
    //设置车辆状态初始状态
    Vehicle_status_hide();
    //绑定信号和槽
    bind_singlas_slots();
    //启动接收服务器数据
    receiver.start();
}


/// 开始执行计划
void Widget::Route_planning_start_slots()
{
    send_msg = {0};
    cin_logTetx("已为您规划好路线，程序即将开启执行~");
    stations_bool();
    send_msg.timestamp = update_msg.timestamp;
    send_msg.frame_id = update_msg.frame_id;
    send_msg.scenario_name = Route_planning_scenario_QComboBox->currentText().toStdString();
    send_msg.route_name = Route_planning_routes_QComboBox->currentText().toStdString();
    send_msg.stop_size = park_numbers;
    send_msg.stop_enable_status = bool_parks;
    send_msg.start_stella_system = true;
    send_msg.stop_stella_system = false;
    send_msg.exit_stella_system = false;
    send_msg.enable_auto_drive = true;
    send_msg.close_the_door = false;
    app_Progressbar_moudle(20);
    emit send_msg_signal(send_msg);
    senders.start();
    timer_LCD->start(1000);
    timer->start(50); // 每33毫秒刷新一次，约30帧每秒
    is_started = true;
    Vehicle_status_show();
}


/// 路线规划执行结束
void Widget::Route_planning_end_slots()
{
    Vehicle_status_Automatic_parking_button->setDisabled(true);
    Vehicle_status_exit_button->setDisabled(true);
    Vehicle_status_door_opening_button->setDisabled(true);
    QFuture<void> future = QtConcurrent::run(&Widget::recv_vehicle_stop, this);
    while (!future.isFinished()) {
        QApplication::processEvents(QEventLoop::AllEvents);
    }
    if (future.isFinished())
    {
        app_Progressbar_moudle(200);
        send_msg.stop_stella_system = true;
        send_msg.start_stella_system = false;
        send_msg.vehicle_scenario_type = 0;
        emit send_msg_signal(send_msg);
        QString plan_end_tips = "已退出自动驾驶系统，任务执行时长：";
        QString log_time = time->toString("hh:mm:ss");
        cin_logTetx(plan_end_tips.toStdString() + log_time.toStdString());
        timer_LCD->stop();
        time->setHMS(0,0,0);
        stations_infomations_time->display("00:00:00");
        Vehicle_status_hide();
        is_started=false;
        Vehicle_status_Automatic_parking_button->setDisabled(false);
        Vehicle_status_exit_button->setDisabled(false);
        Vehicle_status_door_opening_button->setDisabled(false);
        QThread::msleep(100);
    }
}


///退出程序
void Widget::exit_app_slots()
{
    send_msg.exit_stella_system = true;
    emit send_msg_signal(send_msg);
    qDebug() << "recv_thread is end";
    QThread::msleep(500);
    this->close();
}


///通过地图选择路线
void Widget::scenario_route()
{
    Route_planning_routes_QComboBox->clear();
    QString select_scenario = Route_planning_scenario_QComboBox->currentText();
    Route_planning_routes_QComboBox->addItems(m1_maps[select_scenario]);
}


///通过路线选择站点
void Widget::route_station()
{
    QString select_route = Route_planning_routes_QComboBox->currentText();
    park_menu->clear();
    std::vector<bool>().swap(bool_parks);
    for(const auto &stations:m2_maps[select_route])
    {
        parks = new QAction(stations, this);
        parks->setCheckable(true);
        park_menu->addAction(parks);
    }
    checkbutton->setMenu(park_menu);
    send_msg.stop_enable_status = bool_parks;
}


/// 重新规划槽函数
void Widget::recv_vehicle_stop()
{
    if(QString::number(update_msg.vehicle_speed).toDouble() < 0.1)
    {
        return;
    }
    send_msg.vehicle_scenario_type = 2;
    emit send_msg_signal(send_msg);
    cin_logTetx("计划执行结束，即将靠边停车");
    while (true) {
        if(QString::number(update_msg.vehicle_speed).toDouble() < 0.1)
        {
            break;
        }
        QThread::msleep(100);
    }
}


void Widget::play_video()
{
    cv::Mat frame; // 获取视频帧
    if (!update_msg.data_encode.empty())
    {
        cv::imdecode(update_msg.data_encode, cv::IMREAD_COLOR, &frame);
    }
    if (!frame.empty())
    {
        cv::Mat q_frame;
        cv::cvtColor(frame, q_frame, cv::COLOR_BGR2RGB); // 转换颜色通道顺序
        QImage image(q_frame.data, q_frame.cols, q_frame.rows, q_frame.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        video_label->setPixmap(pixmap.scaled(video_label->size(), Qt::KeepAspectRatio));
        video_label->setScaledContents(true);
    }
}

void Widget::play_LCD()
{
    *time = time->addSecs(1);
    stations_infomations_time->display(time->toString("hh:mm:ss"));
}


///获取停靠点状态
void Widget::stations_bool()
{
    std::vector<bool>().swap(bool_parks);
    for(QAction* action : park_menu->actions())
    {
        bool_parks.push_back(action->isChecked());
    }
}


///绑定信号和槽
void Widget::bind_singlas_slots()
{
    QObject::connect(&receiver, &receive::receive_data_update, this, &Widget::updateUI);
    connect(Route_planning_scenario_QComboBox,&QComboBox::currentTextChanged,this,&Widget::scenario_route);        //地图到路线
    connect(Route_planning_routes_QComboBox,&QComboBox::currentTextChanged,this,&Widget::route_station);            //路线到站点
    connect(Route_planning_start_button,&QPushButton::clicked,this,&Widget::Route_planning_start_slots);    //执行计划
    connect(Vehicle_status_Emergency_stop_button,&QPushButton::clicked,[=](){VEHICLE_BOOL(1);});            //紧急停车
    connect(Vehicle_status_Automatic_parking_button,&QPushButton::clicked,[=](){VEHICLE_BOOL(2);});         //靠边停车
    connect(Vehicle_status_door_opening_button,&QPushButton::clicked,[=](){VEHICLE_BOOL(3);});              //关闭车门
    connect(Vehicle_status_exit_button,&QPushButton::clicked,this,&Widget::Route_planning_end_slots);       //终止程序执行
    connect(Exit_application_button,&QPushButton::clicked,this, &Widget::exit_app_slots);                   //关闭应用
    connect(this,&Widget::send_msg_signal,&senders,&sender::send_to_struct);
    connect(timer, &QTimer::timeout,this,&Widget::play_video);
    connect(timer_LCD,&QTimer::timeout,this,&Widget::play_LCD);
}


void Widget::updateUI(stella_type::StellaSocketReceiveMsg def_update_msg)
{
    update_msg =  def_update_msg;
    update_msg.timestamp = def_update_msg.timestamp;
    update_msg.frame_id = def_update_msg.frame_id;

    uint64_t current_frame_id = update_msg.current_frame_id;
    QString current_frame_id_str = QString::number(current_frame_id);
    QString current_frame_id_print = "<font color='blue'>" + current_frame_id_str + "</font>";
    Vehicle_status_runframeID->setText(current_frame_id_print);

    double vehicle_position_x = update_msg.vehicle_position_x;
    QString vehicle_position_x_str = QString::number(vehicle_position_x);
    QString vehicle_position_x_unit = " m";
    QString vehicle_position_x_print = "<font color='blue'>" + vehicle_position_x_str + "</font>" + "<font color='black'>" + vehicle_position_x_unit + "</font>";
    Vehicle_status_position_x_value->setText(vehicle_position_x_print);

    double vehicle_position_y = update_msg.vehicle_position_y;
    QString vehicle_position_y_str = QString::number(vehicle_position_y);
    QString vehicle_position_y_unit = " m";
    QString vehicle_position_y_print = "<font color='blue'>" + vehicle_position_y_str + "</font>" + "<font color='black'>" + vehicle_position_y_unit + "</font>";
    Vehicle_status_position_y_value->setText(vehicle_position_y_print);

    double Vehicle_status_vehicle_theta = update_msg.vehicle_theta;
    QString Vehicle_status_vehicle_theta_value_str = QString::number(Vehicle_status_vehicle_theta);
    QString Vehicle_status_vehicle_theta_degrees = " 度";
    QString Vehicle_status_vehicle_theta_print = "<font color='blue'>" + Vehicle_status_vehicle_theta_value_str + "</font>" + "<font color='black'>" + Vehicle_status_vehicle_theta_degrees + "</font>";
    Vehicle_status_vehicle_theta_value->setText(Vehicle_status_vehicle_theta_print);

    double Vehicle_status_vehicle_speed = update_msg.vehicle_speed;
    QString Vehicle_status_vehicle_speed_value_str = QString::number(Vehicle_status_vehicle_speed);
    QString Vehicle_status_vehicle_speed_unit = " km/h";
     QString Vehicle_status_vehicle_speed_print = "<font color='blue'>" + Vehicle_status_vehicle_speed_value_str + "</font>" + "<font color='black'>" + Vehicle_status_vehicle_speed_unit + "</font>";
    Vehicle_status_vehicle_speed_value->setText(Vehicle_status_vehicle_speed_print);

    double Vehicle_status_vehicle_torque = update_msg.vehicle_torque;
    QString Vehicle_status_vehicle_torque_value_str = QString::number(Vehicle_status_vehicle_torque);
    QString Vehicle_status_vehicle_torque_unit = " N.m";
    QString Vehicle_status_vehicle_torque_print = "<font color='blue'>" + Vehicle_status_vehicle_torque_value_str + "</font>" + "<font color='black'>" + Vehicle_status_vehicle_torque_unit + "</font>";
    Vehicle_status_vehicle_torque_value->setText(Vehicle_status_vehicle_torque_print);

    double Vehicle_status_vehicle_deceleration = update_msg.vehicle_deceleration;
    QString Vehicle_status_vehicle_deceleration_value_str = QString::number(Vehicle_status_vehicle_deceleration);
    QString Vehicle_status_vehicle_deceleration_unit = " m/s^2";
    QString Vehicle_status_vehicle_deceleration_print = "<font color='blue'>" + Vehicle_status_vehicle_deceleration_value_str + "</font>" + "<font color='black'>" + Vehicle_status_vehicle_deceleration_unit + "</font>";
    Vehicle_status_vehicle_deceleration_value->setText(Vehicle_status_vehicle_deceleration_print);

    double vehicle_Steering_wheel_angular_speed = update_msg.steer_wheel_speed;
    QString vehicle_Steering_wheel_angular_speed_value_str = QString::number(vehicle_Steering_wheel_angular_speed);
    QString vehicle_Steering_wheel_angular_speed_unit = " 度/s";
     QString vehicle_Steering_wheel_angular_speed_print = "<font color='blue'>" + vehicle_Steering_wheel_angular_speed_value_str + "</font>" + "<font color='black'>" + vehicle_Steering_wheel_angular_speed_unit + "</font>";
    vehicle_Steering_wheel_angular_speed_value->setText(vehicle_Steering_wheel_angular_speed_print);

    double Vehicle_status_Steering_wheel_corners = update_msg.steer_wheel_angle;
    QString Vehicle_status_Steering_wheel_corners_value_str = QString::number(Vehicle_status_Steering_wheel_corners);
    QString Vehicle_status_Steering_wheel_corners_unit = " 度";
     QString Vehicle_status_Steering_wheel_corners_print = "<font color='blue'>" + Vehicle_status_Steering_wheel_corners_value_str + "</font>" + "<font color='black'>" + Vehicle_status_Steering_wheel_corners_unit + "</font>";
    Vehicle_status_Steering_wheel_corners_value->setText(Vehicle_status_Steering_wheel_corners_print);

    double Vehicle_status_Swing_angle_speed = update_msg.vehicle_yaw_rate;
    QString Vehicle_status_Swing_angle_speed_value_str = QString::number(Vehicle_status_Swing_angle_speed);
    QString Vehicle_status_Swing_angle_speed_unit = " 度/s";
    QString Vehicle_status_Swing_angle_speed_print = "<font color='blue'>" + Vehicle_status_Swing_angle_speed_value_str + "</font>" + "<font color='black'>" + Vehicle_status_Swing_angle_speed_unit + "</font>";
    Vehicle_status_Swing_angle_speed_value->setText(Vehicle_status_Swing_angle_speed_print);
    ///------------------------------------------------------------------------------------------------
    if (!is_map_initialized && !update_msg.maps.empty())
    {
        for(const auto &elem_map:update_msg.maps)
        {
            QString scenario = QString::fromStdString(elem_map.name);
            if(Route_planning_scenario_QComboBox->findText(scenario) == -1)
            {
                Route_planning_scenario_QComboBox->addItem(scenario);
                Route_planning_scenario_QComboBox->setCurrentIndex(0);
            }
            QStringList routes_list;
            for(const auto &elem_routes:elem_map.routes)
            {
                QString routes = QString::fromStdString(elem_routes.name);
                routes_list.append(routes);
                QStringList stations_list;
                for(const auto &elem_stations:elem_routes.stop_stations)
                {
                    QString stations =QString::fromStdString(elem_stations.name);
                    stations_list.append(stations);
                }
                m2_maps[routes] = {stations_list};
            }
            m1_maps[scenario] = {routes_list};
        }
        Route_planning_routes_QComboBox->addItems(m1_maps.begin()->second);
        Route_planning_routes_QComboBox->setCurrentIndex(0);
        park_menu->clear();
        park_numbers = 0;
        for(const auto &station: m2_maps[m1_maps.begin()->second[0]])
        {
            parks = new QAction(station, this);
            parks->setCheckable(true);
            park_menu->addAction(parks);
            ++park_numbers;
        }
        checkbutton->setMenu(park_menu);
        send_msg.stop_enable_status = bool_parks;
        is_map_initialized = true;
    }
    if(is_started)
    {
        if (update_msg.stop_passed_status.empty())
        {
            update_msg.stop_passed_status.resize(park_numbers, false);
        }
        for(int32_t i = 0; i < park_numbers;++i)
        {
            if (!is_station_grid_layout_initialized)
            {
                QLabel *stations_label_tips = new QLabel(stations_infomations_groupbox);
                QLabel *stations_label = new QLabel(stations_infomations_groupbox);
                stations_label->setMinimumSize(100,35);
                stations_label->setStyleSheet("font-family:'微软雅黑';font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 red, stop:1 blue);");
                stations_label->setAlignment(Qt::AlignCenter);
                QString select_route = Route_planning_routes_QComboBox->currentText();
                stations_label->setText(m2_maps[select_route].at(i));
                if(!bool_parks[i])
                {
                    stations_label_tips->setPixmap(QPixmap(":/image/Icons8-Windows-8-Arrows-Down.16.png"));
                    stations_label->setText(m2_maps[select_route].at(i));
                }
                else
                {
                    stations_label_tips->setPixmap(QPixmap(":/image/Custom-Icon-Design-Flat-Cute-Arrows-Arrow-Down-1.16.png"));
                    stations_label->setText(m2_maps[select_route].at(i));
                }
                if(update_msg.stop_passed_status[i])
                {
                    stations_label_tips->setPixmap(QPixmap(":/image/Custom-Icon-Design-Flat-Cute-Arrows-Arrow-Down.16.png"));
                }
                stations_infomations_groupbox_gridLayout->addWidget(stations_label_tips,2 + i,1,1,1);
                stations_infomations_groupbox_gridLayout->addWidget(stations_label,2 + i,2,1,1);
            }
            else
            {
                auto label_item = (QLabel *)stations_infomations_groupbox_gridLayout->itemAtPosition(2+i,1)->widget();
                if(!bool_parks[i])
                {
                    label_item->setPixmap(QPixmap(":/image/Icons8-Windows-8-Arrows-Down.16.png"));
                }
                else
                {
                    label_item->setPixmap(QPixmap(":/image/Custom-Icon-Design-Flat-Cute-Arrows-Arrow-Down-1.16.png"));
                }
                if(update_msg.stop_passed_status[i])
                {
                    label_item->setPixmap(QPixmap(":/image/Custom-Icon-Design-Flat-Cute-Arrows-Arrow-Down.16.png"));
                }
            }
        }
        if (!is_station_grid_layout_initialized)
        {
            is_station_grid_layout_initialized = true;
        }
    }
}


////////////////////////////////////////////////////无修改/////////////////////////////////////////
void Widget::resizeEvent(QResizeEvent *event)   //全局UI控制
{
    int winodw_width = this->width() - 5;
    int winodw_height =this->height() -5;
    stella_introduction_frame->setGeometry(5,5,winodw_width - 1200 - 10,55);
    Route_planning_groupbox->setGeometry(5,60,winodw_width - 1200 - 10,(winodw_height - 60) * 1/3);
    stations_infomations_groupbox->setGeometry(5,60,winodw_width - 1200 - 10,(winodw_height - 60) * 1/3);
    Vehicle_status_groupbox->setGeometry(5,60 + ((winodw_height - 60) * 1/3) + 5,winodw_width - 1200 - 10,winodw_height - stella_introduction_frame->height() - Route_planning_groupbox->height() - app_Progressbar->height() - 15);
    app_Progressbar->setGeometry(5,(stella_introduction_frame->height() + Route_planning_groupbox->height() + Vehicle_status_groupbox->height() + 15),winodw_width - 1200 - 10,20);
    video_label->setGeometry(winodw_width - 1200,5,1200,700);
    video_label_ground->setGeometry(winodw_width - 1200,5,1200,700);
    log_TextEdit->setGeometry(winodw_width - 1200,video_label->height() + 10,1200,winodw_height - video_label->height() - 10);
}

///创建伪进度条
void Widget::app_Progressbar_moudle(int time)
{
    app_Progressbar->setFormat("%p%");
    app_Progressbar->setValue(50);
    QThread::msleep(200);
    int times = time/10 >= 50?30:time/10;
    app_Progressbar->setValue(50 + times);
    QThread::msleep(time);
    app_Progressbar->setValue(100);
    QThread::msleep(200);
    app_Progressbar->setValue(0);
    app_Progressbar->setFormat("🌐");
}

///追加日志
void Widget::cin_logTetx(std::string log)
{
    QDateTime logtext_time = QDateTime::currentDateTime();
    QString logtext = logtext_time.toString("HH:mm:ss") + "👉" + QString::fromStdString(log);
    log_TextEdit->append(logtext);
    log_TextEdit->moveCursor(QTextCursor::End); //insertPlainText
}


///急停，靠边停车，车门控制
void Widget::VEHICLE_BOOL(int vehicle_control_select)
{
    switch (vehicle_control_select)
    {
    case 1:
        if (QString::number(update_msg.vehicle_speed).toDouble() >= 0.1)
        {
            Vehicle_status_Emergency_stop_button->setDisabled(true);
            Vehicle_status_Automatic_parking_button->setDisabled(true);
            Vehicle_status_door_opening_button->setDisabled(true);
            // Vehicle_Continue_execution_button->setDisabled(true);
            Vehicle_status_exit_button->setDisabled(true);
            vehicle_scenario_type_1_future = QtConcurrent::run(&Widget::vehicle_scenario_type_1_run,this);
            while(!vehicle_scenario_type_1_future.isFinished())
            {
                QApplication::processEvents(QEventLoop::AllEvents);
            }
            if(vehicle_scenario_type_1_future.isFinished())
            {
                Vehicle_status_Emergency_stop_button->setDisabled(false);
                Vehicle_status_Automatic_parking_button->setDisabled(false);
                Vehicle_status_door_opening_button->setDisabled(false);
                // Vehicle_Continue_execution_button->setDisabled(false);
                Vehicle_status_exit_button->setDisabled(false);
            }
        }
        else
        {
            cin_logTetx("车辆状态未在运行中，请启动车辆~");
        }
        break;
    case 2:
        if(QString::number(update_msg.vehicle_speed).toDouble() >= 0.1)
        {
            Vehicle_status_Automatic_parking_button->setDisabled(true);
            Vehicle_status_door_opening_button->setDisabled(true);
            // Vehicle_Continue_execution_button->setDisabled(true);
            Vehicle_status_exit_button->setDisabled(true);
            vehicle_scenario_type_2_future = QtConcurrent::run(&Widget::vehicle_scenario_type_2_run,this);
            while(!vehicle_scenario_type_2_future.isFinished())
            {
                QApplication::processEvents(QEventLoop::AllEvents);
            }
            if(vehicle_scenario_type_2_future.isFinished())
            {
                Vehicle_status_Automatic_parking_button->setDisabled(false);
                Vehicle_status_door_opening_button->setDisabled(false);
                // Vehicle_Continue_execution_button->setDisabled(false);
                Vehicle_status_exit_button->setDisabled(false);
            }
        }
        else
        {           
            cin_logTetx("车辆状态未在运行中，请启动车辆~");
        }
        break;
    default:
        if(QString::number(update_msg.vehicle_speed).toDouble() < 0.1)
        {
            cin_logTetx("车门即将关闭~");
            send_msg.close_the_door = true;
            send_msg.vehicle_scenario_type = 3;
            emit send_msg_signal(send_msg);
            app_Progressbar_moudle(10);
            send_msg.close_the_door = false;
            send_msg.vehicle_scenario_type = 0;
            emit send_msg_signal(send_msg);
        }
        else
        {
            cin_logTetx("车门未打开！");
        }
        break;
    }
}


void Widget::vehicle_scenario_type_1_run()
{
    cin_logTetx("注意：车辆即将采取紧急制动！");
    send_msg.vehicle_scenario_type = 1;
    emit send_msg_signal(send_msg);
    int flag = 1;
    while(flag)
    {
        if(QString::number(update_msg.vehicle_speed).toDouble() < 0.1)
        {
            flag = 0;
        }
        QThread::msleep(100);
    }
}

void Widget::vehicle_scenario_type_2_run()
{
    cin_logTetx("即将靠边停车");
    send_msg.vehicle_scenario_type = 2;
    emit send_msg_signal(send_msg);
    int flag = 1;
    while(flag)
    {
        if(QString::number(update_msg.vehicle_speed).toDouble() < 0.1)
        {
            flag = 0;
        }
        QThread::msleep(100);
    }
}





Widget::~Widget()
{
//    receiver->quit();
}

////////////////////////////////////PRIVATE:页面UI信息(无法变动)////////////////////////////////////////////
void Widget::window_head()
{
    this->resize(1650,930);
    this->setWindowTitle("StellaADAS@GUI");
    this->setWindowIcon(QIcon(":/image/255afd1b324bc91652024e1940a7ae3.png"));
}

void Widget::window_page_message()
{
    stella_introduction_frame = new QFrame(this);
    Route_planning_groupbox = new QGroupBox("路线规划",this);
    stations_infomations_groupbox = new QGroupBox("站点信息",this);
    Vehicle_status_groupbox = new QGroupBox("车辆状态",this);
    app_Progressbar = new QProgressBar(this);
    video_label = new QLabel(this);
    video_label_ground = new QLabel(this);
    log_TextEdit = new QTextEdit(this);
}

void Widget::page_elements()
{
    timer = new QTimer;
    timer_LCD = new QTimer;
    time = new QTime(0,0,0);
    //组件(stella_introduction_frame)
    stella_introduction_chinese_name = new QLabel(stella_introduction_frame);
    stella_introduction_english_name = new QLabel(stella_introduction_frame);
    stella_introduction_line = new QLabel(stella_introduction_frame);
    stella_introduction_logo = new QLabel(stella_introduction_frame);
    //组件(Route_planning_groupbox)
    Route_planning_scenario_photo = new QLabel(Route_planning_groupbox);
    Route_planning_scenario_tips = new QLabel(Route_planning_groupbox);
    Route_planning_scenario_QComboBox = new QComboBox(Route_planning_groupbox);
    Route_planning_routes_photo = new QLabel(Route_planning_groupbox);
    Route_planning_routes_tips = new QLabel(Route_planning_groupbox);
    Route_planning_routes_QComboBox = new QComboBox(Route_planning_groupbox);
    Route_planning_park_photo = new QLabel(Route_planning_groupbox);
    Route_planning_park_tips = new QLabel(Route_planning_groupbox);
    checkbutton = new QToolButton(Route_planning_groupbox);
    park_menu = new QMenu(checkbutton);
    parks = new QAction(park_menu);
    Route_planning_start_button = new QPushButton(Route_planning_groupbox);
    Exit_application_button = new QPushButton(Route_planning_groupbox);
    //组件(stations_infomations_groupbox)
    stations_infomations_time = new QLCDNumber(stations_infomations_groupbox);
    //组件(Vehicle_status_groupbox)
    Vehicle_status_runframeID_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_runframeID = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_position_x_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_position_y_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_position_x_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_position_y_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_theta_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_theta_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_speed_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_speed_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_torque_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_torque_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_deceleration_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_vehicle_deceleration_value = new QLabel(Vehicle_status_groupbox);
    vehicle_Steering_wheel_angular_speed_tips = new QLabel(Vehicle_status_groupbox);
    vehicle_Steering_wheel_angular_speed_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_Steering_wheel_corners_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_Steering_wheel_corners_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_Swing_angle_speed_tips = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_Swing_angle_speed_value = new QLabel(Vehicle_status_groupbox);
    Vehicle_status_Emergency_stop_button = new QPushButton(Vehicle_status_groupbox);
    Vehicle_status_Automatic_parking_button = new QPushButton(Vehicle_status_groupbox);
    Vehicle_status_door_opening_button = new QPushButton(Vehicle_status_groupbox);
    Vehicle_status_exit_button = new QPushButton(Vehicle_status_groupbox);
    // Vehicle_Continue_execution_button = new QPushButton(Vehicle_status_groupbox);
}

void Widget::page_text_message()
{
    //插入提示信息(stella_introduction_frame)
    stella_introduction_chinese_name->setText("星际舟科技");
    stella_introduction_english_name->setText("Stella Technology");
    stella_introduction_line->setText("————————————————————————");
    stella_introduction_logo->setPixmap(QPixmap(":/image/c638f69d8df96a7859a5274115d84d5.png").scaled(QSize(142,71),Qt::KeepAspectRatio));
    //插入提示信息(Route_planning_groupbox)
    Route_planning_scenario_photo->setPixmap(QPixmap(":/image/Pictogrammers-Material-Routes.48.png").scaled(QSize(48,48),Qt::KeepAspectRatio));
    Route_planning_scenario_tips->setText("场景类型：");
    Route_planning_routes_photo->setPixmap(QPixmap(":/image/Pictogrammers-Material-Select-marker.48.png").scaled(QSize(48,48),Qt::KeepAspectRatio));
    Route_planning_routes_tips->setText("行驶路线：");;
    Route_planning_park_photo->setPixmap(QPixmap(":/image/Github-Octicons-Stop-24.48.png").scaled(QSize(48,48),Qt::KeepAspectRatio));
    Route_planning_park_tips->setText("停靠地点：");
    checkbutton->setText("点击选择");
    checkbutton->setPopupMode(QToolButton::InstantPopup);
    Route_planning_start_button->setIcon(QIcon(":/image/Custom-Icon-Design-Pretty-Office-5-Start.64.png"));
    Exit_application_button->setIcon(QIcon(":/image/Microsoft-Fluentui-Emoji-Mono-Stop-Button.64.png"));
    //插入提示信息(stations_infomations_groupbox)
    stations_infomations_time->setDigitCount(8);
    stations_infomations_time->display("00:00:00");
    //插入提示信息(Vehicle_status_groupbox)
    Vehicle_status_runframeID_tips->setText("运行帧：");
    Vehicle_status_position_x_tips->setText("X坐标：");
    Vehicle_status_position_y_tips->setText("Y坐标：");
    Vehicle_status_vehicle_theta_tips->setText("车辆方向角：");;
    Vehicle_status_vehicle_speed_tips->setText("车辆速度：");
    Vehicle_status_vehicle_torque_tips->setText("扭矩：");
    Vehicle_status_vehicle_deceleration_tips->setText("车辆减速度：");
    vehicle_Steering_wheel_angular_speed_tips->setText("方向盘转角速度：");
    Vehicle_status_Steering_wheel_corners_tips->setText("方向盘转角：");
    Vehicle_status_Swing_angle_speed_tips->setText("车辆摆转角速度：");
    Vehicle_status_Emergency_stop_button->setText("紧急\n停车");
    Vehicle_status_Emergency_stop_button->setIcon(QIcon(":/image/Graphicloads-100-Flat-Stop.64.png"));
    Vehicle_status_Automatic_parking_button->setText("靠边\n停车");
    Vehicle_status_Automatic_parking_button->setIcon(QIcon(":/image/Fa-Team-Fontawesome-FontAwesome-Square-Parking.64.png"));
    Vehicle_status_door_opening_button->setText("关闭\n车门");
    Vehicle_status_door_opening_button->setIcon(QIcon(":/image/Pictogrammers-Material-Car-door-lock.64.png"));
    Vehicle_status_exit_button->setText("Exit\n重新规划");
    // Vehicle_Continue_execution_button->setText("继续\n执行");
    // Vehicle_Continue_execution_button->setIcon(QIcon(":/image/Bootstrap-Bootstrap-Bootstrap-skip-start-fill.64.png"));
    //插入提示信息app_Progressbar
    app_Progressbar->setValue(0);
    app_Progressbar->setFormat("🌐");
    //插入提示信息log_TextEdit
    QString company_info = "本程序为昆山星际舟ADAS中控显示系统，详细内容请访问：http://www.stellatechno.com/";
    log_TextEdit->append(company_info);
    log_TextEdit->setReadOnly(true);
}

void Widget::UIpage_style()
{
    //设置风格(stella_introduction_frame)
    video_label_ground->setStyleSheet("background-color:black");
    stella_introduction_chinese_name->setStyleSheet("font-family:'华文彩云';font-size:30px; font-weight: bold;");
    stella_introduction_english_name->setStyleSheet("font-family:'Bodoni MT Black';font-size:15px; font-weight: bold;font-style:italic;");
    //设置风格(Route_planning_groupbox)
    Route_planning_scenario_tips->setStyleSheet("font-family:'微软雅黑';font-size:20px;");
    Route_planning_scenario_QComboBox->setStyleSheet("font-family:'微软雅黑';font-size:19px;");
    Route_planning_routes_tips->setStyleSheet("font-family:'微软雅黑';font-size:20px;");
    Route_planning_routes_QComboBox->setStyleSheet("font-family:'微软雅黑';font-size:19px;");
    Route_planning_park_tips->setStyleSheet("font-family:'微软雅黑';font-size:20px;");
    checkbutton->setStyleSheet("font-family:'微软雅黑';font-size:20px;");
    Vehicle_status_exit_button->setStyleSheet("font-family:'微软雅黑';font-size:18px;color:red;font-weight:bold");
    //设置风格(stations_infomations_groupbox)
    stations_infomations_time->setStyleSheet("border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 red, stop:1 blue);");
    //设置风格(Vehicle_status_groupbox)
    Vehicle_status_runframeID_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_runframeID->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_position_x_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_position_y_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_position_x_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_position_y_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_vehicle_theta_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_vehicle_theta_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_vehicle_speed_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_vehicle_speed_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_vehicle_torque_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_vehicle_torque_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_vehicle_deceleration_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_vehicle_deceleration_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    vehicle_Steering_wheel_angular_speed_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    vehicle_Steering_wheel_angular_speed_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_Steering_wheel_corners_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_Steering_wheel_corners_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    Vehicle_status_Swing_angle_speed_tips->setStyleSheet("font-family:'微软雅黑';font-size:15px;");
    Vehicle_status_Swing_angle_speed_value->setStyleSheet("font-family:'微软雅黑';border:1px solid black;font-size:15px;border: 2px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 green, stop:1 yellow);");
    //设置风格log_TextEdit
    log_TextEdit->setReadOnly(true);
    log_TextEdit->setStyleSheet("border:none;font-family:'微软雅黑';font-size:18px;");
}

void Widget::elements_layout()
{
    //组件布局(stella_introduction_frame)
    stella_introduction_chinese_name->move(5,5);
    stella_introduction_english_name ->move(5,35);
    stella_introduction_line->move(5,46);
    stella_introduction_logo->move(150,-7);
    //组件布局(Route_planning_groupbox)
    Route_planning_groupbox_layout = new QGridLayout(Route_planning_groupbox);
    Route_planning_groupbox_layout->setAlignment(Qt::AlignCenter); // 设置布局内部组件的对齐方式为居中对齐
    Route_planning_groupbox_layout->setColumnMinimumWidth(0, 50);  // 设置第一列的最小宽度为 100
    Route_planning_groupbox_layout->setColumnMinimumWidth(1, 100); // 设置第二列的最小宽度为 200
    Route_planning_groupbox_layout->setColumnMinimumWidth(2, 250); // 设置第三列的最小宽度为 100
    Route_planning_groupbox_layout->setRowMinimumHeight(0, 70);
    Route_planning_groupbox_layout->setRowMinimumHeight(1, 70);
    Route_planning_groupbox_layout->setRowMinimumHeight(2, 70);
    Route_planning_groupbox_layout->addWidget(Route_planning_scenario_photo,0,0);
    Route_planning_groupbox_layout->addWidget(Route_planning_scenario_tips,0,1);
    Route_planning_groupbox_layout->addWidget(Route_planning_scenario_QComboBox,0,2,1,2);
    Route_planning_groupbox_layout->addWidget(Route_planning_routes_photo,1,0);
    Route_planning_groupbox_layout->addWidget(Route_planning_routes_tips,1,1);
    Route_planning_groupbox_layout->addWidget(Route_planning_routes_QComboBox,1,2,1,2);
    Route_planning_groupbox_layout->addWidget(Route_planning_park_photo,2,0);
    Route_planning_groupbox_layout->addWidget(Route_planning_park_tips,2,1);
    Route_planning_groupbox_layout->addWidget(checkbutton,2,2,1,2);
    Route_planning_groupbox_layout->addWidget(Route_planning_start_button,4,1,1,1, Qt::AlignCenter);
    Route_planning_groupbox_layout->addWidget(Exit_application_button,4,2,1,1, Qt::AlignCenter);
    //组件布局(Vehicle_status_groupbox)
    Vehicle_status_groupbox_formlayout = new QFormLayout;
    Vehicle_status_groupbox_HBoxLayout = new QHBoxLayout;
    Vehicle_status_groupbox_VBoxLayout = new QVBoxLayout(Vehicle_status_groupbox);
    Vehicle_status_groupbox_VBoxLayout->addLayout(Vehicle_status_groupbox_formlayout);
    Vehicle_status_groupbox_VBoxLayout->addStretch();
    Vehicle_status_groupbox_VBoxLayout->addLayout(Vehicle_status_groupbox_HBoxLayout);
    Vehicle_status_groupbox_VBoxLayout->addStretch();
    Vehicle_status_groupbox_formlayout->setSpacing(25);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_runframeID_tips,Vehicle_status_runframeID);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_position_x_tips,Vehicle_status_position_x_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_position_y_tips,Vehicle_status_position_y_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_vehicle_theta_tips,Vehicle_status_vehicle_theta_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_vehicle_speed_tips,Vehicle_status_vehicle_speed_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_vehicle_torque_tips,Vehicle_status_vehicle_torque_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_vehicle_deceleration_tips,Vehicle_status_vehicle_deceleration_value);
    Vehicle_status_groupbox_formlayout->addRow(vehicle_Steering_wheel_angular_speed_tips,vehicle_Steering_wheel_angular_speed_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_Steering_wheel_corners_tips,Vehicle_status_Steering_wheel_corners_value);
    Vehicle_status_groupbox_formlayout->addRow(Vehicle_status_Swing_angle_speed_tips,Vehicle_status_Swing_angle_speed_value);
    Vehicle_status_groupbox_HBoxLayout->addWidget(Vehicle_status_Emergency_stop_button);
    Vehicle_status_groupbox_HBoxLayout->addWidget(Vehicle_status_Automatic_parking_button);
    Vehicle_status_groupbox_HBoxLayout->addWidget(Vehicle_status_door_opening_button);
    // Vehicle_status_groupbox_HBoxLayout->addWidget(Vehicle_Continue_execution_button);
    Vehicle_status_groupbox_HBoxLayout->addStretch();
    Vehicle_status_groupbox_HBoxLayout->addWidget(Vehicle_status_exit_button);
    //组件布局(stations_infomations_groupbox)
    stations_infomations_groupbox_gridLayout = new QGridLayout(stations_infomations_groupbox);
    stations_infomations_groupbox_gridLayout->setAlignment(Qt::AlignCenter);
    stations_infomations_groupbox_gridLayout->setSpacing(5);
    stations_infomations_groupbox_gridLayout->addWidget(stations_infomations_time,0,0,1,4);
}

void Widget::elements_size()
{
    Route_planning_start_button->setIconSize(QSize(50,54));
    Exit_application_button->setIconSize(QSize(54,54));
    Vehicle_status_Emergency_stop_button->setIconSize(QSize(40,40));
    Vehicle_status_Automatic_parking_button->setIconSize(QSize(40,40));
    Vehicle_status_door_opening_button->setIconSize(QSize(40,40));
    // Vehicle_Continue_execution_button->setIconSize(QSize(42,42));
    Vehicle_status_exit_button->setMinimumSize(100,60);
    stations_infomations_time->setMinimumSize(300,70);
    stations_infomations_time->setMaximumSize(340,80);
    checkbutton->setMaximumSize(QSize(280,35));
}

void Widget::Vehicle_status_hide()
{
    video_label->hide();
    video_label_ground->show();
    stations_infomations_groupbox->hide();
    Vehicle_status_Emergency_stop_button->hide();
    Vehicle_status_Automatic_parking_button->hide();
    Vehicle_status_door_opening_button->hide();
    Vehicle_status_exit_button->hide();
    // Vehicle_Continue_execution_button->hide();
    Route_planning_groupbox->show();
}

void Widget::Vehicle_status_show()
{
    Route_planning_groupbox->hide();
    stations_infomations_groupbox->show();
    Vehicle_status_Emergency_stop_button->show();
    Vehicle_status_Automatic_parking_button->show();
    Vehicle_status_door_opening_button->show();
    Vehicle_status_exit_button->show();
    // Vehicle_Continue_execution_button->show();
    video_label_ground->hide();
    video_label->show();
}
