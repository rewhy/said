function [output, t_max] = func_plot_LTR_vx(h,track, win_phone, win_odom, is_plot_adjust, odom_name, acker_name, imu_name, T_move, T_start,T_end,is_fast_motor, C_motor, Y_lim_left, Y_lim_right, Y_ticks_left, Y_ticks_right, str_acc_brk)

%% Load data
% [acc_x_mobile, acc_y_mobile, acc_z_mobile, gyro_x_mobile, gyro_y_mobile, gyro_z_mobile, t_mobile] = import_mobile_file(mobile_name);
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

% t_mobile = t_mobile/1000;
t_mobile = t_imu;

t_max = max(t_odom);

if is_plot_adjust
%     plot_adjust
%     title('before adjust (gyro z)')
end

% [t_mobile, acc_x_mobile, acc_y_mobile, acc_z_mobile, gyro_x_mobile, gyro_y_mobile, gyro_z_mobile] = PlotFunc.adjust_start_time(T_move, t_mobile, acc_x_mobile, acc_y_mobile, acc_z_mobile, gyro_x_mobile, gyro_y_mobile, gyro_z_mobile);


if is_plot_adjust
%     plot_adjust
    plot(t_imu,gyro_z_imu);
    title('after adjust (gyro z)')
%     fftSignal(acc_x_mobile(T_start_n:T_end_n),50)
end


% filter
% acc_x_mobile = lowpass(acc_x_mobile, 1, 50);
% acc_x_mobile = movmean(acc_x_mobile, win_phone);

% acc_x_mobile =  medianAverageWindow(acc_x_mobile,20, 10);
 
v_x_car = lowpass(v_x_car, 0.1, 50)*C_motor;
v_x_car = movmean(v_x_car, win_odom);

% acc_z_mobile = lowpass(acc_z_mobile, 1, 50);
% acc_z_mobile = movmean(acc_z_mobile, win_phone);

acc_y_imu = lowpass(acc_y_imu, 1, 50);
acc_y_imu = movmean(acc_y_imu, win_odom);

acc_z_imu = lowpass(acc_z_imu, 1, 50);
acc_z_imu = movmean(acc_z_imu, win_odom);

gyro_z_imu = lowpass(gyro_z_imu, 0.1, 50);
gyro_z_imu = movmean(gyro_z_imu, win_odom);

%% v_x and gyro to calcualte a_y
acc_x_mobile  = gyro_z_imu .* v_x_car;
acc_z_mobile = acc_z_imu;
%%
if is_plot_adjust
    figure
    l3 = plot(t_odom, v_x_car, 'LineWidth', 3, 'DisplayName', 'v_x');
    legend
end
% delta_n = neutralSteer(v_x_car(T_start_n:T_end_n), -acc_x_mobile(T_start_n:T_end_n), track);

ltr_est = LTR(acc_x_mobile, acc_z_mobile, h, track);
t_plt_left = t_mobile;

