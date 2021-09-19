function [acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, t] = import_mobile_file(filename, dataLines)
%IMPORTFILE Import data from a text file
%  [ACC_X, ACC_Y, ACC_Z, GYRO_X, GYRO_Y, GYRO_Z, T] =
%  IMPORTFILE(FILENAME) reads data from text file FILENAME for the
%  default selection.  Returns the data as column vectors.
%
%  [ACC_X, ACC_Y, ACC_Z, GYRO_X, GYRO_Y, GYRO_Z, T] = IMPORTFILE(FILE,
%  DATALINES) reads data for the specified row interval(s) of text file
%  FILENAME. Specify DATALINES as a positive scalar integer or a N-by-2
%  array of positive scalar integers for dis-contiguous row intervals.
%
%  Example:
%  [acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, t] = importfile("D:\OneDrive - The Hong Kong Polytechnic University\VD_IDS\Test data\2021 02 18\Straight_thr_brk_sensor_0218_034513_026.txt", [2, Inf]);
%

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [2, Inf];
end

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 10);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = [",", ":", ";"];

% Specify column names and types
opts.VariableNames = ["Var1", "acc_x", "acc_y", "acc_z", "Var5", "gyro_x", "gyro_y", "gyro_z", "Var9", "t"];
opts.SelectedVariableNames = ["acc_x", "acc_y", "acc_z", "gyro_x", "gyro_y", "gyro_z", "t"];
opts.VariableTypes = ["string", "double", "double", "double", "string", "double", "double", "double", "string", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Specify variable properties
opts = setvaropts(opts, ["Var1", "Var5", "Var9"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Var1", "Var5", "Var9"], "EmptyFieldRule", "auto");

% Import the data
tbl = readtable(filename, opts);

%% Convert to output type
acc_x = tbl.acc_x;
acc_y = tbl.acc_y;
acc_z = tbl.acc_z;
gyro_x = tbl.gyro_x;
gyro_y = tbl.gyro_y;
gyro_z = tbl.gyro_z;
t = tbl.t;
end