function [ratio_delta,t_max] = func_UO_ratio_vx(wheelbase, win_odom,  odom_name, acker_name, imu_name, T_start,T_end,is_fast_motor, C_motor)
%
[t_odom,v_x_car,v_y_car,v_z_car,w_x_car,w_y_car,w_z_car] = importfile_Odom(odom_name);
[t_ackermann,delta] = importfile_ackermann(acker_name);
[t_imu, acc_x_imu, acc_y_imu, acc_z_imu, gyro_x_imu, gyro_y_imu, gyro_z_imu] = import_car_txt(imu_name);
%% adjust start time and add filter
[t_imu,t_odom, acc_x_imu, acc_y_imu, acc_z_imu, gyro_x_imu, gyro_y_imu, gyro_z_imu, v_x_car] = PlotFunc.imuOdomtoSameLength(t_imu,t_odom, acc_x_imu, acc_y_imu, acc_z_imu, gyro_x_imu, gyro_y_imu, gyro_z_imu, v_x_car);
[t_ackermann, delta] = PlotFunc.interpAckermann(t_imu, t_ackermann, delta);


T_end_n = floor(T_end*50);
T_start_n = floor(T_start*50);


if is_fast_motor
    delta = -delta;
end

t_odom = (t_odom - t_odom(1))/1000;
t_imu = (t_imu - t_imu(1))/1000;
t_ackermann = (t_ackermann - t_ackermann(1))/1000;

t_max = max(t_odom);

v_x_car = lowpass(v_x_car, 0.1, 50)*C_motor;
v_x_car = movmean(v_x_car, win_odom);
gyro_z_imu = lowpass(gyro_z_imu, 0.1, 50);
gyro_z_imu = movmean(gyro_z_imu, win_odom);

delta_n = neutralSteer(v_x_car(T_start_n:T_end_n),...
    -gyro_z_imu(T_start_n:T_end_n).*v_x_car(T_start_n:T_end_n), wheelbase);

ratio_delta = delta(T_start_n:T_end_n)./ delta_n;

plot(t_odom(T_start_n:T_end_n), ratio_delta)
ylim([-1.3 1.3])
end