if is_plot_adjust
    %% use IMU to calculate LTR or mobile
    figure
    subplot(5,1,1)
    hold on
    
    ltr_est = LTR(acc_y_imu, 9.81, h, track);
    t_plt_left = t_odom;
    % l1 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ltr_est(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','LTR car');
    
    ltr_est = LTR(acc_x_mobile, acc_z_mobile, h, track);
    t_plt_left = t_mobile;
    l2 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ltr_est(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','LTR mobile');
    
    l4 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ones(length(t_plt_left(T_start_n:T_end_n)),1)*0.6, '--','LineWidth', 1, 'DisplayName','LTR Threshold: \pm 0.6');
    l5 =  plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), -ones(length(t_plt_left(T_start_n:T_end_n)),1)*0.6, '--','LineWidth', 1);
    legend([ l2])
    
    xlim([0, T_end-T_start])
    
    subplot(5,1,2)
    hold on
    % plot(t_odom(T_start_n:T_end_n)-t_odom(T_start_n), acc_y_imu(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','a_y car');
    plot(t_mobile(T_start_n:T_end_n)-t_mobile(T_start_n), ltr_est(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','a_y mobile');
    % plot(t_odom(T_start_n:T_end_n)-t_odom(T_start_n), acc_z_imu(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','a_z car');
    plot(t_mobile(T_start_n:T_end_n)-t_mobile(T_start_n), 1./acc_z_mobile(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','1/a_z mobile');
    legend
    xlim([0, T_end-T_start])
    
    subplot(5,1,3)
    plot(t_mobile(T_start_n:T_end_n)-t_mobile(T_start_n), acc_z_mobile(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','a_z mobile');
    legend
    xlim([0, T_end-T_start])
    
    subplot(5,1,4)
    plot(t_odom(T_start_n:T_end_n)-t_odom(T_start_n), v_x_car(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','v_x car');
    legend
    xlim([0, T_end-T_start])
    
    subplot(5,1,5)
    plot(t_ackermann(T_start_n:T_end_n)-t_ackermann(T_start_n), delta(T_start_n:T_end_n),'DisplayName','\delta','LineWidth', 3);
    legend
    xlim([0, T_end-T_start])
    ylim([0 inf])
end
%%

f1 = figure;
f1.Units = 'centimeters';
f1.Position = [5 5 6 4]*2;

switch str_acc_brk
    case 'str'
        hold on;
        yyaxis left
        l1 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ltr_est(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','LTR');
        l4 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ones(length(t_plt_left(T_start_n:T_end_n)),1)*0.6, '--','LineWidth', 1, 'DisplayName','LTR Threshold \pm 0.6');
        l5 =  plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), -ones(length(t_plt_left(T_start_n:T_end_n)),1)*0.6, '--','LineWidth', 1);
        l0l = plot([0, T_end],[0 0], 'k-','LineWidth', 0.1);
        ylim(Y_lim_left)
        yticks(Y_ticks_left)
        ylabel('LTR [-]')
        
        yyaxis right
        l2 = plot(t_ackermann(T_start_n:T_end_n)-t_ackermann(T_start_n), delta(T_start_n:T_end_n),'DisplayName','\delta','LineWidth', 3);
        l0r = plot([0, T_end],[0 0],'-','LineWidth', 0.1);
        ylabel('Angle [rad]')
        ylim(Y_lim_right)
        yticks(Y_ticks_right)
        xlim([0, T_end-T_start])
        
        lgd = legend([l1 l4 l2],'location','south'); %lgd.FontSize = 8;
        set(gca, 'FontName','Times New Roman','FontSize', 16, 'FontWeight', 'bold');
    case 'acc'
        hold on;
        yyaxis left
        l1 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ltr_est(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName','LTR');
        l4 = plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), ones(length(t_plt_left(T_start_n:T_end_n)),1)*0.6, '--','LineWidth', 1, 'DisplayName','LTR Threshold \pm 0.6');
        l5 =  plot(t_plt_left(T_start_n:T_end_n)-t_plt_left(T_start_n), -ones(length(t_plt_left(T_start_n:T_end_n)),1)*0.6, '--','LineWidth', 1);
        l0l = plot([0, T_end],[0 0], 'k-','LineWidth', 0.1);
        ylim(Y_lim_left)
        yticks(Y_ticks_left)
        ylabel('LTR [-]')
        
        yyaxis right
        l3 = plot(t_odom(T_start_n:T_end_n)-t_odom(T_start_n), v_x_car(T_start_n:T_end_n), 'LineWidth', 3, 'DisplayName', 'v_x');
        l0r = plot([0, T_end],[0 0],'-','LineWidth', 0.1);
        
        ylim(Y_lim_right)
        yticks(Y_ticks_right)
        xlim([0, T_end-T_start])
        ylabel('Velocity [m/s]')
        lgd = legend([l1 l3 l4],'location','south'); %lgd.FontSize = 8;
        set(gca, 'FontName','Times New Roman','FontSize', 16, 'FontWeight', 'bold');
    case 'brk'
    otherwise
        subplot(2,1,1)
        plot(t_odom(T_start_n:T_end_n)-t_odom(T_start_n), v_x_car(T_start_n:T_end_n),'LineWidth', 3, 'DisplayName', 'v_x');
        ylabel('velocity [m/s]')
        legend('location','northwest');
        
        subplot(2,1,2)
        plot(t_ackermann(T_start_n:T_end_n)-t_ackermann(T_start_n), delta(T_start_n:T_end_n),'DisplayName','\delta');
        hold on
        plot(t_ackermann(T_start_n:T_end_n)-t_ackermann(T_start_n), delta_n*2, 'DisplayName','2\delta_{neutral}');
        plot(t_ackermann(T_start_n:T_end_n)-t_ackermann(T_start_n), delta_n*1.2, 'DisplayName','1.2\delta_{neutral}');
        ylabel('Angle [rad]')
        ylim(Y_lim_left)
        yticks(Y_lim_left)
        legend('location','northwest');
end

output = ltr_est(T_start_n:T_end_n);

xlabel('Time [s]')
box on
fig_name = strsplit(odom_name, '\');

% fig_name = strcat(fig_name{2},'.pdf');
% exportgraphics(gcf,fig_name,'ContentType','vector')

% fig_name_png = strcat(fig_name{2},'.png');
% saveas(gcf, fig_name_png);
end

