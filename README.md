# SAID (State-Aware Injection Defense)
Defense Framework against Vehicular Message Injection Attacks

The system directory contains the source code of SAID implementation, the experiment directory contains used scripts for the evaluations, and the data directory contains the data collected from the real vehicles.


## System
The implementation of SAID includes three major modules, the Android app, the native library, the configurations, and the customized Android system, which are implemented in the directories app, native-lib, config, and kernel_imx_mpu6050, respectively.
**App** is the implementation of the Android application and we developed using Android studio.
**Native-lib** is implemented with C/C++ code and works as a native-library. It receives and transmits OBD/UDS/CAN messages leveraging the SocketCAN. Meanwhile, it parses and builds OBD/UDS messages by loading the DBC configuration files, which stores the information of parsing the in-vehicle traffic, and we build the DBC files by revers-engienring diagnostic protocols and based on the standards.
**Config** contains the rquired configuration files, such as the DBC files and detection rules.
**kernel_imx_mpu6050** is the customized Android system running on the hardware of SAID, i.e., the iMX.6 board along with the additional IMU sensor MPU6050, as well as the specific services of reporting the received in-vehicle messages automatically.

## Experiment
During evaluations, we conducted three major types of expeirments, including attacks on real vehielcs and testbed, simulatied attacks, and robotic cars, and you can find the used scripts in Attack, Simulation and Robotic directories. 
The real-road data collected from the in-vehicle network and onboard sensors are also in this directory, including three drivers and two vehicles.
