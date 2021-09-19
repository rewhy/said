classdef plotJetRacer < handle
    %Plot JetRacer pro data
        
    properties
        car_filename
        mobile_name
        height_CoM
        track
        wheelbase
        acc_x_mobile
        acc_y_mobile
        acc_z_mobile
        gyro_x_mobile
        gyro_y_mobile
        gyro_z_mobile
        t_mobile
        timestamp_car
        str
        throttle
        ay_car
    end
    
    methods
        %%
        function obj = plotJetRacer(mobile_name, car_filename, height_CoM, track, wheelbase)
%             [obj.acc_x_mobile, obj.acc_y_mobile, obj.acc_z_mobile, obj.gyro_x_mobile, obj.gyro_y_mobile, obj.gyro_z_mobile, obj.t_mobile] = import_mobile_file(mobile_name);
            obj.import_car_file(car_filename);
            obj.height_CoM = height_CoM;
            obj.track = track;
            obj.wheelbase = wheelbase;
            obj.mobile_name = mobile_name;
            obj.car_filename = car_filename;
        end
        %%
        function [obj] = import_car_file(obj, car_filename)
            %% 输入处理
            dataLines = [1, Inf];
            
            %% 设置导入选项并导入数据
            opts = delimitedTextImportOptions("NumVariables", 4);
            
            % 指定范围和分隔符
            opts.DataLines = dataLines;
            opts.Delimiter = ",";
            
            % 指定列名称和类型
            opts.VariableNames = ["timestamp_car", "str", "throttle", "ay_car"];
            opts.VariableTypes = ["double", "double", "double", "double"];
            
            % 指定文件级属性
            opts.ExtraColumnsRule = "ignore";
            opts.EmptyLineRule = "read";
            
            % 导入数据
            tbl = readtable(car_filename, opts);
            
            %% 转换为输出类型
            obj.timestamp_car = tbl.timestamp_car;
            obj.str = tbl.str;
            obj.throttle = tbl.throttle;
            obj.ay_car = tbl.ay_car;
        end
        %%
        function [box_data, ltr] = process_box(obj)
%             box data [Maximum 
%                         Median 
%                         Minimum 
%                         First quartile
%                         Third quartile];

            ltr = obj.LTR(obj.ay_car, 1, obj.height_CoM, obj.wheelbase);
            isout = isoutlier(ltr,'quartiles');
            ltr(isout) = [];
            box_data = [max(ltr)
                median(ltr)
                min(ltr)
                prctile(ltr,25)
                prctile(ltr,75)
                ];
        end
        
        %%
        function obj = plotLTR(obj, str_offset, str_scale, str_acc_brk, t_start, t_end, Y_lim_left, Y_lim_right)
            %% NOTE!!!!!!!  imported ay_car is ay/az
            ltr = obj.LTR(obj.ay_car, 1, obj.height_CoM, obj.wheelbase);
            %%
            t = (obj.timestamp_car - obj.timestamp_car(1))/1000;
            fprintf(['Car sensor end time is ', num2str(t(end))]);
            
            % find index of start and end time
            i_temp = find(t == interp1(t,t,t_start, 'nearest'));
            i_t_start = i_temp(1);
            i_temp = find(t == interp1(t,t,t_end, 'nearest'));
            i_t_end = i_temp(1);
            
            f1 = figure;
            hold on
            box on
            f1.Units = 'centimeters';
            f1.Position = [5 5 6 4]*2;
            xlim([t_start t_end])
            xlabel('Time [s]')
            
            yyaxis left
            l1 = plot(t(i_t_start:i_t_end), ltr(i_t_start:i_t_end),'LineWidth', 3, 'DisplayName','LTR');
            ylim(Y_lim_left)
            ylabel('LTR [-]')
            l0l = plot([0, t_end],[0 0], '-','LineWidth', 0.1);
            l5 =  plot([0, t_end],[1 1]*0.6, '--','LineWidth', 1);
            
            switch str_acc_brk
                case 'str'
                    str_corrected = (obj.str - str_offset) * str_scale;
                    yyaxis right
                    l2 = plot(t(i_t_start:i_t_end),str_corrected(i_t_start:i_t_end),'DisplayName','\delta','LineWidth', 3);
                    ylim(Y_lim_right)
                    ylabel('Angle [degree]')
                    l0r = plot([0, t_end],[0 0],'-','LineWidth', 0.1);
            end
            
            legend([l1 l2],'location','south');
            
            
%             fig_name = strsplit(obj.car_filename, '/');
%             fig_name = strsplit(fig_name{3},'.');
%             fig_name_png = strcat('fig/', fig_name{1},'.png');
%             saveas(gcf, fig_name_png);
        end
    end
    
    %%
    methods (Static)
        function ltr = LTR(a_y, a_z, h, w)
            ltr = -2*a_y*h./(a_z*w);
        end
    end
end

