function [t, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z] = import_car_txt(filename, dataLines)
%IMPORTFILE Import data from a text file
%  [T, ACC_X, ACC_Y, ACC_Z, GYRO_X, GYRO_Y, GYRO_Z] =
%  IMPORTFILE(FILENAME) reads data from text file FILENAME for the
%  default selection.  Returns the data as column vectors.
%
%  [T, ACC_X, ACC_Y, ACC_Z, GYRO_X, GYRO_Y, GYRO_Z] = IMPORTFILE(FILE,
%  DATALINES) reads data for the specified row interval(s) of text file
%  FILENAME. Specify DATALINES as a positive scalar integer or a N-by-2
%  array of positive scalar integers for dis-contiguous row intervals.
%
%  Example:
%  [t, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z] = importfile("D:\OneDrive - The Hong Kong Polytechnic University\VD_IDS\Test data\2020 02 19\car_txt\imu01.txt", [1, Inf]);
%
%  See also READTABLE.

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [1, Inf];
end

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 7);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = " ";

% Specify column names and types
opts.VariableNames = ["t", "acc_x", "acc_y", "acc_z", "gyro_x", "gyro_y", "gyro_z"];
opts.VariableTypes = ["double", "double", "double", "double", "double", "double", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
opts.ConsecutiveDelimitersRule = "join";
opts.LeadingDelimitersRule = "ignore";

% Import the data
tbl = readtable(filename, opts);

%% Convert to output type
t = tbl.t;
acc_x = tbl.acc_x;
acc_y = tbl.acc_y;
acc_z = tbl.acc_z;
gyro_x = tbl.gyro_x;
gyro_y = tbl.gyro_y;
gyro_z = tbl.gyro_z;
end