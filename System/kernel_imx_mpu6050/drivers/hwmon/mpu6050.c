/*
 * MPU6050 6-axis gyroscope + accelerometer driver
 *
 * Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/pm.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/poll.h>
#include <linux/hwmon.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include "mpu6050.h"
#include <linux/kthread.h>
#include <linux/delay.h>

#define LOG_FUNC	printk(KERN_INFO "MPU6050:%s\n",__FUNCTION__);
#define LOG_LINE	printk(KERN_INFO "MPU6050:%s:%d\n",__FUNCTION__, __LINE__);

#define POLL_INTERVAL_MIN       1
#define POLL_INTERVAL_MAX       500
#define POLL_INTERVAL           100 /* msecs */

#define DEBUG_NODE

#define IS_ODD_NUMBER(x)	(x & 1UL)

/*VDD 2.375V-3.46V VLOGIC 1.8V +-5%*/
#define MPU6050_VDD_MIN_UV	2500000
#define MPU6050_VDD_MAX_UV	3400000
#define MPU6050_VLOGIC_MIN_UV	1800000
#define MPU6050_VLOGIC_MAX_UV	1800000
#define MPU6050_VI2C_MIN_UV	1750000
#define MPU6050_VI2C_MAX_UV	1950000
/*****************���ٶȺ������ǵĲ�����Χ*******************/
#define MPU6050_ACCEL_MIN_VALUE	-32768
#define MPU6050_ACCEL_MAX_VALUE	32767
#define MPU6050_GYRO_MIN_VALUE	-32768
#define MPU6050_GYRO_MAX_VALUE	32767

#define MPU6050_MAX_EVENT_CNT	170
/* Limit mininum delay to 10ms as we do not need higher rate so far �ѵ�ǰ���ļ�ָ��ҵ��ȴ����� */
#define MPU6050_ACCEL_MIN_POLL_INTERVAL_MS	5                    //10 to 5   ������Ϣ���ϲ�Ӧ��
#define MPU6050_ACCEL_MAX_POLL_INTERVAL_MS	100              //5000 to 100һ��ûʲô�������������޶���
#define MPU6050_ACCEL_DEFAULT_POLL_INTERVAL_MS	200
#define MPU6050_ACCEL_INT_MAX_DELAY			19
/******************************/
#define MPU6050_GYRO_MIN_POLL_INTERVAL_MS	5
#define MPU6050_GYRO_MAX_POLL_INTERVAL_MS	100
#define MPU6050_GYRO_DEFAULT_POLL_INTERVAL_MS	200
#define MPU6050_GYRO_INT_MAX_DELAY		18

#define MPU6050_RAW_ACCEL_DATA_LEN	6
#define MPU6050_RAW_GYRO_DATA_LEN	6
/************����ʱ��*************/
#define MPU6050_RESET_SLEEP_US	5                        //10 to 5

#define MPU6050_DEV_NAME_ACCEL	"MPU6050-accel"
#define MPU6050_DEV_NAME_GYRO	"gyroscope"

#define MPU6050_PINCTRL_DEFAULT	"mpu_default"
#define MPU6050_PINCTRL_SUSPEND	"mpu_sleep"

#define CAL_SKIP_COUNT	5
#define MPU_ACC_CAL_COUNT	15
#define MPU_ACC_CAL_NUM	(MPU_ACC_CAL_COUNT - CAL_SKIP_COUNT)            //15-5=10
#define MPU_ACC_CAL_BUF_SIZE	22
#define RAW_TO_1G	16384
#define MPU_ACC_CAL_DELAY 10	/* ms                               100 to 10        change*/
#define POLL_MS_100HZ 10                               //mybe relation

enum mpu6050_place {
    MPU6050_PLACE_PU = 0,
    MPU6050_PLACE_PR = 1,
    MPU6050_PLACE_LD = 2,
    MPU6050_PLACE_LL = 3,
    MPU6050_PLACE_PU_BACK = 4,
    MPU6050_PLACE_PR_BACK = 5,
    MPU6050_PLACE_LD_BACK = 6,
    MPU6050_PLACE_LL_BACK = 7,
    MPU6050_PLACE_UNKNOWN = 8,
    MPU6050_AXIS_REMAP_TAB_SZ = 8
};

struct mpu6050_place_name {
    char name[32];
    enum mpu6050_place place;
};

struct axis_data {
    s16 x;
    s16 y;
    s16 z;
    s16 rx;
    s16 ry;
    s16 rz;
};

struct report_axis_data {
    s16 x;
    s16 y;
    s16 z;
};


struct mpu6050_sensor {
    struct i2c_client *client;
    struct device *dev;
    struct hrtimer gyro_timer;
    struct hrtimer accel_timer;
    struct input_dev *accel_dev;
    struct input_dev *gyro_dev;
    struct delayed_work work;
    struct mutex data_lock;
    struct mpu6050_platform_data *pdata;
    struct mutex op_lock;
    enum inv_devices chip_type;
    struct workqueue_struct *data_wq;
    struct delayed_work fifo_flush_work;
    struct mpu_reg_map reg;
    struct mpu_chip_config cfg;
    struct axis_data axis;
    
    struct report_axis_data accel_report_data;
    struct report_axis_data gyro_report_data;
    
    u32 gyro_poll_ms;
    u32 accel_poll_ms;
    u32 accel_latency_ms;
    u32 gyro_latency_ms;
    atomic_t accel_en;
    atomic_t gyro_en;
    bool use_poll;
	bool motion_det_en;
    bool batch_accel;
    bool batch_gyro;

    /* calibration */
    char acc_cal_buf[MPU_ACC_CAL_BUF_SIZE];
    int acc_cal_params[3];
    bool acc_use_cal;

    wait_queue_head_t fifo_wq;
    atomic_t fifo_ready;
    int active;
    int delay;
    int position;
    u8 chip_id;
    int mode;
    int awaken;			// is just awake from suspend
    s64 period_rel;
    int fifo_wakeup;
    int fifo_timeout;


    u64 fifo_start_ns;
    atomic_t gyro_wkp_flag;
    atomic_t accel_wkp_flag;
    struct task_struct *gyr_task;
    struct task_struct *accel_task;
    bool gyro_delay_change;
    bool accel_delay_change;
    wait_queue_head_t	gyro_wq;
    wait_queue_head_t	accel_wq;
};
static struct mpu6050_sensor * p_mpu6050_sensor = NULL;

/* Addresses scanned */
static const unsigned short normal_i2c[] = { 0x68, 0x69, I2C_CLIENT_END };

struct sensor_axis_remap {
    /* src means which source will be mapped to target x, y, z axis */
    /* if an target OS axis is remapped from (-)x,
     * src is 0, sign_* is (-)1 */
    /* if an target OS axis is remapped from (-)y,
     * src is 1, sign_* is (-)1 */
    /* if an target OS axis is remapped from (-)z,
     * src is 2, sign_* is (-)1 */
    int src_x:3;
    int src_y:3;
    int src_z:3;

    int sign_x:2;
    int sign_y:2;
    int sign_z:2;
};

static const struct sensor_axis_remap
mpu6050_accel_axis_remap_tab[MPU6050_AXIS_REMAP_TAB_SZ] = {
    /* src_x src_y src_z  sign_x  sign_y  sign_z */
    {  0,    1,    2,     1,      1,      1 }, /* P0 */
    {  1,    0,    2,     1,     -1,      1 }, /* P1 */
    {  0,    1,    2,    -1,     -1,      1 }, /* P2 */
    {  1,    0,    2,    -1,      1,      1 }, /* P3 */

    {  0,    1,    2,    -1,      1,     -1 }, /* P4 */
    {  1,    0,    2,    -1,     -1,     -1 }, /* P5 */
    {  0,    1,    2,     1,     -1,     -1 }, /* P6 */
    {  1,    0,    2,     1,      1,     -1 }, /* P7 */
};

static const struct sensor_axis_remap
mpu6050_gyro_axis_remap_tab[MPU6050_AXIS_REMAP_TAB_SZ] = {
    /* src_x src_y src_z  sign_x  sign_y  sign_z */
    {  0,    1,    2,    -1,      1,     -1 }, /* P0 */
    {  1,    0,    2,    -1,     -1,     -1 }, /* P1*/
    {  0,    1,    2,     1,     -1,     -1 }, /* P2 */
    {  1,    0,    2,     1,      1,     -1 }, /* P3 */

    {  0,    1,    2,     1,      1,      1 }, /* P4 */
    {  1,    0,    2,     1,     -1,      1 }, /* P5 */
    {  0,    1,    2,    -1,     -1,      1 }, /* P6 */
    {  1,    0,    2,    -1,      1,      1 }, /* P7 */
};

static const struct mpu6050_place_name
mpu6050_place_name2num[MPU6050_AXIS_REMAP_TAB_SZ] = {
    {"Portrait Up", MPU6050_PLACE_PU},
    {"Landscape Right", MPU6050_PLACE_PR},
    {"Portrait Down", MPU6050_PLACE_LD},
    {"Landscape Left", MPU6050_PLACE_LL},
    {"Portrait Up Back Side", MPU6050_PLACE_PU_BACK},
    {"Landscape Right Back Side", MPU6050_PLACE_PR_BACK},
    {"Portrait Down Back Side", MPU6050_PLACE_LD_BACK},
    {"Landscape Left Back Side", MPU6050_PLACE_LL_BACK},
};

/* Map gyro measurement range setting to number of bit to shift */
static const u8 mpu_gyro_fs_shift[NUM_FSR] = {
    GYRO_SCALE_SHIFT_FS0, /* MPU_FSR_250DPS */
    GYRO_SCALE_SHIFT_FS1, /* MPU_FSR_500DPS */
    GYRO_SCALE_SHIFT_FS2, /* MPU_FSR_1000DPS */
    GYRO_SCALE_SHIFT_FS3, /* MPU_FSR_2000DPS */
};

/* Map accel measurement range setting to number of bit to shift */
static const u8 mpu_accel_fs_shift[NUM_ACCL_FSR] = {
    ACCEL_SCALE_SHIFT_02G, /* ACCEL_FS_02G */
    ACCEL_SCALE_SHIFT_04G, /* ACCEL_FS_04G */
    ACCEL_SCALE_SHIFT_08G, /* ACCEL_FS_08G */
    ACCEL_SCALE_SHIFT_16G, /* ACCEL_FS_16G */
};

/* Function declarations */
static int gyro_poll_thread(void *data);
static int accel_poll_thread(void *data);
static int mpu6050_set_interrupt(struct mpu6050_sensor *sensor,
        const u8 mask, bool on);
static int mpu6050_set_fifo(struct mpu6050_sensor *sensor,
        bool en_accel, bool en_gyro);
static void mpu6050_flush_fifo(struct mpu6050_sensor *sensor);
static int mpu6050_config_sample_rate(struct mpu6050_sensor *sensor);
static void mpu6050_acc_data_process(struct mpu6050_sensor *sensor);
static int mpu6050_restore_context(struct mpu6050_sensor *sensor);
static int mpu6050_gyro_enable(struct mpu6050_sensor *sensor, bool on);

static inline void mpu6050_set_fifo_start_time(struct mpu6050_sensor *sensor)
{
    struct timespec ts;

    get_monotonic_boottime(&ts);
    sensor->fifo_start_ns = timespec_to_ns(&ts);
}

static int mpu6050_power_ctl(struct mpu6050_sensor *sensor, bool on)
{
    return 0;
}


/**
 * mpu6050_read_reg() - read multiple register data
 * @start_addr: register address read from
 * @buffer: provide register addr and get register
 * @length: length of register
 *
 * Reads the register values in one transaction or returns a negative
 * error code on failure.
 */
static int mpu6050_read_reg(struct i2c_client *client, u8 start_addr,
        u8 *buffer, int length)
{
    /*
     * Annoying we can't make this const because the i2c layer doesn't
     * declare input buffers const.
     */
    struct i2c_msg msg[] = {
        {
            .addr = client->addr,
            .flags = 0,
            .len = 1,
            .buf = &start_addr,
        },
        {
            .addr = client->addr,
            .flags = I2C_M_RD,
            .len = length,
            .buf = buffer,
        },
    };

    return i2c_transfer(client->adapter, msg, 2);
}

/**
 * mpu6050_read_accel_data() - get accelerometer data from device
 * @sensor: sensor device instance
 * @data: axis data to update
 *
 * Return the converted X Y and Z data from the sensor device
 */
static void mpu6050_read_accel_data(struct mpu6050_sensor *sensor,
        struct axis_data *data)
{
    u16 buffer[3];

    mpu6050_read_reg(sensor->client, sensor->reg.raw_accel,
            (u8 *)buffer, MPU6050_RAW_ACCEL_DATA_LEN);
    data->x = be16_to_cpu(buffer[0]);
    data->y = be16_to_cpu(buffer[1]);
    data->z = be16_to_cpu(buffer[2]);
}

/**
 * mpu6050_read_gyro_data() - get gyro data from device
 * @sensor: sensor device instance
 * @data: axis data to update
 *
 * Return the converted RX RY and RZ data from the sensor device
 */
static void mpu6050_read_gyro_data(struct mpu6050_sensor *sensor,
        struct axis_data *data)
{
    u16 buffer[3];

    mpu6050_read_reg(sensor->client, sensor->reg.raw_gyro,
            (u8 *)buffer, MPU6050_RAW_GYRO_DATA_LEN);
    data->rx = be16_to_cpu(buffer[0]);
    data->ry = be16_to_cpu(buffer[1]);
    data->rz = be16_to_cpu(buffer[2]);
}

/**
 * mpu6050_remap_accel_data() - remap accelerometer raw data to axis data
 * @data: data needs remap
 * @place: sensor position
 */
static void mpu6050_remap_accel_data(struct axis_data *data, int place)
{
    const struct sensor_axis_remap *remap;
    s16 tmp[3];
    /* sensor with place 0 needs not to be remapped */
    if ((place <= 0) || (place >= MPU6050_AXIS_REMAP_TAB_SZ))
        return;

    remap = &mpu6050_accel_axis_remap_tab[place];

    tmp[0] = data->x;
    tmp[1] = data->y;
    tmp[2] = data->z;
    data->x = tmp[remap->src_x] * remap->sign_x;
    data->y = tmp[remap->src_y] * remap->sign_y;
    data->z = tmp[remap->src_z] * remap->sign_z;

    return;
}

/**
 * mpu6050_remap_gyro_data() - remap gyroscope raw data to axis data
 * @data: data to remap
 * @place: sensor position
 */
static void mpu6050_remap_gyro_data(struct axis_data *data, int place)
{
    const struct sensor_axis_remap *remap;
    s16 tmp[3];
    /* sensor with place 0 needs not to be remapped */
    if ((place <= 0) || (place >= MPU6050_AXIS_REMAP_TAB_SZ))
        return;

    remap = &mpu6050_gyro_axis_remap_tab[place];
    tmp[0] = data->rx;
    tmp[1] = data->ry;
    tmp[2] = data->rz;
    data->rx = tmp[remap->src_x] * remap->sign_x;
    data->ry = tmp[remap->src_y] * remap->sign_y;
    data->rz = tmp[remap->src_z] * remap->sign_z;

    return;
}

static int mpu6050_gyro_batching_enable(struct mpu6050_sensor *sensor)
{
    int ret = 0;
    u32 latency;

    if (!sensor->batch_accel) {
        latency = sensor->gyro_latency_ms;
    } else {
        cancel_delayed_work_sync(&sensor->fifo_flush_work);
        if (sensor->accel_latency_ms < sensor->gyro_latency_ms)
            latency = sensor->accel_latency_ms;
        else
            latency = sensor->gyro_latency_ms;
    }
    ret = mpu6050_set_fifo(sensor, sensor->cfg.accel_enable, true);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to enable FIFO for gyro, ret=%d\n", ret);
        return ret;
    }

    if (sensor->use_poll) {
        queue_delayed_work(sensor->data_wq,
                &sensor->fifo_flush_work,
                msecs_to_jiffies(latency));
    } else if (!sensor->cfg.int_enabled) {
        mpu6050_set_interrupt(sensor, BIT_FIFO_OVERFLOW, true);
        enable_irq(sensor->client->irq);
        sensor->cfg.int_enabled = true;
    }

    return ret;
}


static int mpu6050_gyro_batching_disable(struct mpu6050_sensor *sensor)
{
    int ret = 0;
    u32 latency;

    ret = mpu6050_set_fifo(sensor, sensor->cfg.accel_enable, false);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to disable FIFO for accel, ret=%d\n", ret);
        return ret;
    }
    if (!sensor->use_poll) {
        if (sensor->cfg.int_enabled && !sensor->cfg.accel_enable) {
            mpu6050_set_interrupt(sensor,
                    BIT_FIFO_OVERFLOW, false);
            disable_irq(sensor->client->irq);
            sensor->cfg.int_enabled = false;
        }
    } else {
        if (!sensor->batch_accel) {
            cancel_delayed_work_sync(&sensor->fifo_flush_work);
        } else if (sensor->gyro_latency_ms <
                sensor->accel_latency_ms) {
            cancel_delayed_work_sync(&sensor->fifo_flush_work);
            latency = sensor->accel_latency_ms;
            queue_delayed_work(sensor->data_wq,
                    &sensor->fifo_flush_work,
                    msecs_to_jiffies(latency));
        }
    }
    sensor->batch_gyro = false;

    return ret;
}

static int mpu6050_gyro_set_enable(struct mpu6050_sensor *sensor, bool enable)
{
    int ret = 0;

    dev_dbg(&sensor->client->dev,
            "mpu6050_gyro_set_enable enable=%d\n", enable);
    mutex_lock(&sensor->op_lock);
    if (enable) {

        ret = mpu6050_gyro_enable(sensor, true);
        if (ret) {
            dev_err(&sensor->client->dev,
                    "Fail to enable gyro engine ret=%d\n", ret);
            ret = -EBUSY;
            goto exit;
        }

        ret = mpu6050_config_sample_rate(sensor);
        if (ret < 0)
            dev_info(&sensor->client->dev,
                    "Unable to update sampling rate! ret=%d\n",
                    ret);

        if (sensor->batch_gyro) {
            ret = mpu6050_gyro_batching_enable(sensor);
            if (ret) {
                dev_err(&sensor->client->dev,
                        "Fail to enable gyro batching =%d\n",
                        ret);
                ret = -EBUSY;
                goto exit;
            }
        } else {
            ktime_t ktime;
            ktime = ktime_set(0,
                    sensor->gyro_poll_ms * NSEC_PER_MSEC);
            hrtimer_start(&sensor->gyro_timer, ktime, HRTIMER_MODE_REL);
        }
        atomic_set(&sensor->gyro_en, 1);
    } else {
        atomic_set(&sensor->gyro_en, 0);
        if (sensor->batch_gyro) {
            ret = mpu6050_gyro_batching_disable(sensor);
            if (ret) {
                dev_err(&sensor->client->dev,
                        "Fail to enable gyro batching =%d\n",
                        ret);
                ret = -EBUSY;
                goto exit;
            }
        } else {
            hrtimer_cancel(&sensor->gyro_timer);
        }
        ret = mpu6050_gyro_enable(sensor, false);
        if (ret) {
            dev_err(&sensor->client->dev,
                    "Fail to disable gyro engine ret=%d\n", ret);
            ret = -EBUSY;
            goto exit;
        }

    }

exit:
    mutex_unlock(&sensor->op_lock);
    return ret;
}


/*
 * Set interrupt enabling bits to enable/disable specific type of interrupt.
 */
static int mpu6050_set_interrupt(struct mpu6050_sensor *sensor,
        const u8 mask, bool on)
{
    int ret;
    u8 data;

    if (sensor->cfg.is_asleep)
        return -EINVAL;

    ret = i2c_smbus_read_byte_data(sensor->client,
            sensor->reg.int_enable);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail read interrupt mode. ret=%d\n", ret);
        return ret;
    }

    if (on) {
        data = (u8)ret;
        data |= mask;
    } else {
        data = (u8)ret;
        data &= ~mask;
    }

    ret = i2c_smbus_write_byte_data(sensor->client,
            sensor->reg.int_enable, data);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to set interrupt. ret=%d\n", ret);
        return ret;
    }
    return 0;
}

/*
 * Enable/disable motion detection interrupt.
 */
static int mpu6050_set_motion_det(struct mpu6050_sensor *sensor, bool on)
{
    int ret;

    if (on) {
        ret = i2c_smbus_write_byte_data(sensor->client,
                sensor->reg.mot_thr, DEFAULT_MOT_THR);
        if (ret < 0)
            goto err_exit;

        ret = i2c_smbus_write_byte_data(sensor->client,
                sensor->reg.mot_dur, DEFAULT_MOT_DET_DUR);
        if (ret < 0)
            goto err_exit;

    }

    ret = mpu6050_set_interrupt(sensor, BIT_MOT_EN, on);
    if (ret < 0)
        goto err_exit;

    sensor->cfg.mot_det_on = on;
    /* Use default motion detection delay 4ms */

    return 0;

err_exit:
    dev_err(&sensor->client->dev,
            "Fail to set motion detection. ret=%d\n", ret);
    return ret;
}


/* Update sensor sample rate divider upon accel and gyro polling rate. */
static int mpu6050_config_sample_rate(struct mpu6050_sensor *sensor)
{
    int ret;
    u32 delay_ms;
    u8 div, saved_pwr;

    if (sensor->cfg.is_asleep)
        return -EINVAL;

    if (sensor->accel_poll_ms <= sensor->gyro_poll_ms)
        delay_ms = sensor->accel_poll_ms;
    else
        delay_ms = sensor->gyro_poll_ms;

    /*Sample_rate = internal_ODR/(1+SMPLRT_DIV) */
    if ((sensor->cfg.lpf != MPU_DLPF_256HZ_NOLPF2) &&
            (sensor->cfg.lpf != MPU_DLPF_RESERVED)) {
        if (delay_ms > DELAY_MS_MAX_DLPF)
            delay_ms = DELAY_MS_MAX_DLPF;
        if (delay_ms < DELAY_MS_MIN_DLPF)
            delay_ms = DELAY_MS_MIN_DLPF;

        div = (u8)(((ODR_DLPF_ENA * delay_ms) / MSEC_PER_SEC) - 1);                              //...........................................
    } else {
        if (delay_ms > DELAY_MS_MAX_NODLPF)
            delay_ms = DELAY_MS_MAX_NODLPF;
        if (delay_ms < DELAY_MS_MIN_NODLPF)
            delay_ms = DELAY_MS_MIN_NODLPF;
        div = (u8)(((ODR_DLPF_DIS * delay_ms) / MSEC_PER_SEC) - 1);
    }

    if (sensor->cfg.rate_div == div)
        return 0;

    ret = i2c_smbus_read_byte_data(sensor->client, sensor->reg.pwr_mgmt_1);
    if (ret < 0)
        goto err_exit;

    saved_pwr = (u8)ret;

    ret = i2c_smbus_write_byte_data(sensor->client, sensor->reg.pwr_mgmt_1,
            (saved_pwr & ~BIT_SLEEP));
    if (ret < 0)
        goto err_exit;

    ret = i2c_smbus_write_byte_data(sensor->client,
            sensor->reg.sample_rate_div, div);
    if (ret < 0)
        goto err_exit;

    ret = i2c_smbus_write_byte_data(sensor->client, sensor->reg.pwr_mgmt_1,
            saved_pwr);
    if (ret < 0)
        goto err_exit;

    sensor->cfg.rate_div = div;

    return 0;
err_exit:
    dev_err(&sensor->client->dev,
            "update sample div failed, div=%d, ret=%d\n",
            div, ret);
    return ret;
}


static void setup_mpu6050_reg(struct mpu_reg_map *reg) {
    LOG_FUNC;

    reg->sample_rate_div = REG_SAMPLE_RATE_DIV;
    reg->lpf = REG_CONFIG;
    reg->fifo_en = REG_FIFO_EN;
    reg->gyro_config = REG_GYRO_CONFIG;
    reg->accel_config = REG_ACCEL_CONFIG;
    reg->mot_thr = REG_ACCEL_MOT_THR;
    reg->mot_dur = REG_ACCEL_MOT_DUR;
    reg->fifo_count_h = REG_FIFO_COUNT_H;
    reg->fifo_r_w = REG_FIFO_R_W;
    reg->raw_gyro = REG_RAW_GYRO;
    reg->raw_accel = REG_RAW_ACCEL;
    reg->temperature = REG_TEMPERATURE;
    reg->int_pin_cfg = REG_INT_PIN_CFG;
    reg->int_enable = REG_INT_ENABLE;
    reg->int_status = REG_INT_STATUS;
    reg->user_ctrl = REG_USER_CTRL;
    reg->pwr_mgmt_1 = REG_PWR_MGMT_1;
    reg->pwr_mgmt_2 = REG_PWR_MGMT_2;

    LOG_FUNC;
}

static int mpu6050_device_init(struct mpu6050_sensor *sensor) {
    struct i2c_client *client = sensor->client;
    struct mpu_reg_map *reg;
    s32 ret;

    LOG_FUNC;
    sensor->chip_type = INV_MPU6050;
    reg = &sensor->reg;
    setup_mpu6050_reg(reg);

    ret = i2c_smbus_read_byte_data(client, REG_WHOAMI);

    if (ret == MPU6050_ID) {
        sensor->chip_type = INV_MPU6050;
        printk(KERN_INFO "mpu6050_device_init INV_MPU6050 \n");
    } else {
        dev_err(&client->dev, "Invalid chip ID %d\n", ret);
        return -ENODEV;
    }

    LOG_FUNC;
    return 0;
}

/**
 *  mpu6050_set_lpa_freq() - set low power wakeup frequency.
 */
static int mpu6050_set_lpa_freq(struct mpu6050_sensor *sensor, int lpa_freq)
{
    int ret;
    u8 data;
    LOG_FUNC;

    /* only for MPU6050 with fixed rate, need expend */
    if (INV_MPU6050 == sensor->chip_type) {
        ret = i2c_smbus_read_byte_data(sensor->client,
                sensor->reg.pwr_mgmt_2);
        if (ret < 0)
            return ret;

        data = (u8)ret;
        data &= ~BIT_LPA_FREQ_MASK;
        data |= MPU6050_LPA_5HZ;
        ret = i2c_smbus_write_byte_data(sensor->client,
                sensor->reg.pwr_mgmt_2, data);
        if (ret < 0)
            return ret;
    }
    sensor->cfg.lpa_freq = lpa_freq;

    LOG_FUNC;
    return 0;
}

static int mpu6050_switch_engine(struct mpu6050_sensor *sensor,
        bool en, u32 mask)
{
    struct mpu_reg_map *reg;
    u8 data, mgmt_1;
    int ret;

    LOG_FUNC;
    reg = &sensor->reg;
    /*
     * switch clock needs to be careful. Only when gyro is on, can
     * clock source be switched to gyro. Otherwise, it must be set to
     * internal clock
     */
    mgmt_1 = MPU_CLK_INTERNAL;
    if (BIT_PWR_GYRO_STBY_MASK == mask) {
        ret = i2c_smbus_read_byte_data(sensor->client,
                reg->pwr_mgmt_1);
        if (ret < 0)
            goto error;
        mgmt_1 = (u8)ret;
        mgmt_1 &= ~BIT_CLK_MASK;
    }

    if ((BIT_PWR_GYRO_STBY_MASK == mask) && (!en)) {
        /*
         * turning off gyro requires switch to internal clock first.
         * Then turn off gyro engine
         */
        mgmt_1 |= MPU_CLK_INTERNAL;
        ret = i2c_smbus_write_byte_data(sensor->client,
                reg->pwr_mgmt_1, mgmt_1);
        if (ret < 0)
            goto error;
    }

    ret = i2c_smbus_read_byte_data(sensor->client,
            reg->pwr_mgmt_2);
    if (ret < 0)
        goto error;
    data = (u8)ret;
    if (en)
        data &= (~mask);
    else
        data |= mask;
    ret = i2c_smbus_write_byte_data(sensor->client,
            reg->pwr_mgmt_2, data);
    if (ret < 0)
        goto error;

    if ((BIT_PWR_GYRO_STBY_MASK == mask) && en) {
        /* wait gyro stable */
        msleep(SENSOR_UP_TIME_MS);
        /* after gyro is on & stable, switch internal clock to PLL */
        mgmt_1 |= MPU_CLK_PLL_X;
        ret = i2c_smbus_write_byte_data(sensor->client,
                reg->pwr_mgmt_1, mgmt_1);
        if (ret < 0)
            goto error;
    }

    LOG_FUNC;
    return 0;

error:
    dev_err(&sensor->client->dev, "Fail to switch MPU engine\n");
    return ret;
}


static int mpu6050_init_engine(struct mpu6050_sensor *sensor)
{
    int ret;

    LOG_FUNC;
    ret = mpu6050_switch_engine(sensor, false, BIT_PWR_GYRO_STBY_MASK);
    if (ret)
        return ret;

    ret = mpu6050_switch_engine(sensor, false, BIT_PWR_ACCEL_STBY_MASK);
    if (ret)
        return ret;

    LOG_FUNC;
    return 0;
}

/**
 * mpu6050_set_power_mode() - set the power mode
 * @sensor: sensor data structure
 * @power_on: value to switch on/off of power, 1: normal power,
 *    0: low power
 *
 * Put device to normal-power mode or low-power mode.
 */
static int mpu6050_set_power_mode(struct mpu6050_sensor *sensor,
        bool power_on)
{
    struct i2c_client *client = sensor->client;
    s32 ret;
    u8 val;

    ret = i2c_smbus_read_byte_data(client, sensor->reg.pwr_mgmt_1);
    if (ret < 0) {
        dev_err(&client->dev,
                "Fail to read power mode, ret=%d\n", ret);
        return ret;
    }

    if (power_on)
        val = (u8)ret & ~BIT_SLEEP;
    else
        val = (u8)ret | BIT_SLEEP;
    ret = i2c_smbus_write_byte_data(client, sensor->reg.pwr_mgmt_1, val);
    if (ret < 0) {
        dev_err(&client->dev,
                "Fail to write power mode, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

static int mpu6050_gyro_enable(struct mpu6050_sensor *sensor, bool on)
{
    int ret;
    u8 data;

    if (sensor->cfg.is_asleep) {
        dev_err(&sensor->client->dev,
                "Fail to set gyro state, device is asleep.\n");
        return -EINVAL;
    }

    ret = i2c_smbus_read_byte_data(sensor->client,
            sensor->reg.pwr_mgmt_1);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to get sensor power state, ret=%d\n", ret);
        return ret;
    }

    data = (u8)ret;
    if (on) {
        ret = mpu6050_switch_engine(sensor, true,
                BIT_PWR_GYRO_STBY_MASK);
        if (ret)
            return ret;
        sensor->cfg.gyro_enable = 1;

        data &= ~BIT_SLEEP;
        ret = i2c_smbus_write_byte_data(sensor->client,
                sensor->reg.pwr_mgmt_1, data);
        if (ret < 0) {
            dev_err(&sensor->client->dev,
                    "Fail to set sensor power state, ret=%d\n",
                    ret);
            return ret;
        }

        sensor->cfg.enable = 1;
    } else {

        ret = mpu6050_switch_engine(sensor, false,
                BIT_PWR_GYRO_STBY_MASK);
        if (ret)
            return ret;
        sensor->cfg.gyro_enable = 0;
        if (!sensor->cfg.accel_enable) {
            data |=  BIT_SLEEP;
            ret = i2c_smbus_write_byte_data(sensor->client,
                    sensor->reg.pwr_mgmt_1, data);
            if (ret < 0) {
                dev_err(&sensor->client->dev,
                        "Fail to set sensor power state, ret=%d\n",
                        ret);
                return ret;
            }
            sensor->cfg.enable = 0;
        }
    }
    return 0;
}


/**
 * mpu6050_restore_context() - update the sensor register context
 */

static int mpu6050_restore_context(struct mpu6050_sensor *sensor)
{
    struct mpu_reg_map *reg;
    struct i2c_client *client;
    int ret;
    u8 data, pwr_ctrl;

    client = sensor->client;
    reg = &sensor->reg;

    /* Save power state and wakeup device from sleep */
    ret = i2c_smbus_read_byte_data(client, reg->pwr_mgmt_1);
    if (ret < 0) {
        dev_err(&client->dev, "read power ctrl failed.\n");
        goto exit;
    }
    pwr_ctrl = (u8)ret;

    ret = i2c_smbus_write_byte_data(client, reg->pwr_mgmt_1,
            BIT_WAKEUP_AFTER_RESET);
    if (ret < 0) {
        dev_err(&client->dev, "wakeup sensor failed.\n");
        goto exit;
    }
    ret = i2c_smbus_write_byte_data(client, reg->gyro_config,
            sensor->cfg.fsr << GYRO_CONFIG_FSR_SHIFT);
    if (ret < 0) {
        dev_err(&client->dev, "update fsr failed.\n");
        goto exit;
    }

    ret = i2c_smbus_write_byte_data(client, reg->lpf, sensor->cfg.lpf);
    if (ret < 0) {
        dev_err(&client->dev, "update lpf failed.\n");
        goto exit;
    }

    ret = i2c_smbus_write_byte_data(client, reg->accel_config,
            (sensor->cfg.accel_fs << ACCL_CONFIG_FSR_SHIFT));
    if (ret < 0) {
        dev_err(&client->dev, "update accel_fs failed.\n");
        goto exit;
    }

    ret = i2c_smbus_write_byte_data(client, reg->sample_rate_div,
            sensor->cfg.rate_div);
    if (ret < 0) {
        dev_err(&client->dev, "set sample_rate_div failed.\n");
        goto exit;
    }

    ret = i2c_smbus_read_byte_data(client, reg->fifo_en);
    if (ret < 0) {
        dev_err(&client->dev, "read fifo_en failed.\n");
        goto exit;
    }

    data = (u8)ret;

    if (sensor->cfg.accel_fifo_enable)
        data |= BIT_ACCEL_FIFO;

    if (sensor->cfg.gyro_fifo_enable)
        data |= BIT_GYRO_FIFO;

    if (sensor->cfg.accel_fifo_enable || sensor->cfg.gyro_fifo_enable) {
        ret = i2c_smbus_write_byte_data(client, reg->fifo_en, data);
        if (ret < 0) {
            dev_err(&client->dev, "write fifo_en failed.\n");
            goto exit;
        }
    }

    if (sensor->cfg.cfg_fifo_en) {
        /* Assume DMP and external I2C is not in use*/
        ret = i2c_smbus_write_byte_data(client, reg->user_ctrl,
                BIT_FIFO_EN);
        if (ret < 0) {
            dev_err(&client->dev, "enable FIFO R/W failed.\n");
            goto exit;
        }
    }

    /* Accel and Gyro should set to standby by default */
    ret = i2c_smbus_write_byte_data(client, reg->pwr_mgmt_2,
            BITS_PWR_ALL_AXIS_STBY);
    if (ret < 0) {
        dev_err(&client->dev, "set pwr_mgmt_2 failed.\n");
        goto exit;
    }

    ret = mpu6050_set_lpa_freq(sensor, sensor->cfg.lpa_freq);
    if (ret < 0) {
        dev_err(&client->dev, "set lpa_freq failed.\n");
        goto exit;
    }

    ret = i2c_smbus_write_byte_data(client, reg->int_pin_cfg,
            sensor->cfg.int_pin_cfg);
    if (ret < 0) {
        dev_err(&client->dev, "set int_pin_cfg failed.\n");
        goto exit;
    }

    ret = i2c_smbus_write_byte_data(client, reg->pwr_mgmt_1,
            pwr_ctrl);
    if (ret < 0) {
        dev_err(&client->dev, "write saved power state failed.\n");
        goto exit;
    }

    dev_dbg(&client->dev, "restore context finished\n");

exit:
    return ret;
}



/**
 * mpu6050_reset_chip() - reset chip to default state
 */
static void mpu6050_reset_chip(struct mpu6050_sensor *sensor)
{
    struct i2c_client *client;
    int ret, i;

    LOG_FUNC;
    client = sensor->client;

    ret = i2c_smbus_write_byte_data(client, sensor->reg.pwr_mgmt_1,
            BIT_RESET_ALL);
    if (ret < 0) {
        dev_err(&client->dev, "Reset chip fail!\n");
        goto exit;
    }
    for (i = 0; i < MPU6050_RESET_RETRY_CNT; i++) {
        ret = i2c_smbus_read_byte_data(sensor->client,
                sensor->reg.pwr_mgmt_1);
        if (ret < 0) {
            dev_err(&sensor->client->dev,
                    "Fail to get reset state ret=%d\n", ret);
            goto exit;
        }

        if ((ret & BIT_H_RESET) == 0) {
            dev_dbg(&sensor->client->dev,
                    "Chip reset success! i=%d\n", i);
            break;
        }

        usleep_range(5000,8000);
    }

    LOG_FUNC;
exit:
    return;
}

/**
 *  mpu6050_init_config() - Initialize hardware, disable FIFO.
 *  @indio_dev:	Device driver instance.
 *  Initial configuration:
 *  FSR: +/- 2000DPS
 *  DLPF: 42Hz
 *  FIFO rate: 50Hz
 *  AFS: 2G
 */
static int mpu6050_init_config(struct mpu6050_sensor *sensor)
{
    struct mpu_reg_map *reg;
    struct i2c_client *client;
    s32 ret;
    u8 data;

    LOG_FUNC;
    if (sensor->cfg.is_asleep)
        return -EINVAL;

    reg = &sensor->reg;
    client = sensor->client;

    mpu6050_reset_chip(sensor);
    LOG_LINE;
    memset(&sensor->cfg, 0, sizeof(struct mpu_chip_config));
    LOG_LINE;
    /* Wake up from sleep */
    ret = i2c_smbus_write_byte_data(client, reg->pwr_mgmt_1,
            BIT_WAKEUP_AFTER_RESET);
    if (ret < 0)
        return ret;
    LOG_LINE;
    /* Gyro full scale range configure */
    ret = i2c_smbus_write_byte_data(client, reg->gyro_config,
            MPU_FSR_2000DPS << GYRO_CONFIG_FSR_SHIFT);
    if (ret < 0)
        return ret;
    sensor->cfg.fsr = MPU_FSR_2000DPS;
    LOG_LINE;
    ret = i2c_smbus_write_byte_data(client, reg->lpf, MPU_DLPF_42HZ);
    if (ret < 0)
        return ret;
    sensor->cfg.lpf = MPU_DLPF_42HZ;
    LOG_LINE;
    data = (u8)(ODR_DLPF_ENA / INIT_FIFO_RATE - 1);
    ret = i2c_smbus_write_byte_data(client, reg->sample_rate_div, data);
    if (ret < 0)
        return ret;
    sensor->cfg.rate_div = data;
    LOG_LINE;
    ret = i2c_smbus_write_byte_data(client, reg->accel_config,
            (ACCEL_FS_02G << ACCL_CONFIG_FSR_SHIFT));
    if (ret < 0)
        return ret;
    sensor->cfg.accel_fs = ACCEL_FS_02G;
    LOG_LINE;
    if ((sensor->pdata->int_flags & IRQF_TRIGGER_FALLING) ||
            (sensor->pdata->int_flags & IRQF_TRIGGER_LOW))
        data = BIT_INT_CFG_DEFAULT | BIT_INT_ACTIVE_LOW;
    else
        data = BIT_INT_CFG_DEFAULT;
    ret = i2c_smbus_write_byte_data(client, reg->int_pin_cfg, data);
    if (ret < 0)
        return ret;
    sensor->cfg.int_pin_cfg = data;
    LOG_LINE;
    /* Put sensor into sleep mode */
    ret = i2c_smbus_read_byte_data(client,
            sensor->reg.pwr_mgmt_1);
    if (ret < 0)
        return ret;

    data = (u8)ret;
    data |=  BIT_SLEEP;
    ret = i2c_smbus_write_byte_data(client,
            sensor->reg.pwr_mgmt_1, data);
    if (ret < 0)
        return ret;
    LOG_LINE;
    sensor->cfg.gyro_enable = 1;
    sensor->cfg.gyro_fifo_enable = 0;
    sensor->cfg.accel_enable = 1;
    sensor->cfg.accel_fifo_enable = 0;

    LOG_FUNC;
    return 0;
}


static void mpu6050_sche_next_flush(struct mpu6050_sensor *sensor)
{
    u32 latency;

    if ((sensor->batch_accel) && (sensor->batch_gyro)) {
        if (sensor->gyro_latency_ms < sensor->accel_latency_ms)
            latency = sensor->gyro_latency_ms;
        else
            latency = sensor->accel_latency_ms;
    } else if (sensor->batch_accel)
        latency = sensor->accel_latency_ms;
    else if (sensor->batch_gyro)
        latency = sensor->gyro_latency_ms;
    else
        latency = 0;

    if (latency != 0)
        queue_delayed_work(sensor->data_wq,
                &sensor->fifo_flush_work,
                msecs_to_jiffies(latency));
    else
        dev_err(&sensor->client->dev,
                "unknown error, accel: en=%d latency=%d gyro: en=%d latency=%d\n",
                sensor->batch_accel,
                sensor->accel_latency_ms,
                sensor->batch_gyro,
                sensor->gyro_latency_ms);

    return;
}

/**
 * mpu6050_fifo_flush_fn() - flush shared sensor FIFO
 * @work: the work struct
 */
static void mpu6050_fifo_flush_fn(struct work_struct *work)
{
    struct mpu6050_sensor *sensor = container_of(
            (struct delayed_work *)work,
            struct mpu6050_sensor, fifo_flush_work);

    mpu6050_flush_fifo(sensor);
    mpu6050_sche_next_flush(sensor);

    return;
}


static enum hrtimer_restart gyro_timer_handle(struct hrtimer *hrtimer)
{
    struct mpu6050_sensor *sensor;
    ktime_t ktime;
    sensor = container_of(hrtimer, struct mpu6050_sensor, gyro_timer);
    ktime = ktime_set(0,
            sensor->gyro_poll_ms * NSEC_PER_MSEC);
    hrtimer_forward_now(&sensor->gyro_timer, ktime);
    atomic_set(&sensor->gyro_wkp_flag, 1);    
    wake_up_interruptible(&sensor->gyro_wq);
    return HRTIMER_RESTART;
}

static enum hrtimer_restart accel_timer_handle(struct hrtimer *hrtimer)
{
    struct mpu6050_sensor *sensor;
    ktime_t ktime;
    sensor = container_of(hrtimer, struct mpu6050_sensor, accel_timer);
    ktime = ktime_set(0,
            sensor->accel_poll_ms * NSEC_PER_MSEC);
    hrtimer_forward_now(&sensor->accel_timer, ktime);
    atomic_set(&sensor->accel_wkp_flag, 1);
    wake_up_interruptible(&sensor->accel_wq);
    return HRTIMER_RESTART;
}


static int gyro_poll_thread(void *data)
{
    struct mpu6050_sensor *sensor = data;
    u32 shift;
    ktime_t timestamp;

    while(1)
    {
//    	LOG_LINE;
        wait_event_interruptible(sensor->gyro_wq,
        		((atomic_read(&sensor->gyro_wkp_flag) != 0) || kthread_should_stop()));
        
        atomic_set(&sensor->gyro_wkp_flag,0);
//    	LOG_LINE;
        if (kthread_should_stop())
            break;
        
#if 0
//    	LOG_LINE;
        mutex_lock(&sensor->op_lock);
        if(sensor->gyro_delay_change) {
            sensor->gyro_delay_change = false;
        }
        mutex_unlock(&sensor->op_lock);
//    	LOG_LINE;
        timestamp = ktime_get_boottime();
        mpu6050_read_gyro_data(sensor, &sensor->axis);
        mpu6050_remap_gyro_data(&sensor->axis, sensor->pdata->place);
        shift = mpu_gyro_fs_shift[sensor->cfg.fsr];
        
		sensor->gyro_report_data.x = sensor->axis.rx >> shift;
        sensor->gyro_report_data.y = sensor->axis.ry >> shift;
        sensor->gyro_report_data.z = sensor->axis.rz >> shift;
//    	LOG_LINE;
        input_report_abs(sensor->gyro_dev, ABS_RX,
        		sensor->gyro_report_data.x);
        input_report_abs(sensor->gyro_dev, ABS_RY,
        		sensor->gyro_report_data.y);
        input_report_abs(sensor->gyro_dev, ABS_RZ,
        		sensor->gyro_report_data.z);
        
        
//        printk(KERN_INFO "gyro_poll_thread x=0x%x,y=0x%x,z=0x%x \n",
//        		sensor->gyro_report_data.x,
//				sensor->gyro_report_data.y,
//				sensor->gyro_report_data.z);
        
        input_sync(sensor->gyro_dev);
        
#endif
    }
    return 0;
}

static int accel_poll_thread(void *data)
{
    struct mpu6050_sensor *sensor = data;
    u32 shift;
    ktime_t timestamp;

    while(1)
    {
//    	LOG_LINE;
        wait_event_interruptible(sensor->accel_wq,
                ((atomic_read(&sensor->accel_wkp_flag) != 0) || kthread_should_stop()));
//    	LOG_LINE;
        atomic_set(&sensor->accel_wkp_flag,0);

        if (kthread_should_stop())
            break;
//    	LOG_LINE;
        mutex_lock(&sensor->op_lock);
        if(sensor->accel_delay_change) {
            //			if(sensor->accel_poll_ms <= POLL_MS_100HZ)
            //				set_wake_up_idle(true);
            //			else
            //				set_wake_up_idle(false);
            sensor->accel_delay_change = false;
        }
        mutex_unlock(&sensor->op_lock);
//    	LOG_LINE;
        timestamp = ktime_get_boottime();
        mpu6050_acc_data_process(sensor);
        shift = mpu_accel_fs_shift[sensor->cfg.accel_fs];

		sensor->accel_report_data.x = sensor->axis.x >> shift;
        sensor->accel_report_data.y = sensor->axis.y >> shift;
        sensor->accel_report_data.z = sensor->axis.z >> shift;
//    	LOG_LINE;
    	
        mpu6050_read_gyro_data(sensor, &sensor->axis);
        mpu6050_remap_gyro_data(&sensor->axis, sensor->pdata->place);
        shift = mpu_gyro_fs_shift[sensor->cfg.fsr];
        
        
		sensor->gyro_report_data.x = sensor->axis.rx >> shift;
        sensor->gyro_report_data.y = sensor->axis.ry >> shift;
        sensor->gyro_report_data.z = sensor->axis.rz >> shift;
        
        
        
        input_report_abs(sensor->accel_dev, ABS_X,
        		sensor->accel_report_data.x);
        input_report_abs(sensor->accel_dev, ABS_Y,
        		sensor->accel_report_data.y);
        input_report_abs(sensor->accel_dev, ABS_Z,
        		sensor->accel_report_data.z);
        
        input_report_abs(sensor->accel_dev, ABS_RX,
        		sensor->gyro_report_data.x);
        input_report_abs(sensor->accel_dev, ABS_RY,
        		sensor->gyro_report_data.y);
        input_report_abs(sensor->accel_dev, ABS_RZ,
        		sensor->gyro_report_data.z);
        
        printk(KERN_INFO "accel_poll_thread x=0x%x,y=0x%x,z=0x%x rx=0x%x,ry=0x%x,rz=0x%x \n",
        		sensor->accel_report_data.x,
				sensor->accel_report_data.y,
				sensor->accel_report_data.z,
        		sensor->gyro_report_data.x,
				sensor->gyro_report_data.y,
				sensor->gyro_report_data.z
        );

        input_sync(sensor->accel_dev);
        
        
    }

    return 0;
}

/*
 * Calculate sample interval according to sample rate.
 * Return sample interval in millisecond.
 */
static inline u64 mpu6050_get_sample_interval(struct mpu6050_sensor *sensor)
{
    u64 interval_ns;

    if ((sensor->cfg.lpf == MPU_DLPF_256HZ_NOLPF2) ||
            (sensor->cfg.lpf == MPU_DLPF_RESERVED)) {
        interval_ns = (sensor->cfg.rate_div + 1) * NSEC_PER_MSEC;               //NSEC_PER_MSEC==ms
        interval_ns /= 8;
    } else {
        interval_ns = (sensor->cfg.rate_div + 1) * NSEC_PER_MSEC;
    }

    return interval_ns;
}

/**
 * mpu6050_flush_fifo() - flush fifo and send sensor event
 * @sensor: sensor device instance
 * Return 0 on success and returns a negative error code on failure.
 *
 * This function assumes only accel and gyro data will be stored into FIFO
 * and does not check FIFO enabling bits, if other sensor data is stored into
 * FIFO, it will cause confusion.
 */
static void mpu6050_flush_fifo(struct mpu6050_sensor *sensor)
{
    struct i2c_client *client = sensor->client;
    u64 interval_ns, ts_ns, sec;
    int ret, i, ns;
    u16 *buf, cnt;
    u8 shift;

    ret = mpu6050_read_reg(sensor->client, sensor->reg.fifo_count_h,
            (u8 *)&cnt, MPU6050_FIFO_CNT_SIZE);
    if (ret < 0) {
        dev_err(&client->dev, "read FIFO count failed, ret=%d\n", ret);
        return;
    }

    cnt = be16_to_cpu(cnt);
    dev_dbg(&client->dev, "Flush: FIFO count=%d\n", cnt);
    if (cnt == 0)
        return;
    if (cnt > MPU6050_FIFO_SIZE_BYTE || IS_ODD_NUMBER(cnt)) {
        dev_err(&client->dev, "Invalid FIFO count number %d\n", cnt);
        return;
    }

    interval_ns = mpu6050_get_sample_interval(sensor);
    dev_dbg(&client->dev, "interval_ns=%llu, fifo_start_ns=%llu\n",
            interval_ns, sensor->fifo_start_ns);
    ts_ns = sensor->fifo_start_ns + interval_ns;
    mpu6050_set_fifo_start_time(sensor);

    buf = kmalloc(cnt, GFP_KERNEL);
    if (!buf) {
        dev_err(&client->dev,
                "Allocate FIFO buffer error!\n");
        return;
    }

    ret = mpu6050_read_reg(sensor->client, sensor->reg.fifo_r_w,
            (u8 *)buf, cnt);
    if (ret < 0) {
        dev_err(&client->dev, "Read FIFO data error!\n");
        goto exit;
    }

    for (i = 0; i < (cnt >> 1); ts_ns += interval_ns) {
        if (sensor->cfg.accel_fifo_enable) {
            sensor->axis.x = be16_to_cpu(buf[i++]);
            sensor->axis.y = be16_to_cpu(buf[i++]);
            sensor->axis.z = be16_to_cpu(buf[i++]);
            sec = ts_ns;
            ns = do_div(sec, NSEC_PER_SEC);

            mpu6050_remap_accel_data(&sensor->axis,
                    sensor->pdata->place);

            shift = mpu_accel_fs_shift[sensor->cfg.accel_fs];
            input_report_abs(sensor->accel_dev, ABS_X,
                    (sensor->axis.x << shift));
            input_report_abs(sensor->accel_dev, ABS_Y,
                    (sensor->axis.y << shift));
            input_report_abs(sensor->accel_dev, ABS_Z,
                    (sensor->axis.z << shift));
            input_sync(sensor->accel_dev);
        }

        if (sensor->cfg.gyro_fifo_enable) {
            sensor->axis.rx = be16_to_cpu(buf[i++]);
            sensor->axis.ry = be16_to_cpu(buf[i++]);
            sensor->axis.rz = be16_to_cpu(buf[i++]);
            sec = ts_ns;
            ns = do_div(sec, NSEC_PER_SEC);

            mpu6050_remap_gyro_data(&sensor->axis,
                    sensor->pdata->place);

            shift = mpu_gyro_fs_shift[sensor->cfg.fsr];
            input_report_abs(sensor->gyro_dev, ABS_RX,
                    (sensor->axis.rx >> shift));
            input_report_abs(sensor->gyro_dev, ABS_RY,
                    (sensor->axis.ry >> shift));
            input_report_abs(sensor->gyro_dev, ABS_RZ,
                    (sensor->axis.rz >> shift));
            input_sync(sensor->gyro_dev);
        }
    }

exit:
    kfree(buf);
    return;
}


/**
 * mpu6050_set_fifo() - Configure and enable sensor FIFO
 * @sensor: sensor device instance
 * @en_accel: buffer accel event to fifo
 * @en_gyro: buffer gyro event to fifo
 * Return 0 on success and returns a negative error code on failure.
 *
 * This function will remove all existing FIFO setting and flush FIFO data,
 * new FIFO setting will be applied after that.
 */
static int mpu6050_set_fifo(struct mpu6050_sensor *sensor,
        bool en_accel, bool en_gyro)
{
    struct i2c_client *client = sensor->client;
    struct mpu_reg_map *reg = &sensor->reg;
    int ret;
    u8 en, user_ctl;

    en = FIFO_DISABLE_ALL;
    ret = i2c_smbus_write_byte_data(client,
            reg->fifo_en, en);
    if (ret < 0)
        goto err_exit;

    mpu6050_flush_fifo(sensor);

    /* Enable sensor output to FIFO */
    if (en_accel)
        en |= BIT_ACCEL_FIFO;

    if (en_gyro)
        en |= BIT_GYRO_FIFO;

    ret = i2c_smbus_write_byte_data(client,
            reg->fifo_en, en);
    if (ret < 0)
        goto err_exit;

    /* Enable/Disable FIFO RW*/
    ret = i2c_smbus_read_byte_data(client,
            reg->user_ctrl);
    if (ret < 0)
        goto err_exit;

    user_ctl = (u8)ret;
    if (en_accel | en_gyro) {
        user_ctl |= BIT_FIFO_EN;
        sensor->cfg.cfg_fifo_en = true;
    } else {
        user_ctl &= ~BIT_FIFO_EN;
        sensor->cfg.cfg_fifo_en = false;
    }

    ret = i2c_smbus_write_byte_data(client,
            reg->user_ctrl, user_ctl);
    if (ret < 0)
        goto err_exit;

    mpu6050_set_fifo_start_time(sensor);
    sensor->cfg.accel_fifo_enable = en_accel;
    sensor->cfg.gyro_fifo_enable = en_gyro;

    return 0;

err_exit:
    dev_err(&client->dev, "Set fifo failed, ret=%d\n", ret);
    return ret;
}


static int mpu6050_gyro_set_poll_delay(struct mpu6050_sensor *sensor,
        unsigned long delay)
{
    int ret;

    dev_dbg(&sensor->client->dev,
            "mpu6050_gyro_set_poll_delay delay=%ld\n", delay);
    if (delay < MPU6050_GYRO_MIN_POLL_INTERVAL_MS)
        delay = MPU6050_GYRO_MIN_POLL_INTERVAL_MS;
    if (delay > MPU6050_GYRO_MAX_POLL_INTERVAL_MS)
        delay = MPU6050_GYRO_MAX_POLL_INTERVAL_MS;               //..........................

    mutex_lock(&sensor->op_lock);
    if (sensor->gyro_poll_ms == delay)
        goto exit;

    sensor->gyro_delay_change = true;
    sensor->gyro_poll_ms = delay;

    if (!atomic_read(&sensor->gyro_en))
        goto exit;

    if (sensor->use_poll) {
        ktime_t ktime;
        hrtimer_cancel(&sensor->gyro_timer);
        ktime = ktime_set(0,
                sensor->gyro_poll_ms * NSEC_PER_MSEC);
        hrtimer_start(&sensor->gyro_timer, ktime, HRTIMER_MODE_REL);

    } else {
        ret = mpu6050_config_sample_rate(sensor);
        if (ret < 0)
            dev_err(&sensor->client->dev,
                    "Unable to set polling delay for gyro!\n");
    }

exit:
    mutex_unlock(&sensor->op_lock);
    return 0;
}

/**
 * mpu6050_gyro_attr_get_polling_delay() - get the sampling rate
 */
static ssize_t mpu6050_gyro_attr_get_polling_delay(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    int val;
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);

    val = sensor ? sensor->gyro_poll_ms : 0;
    return snprintf(buf, 8, "%d\n", val);
}


/**
 * mpu6050_gyro_attr_set_polling_delay() - set the sampling rate
 */
static ssize_t mpu6050_gyro_attr_set_polling_delay(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t size)
{
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);
    unsigned long interval_ms;
    int ret;

    if (kstrtoul(buf, 10, &interval_ms))
        return -EINVAL;

    ret = mpu6050_gyro_set_poll_delay(sensor, interval_ms);

    return ret ? -EBUSY : size;
}

static ssize_t mpu6050_gyro_attr_get_enable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);

    return snprintf(buf, 4, "%d\n", sensor->cfg.gyro_enable);
}

/**
 * mpu6050_gyro_attr_set_enable() -
 *    Set/get enable function is just needed by sensor HAL.
 */
static ssize_t mpu6050_gyro_attr_set_enable(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);
    unsigned long enable;
    int ret;

    if (kstrtoul(buf, 10, &enable))
        return -EINVAL;

    if (enable)
        ret = mpu6050_gyro_set_enable(sensor, true);
    else
        ret = mpu6050_gyro_set_enable(sensor, false);

    return ret ? -EBUSY : count;
}

static struct device_attribute gyro_attr[] = {
    __ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
            mpu6050_gyro_attr_get_polling_delay,
            mpu6050_gyro_attr_set_polling_delay),
    __ATTR(enable, S_IRUGO | S_IWUSR,
            mpu6050_gyro_attr_get_enable,
            mpu6050_gyro_attr_set_enable),
};

static int create_gyro_sysfs_interfaces(struct device *dev)
{
    int i;
    int err;
    for (i = 0; i < ARRAY_SIZE(gyro_attr); i++) {
        err = device_create_file(dev, gyro_attr + i);
        if (err)
            goto error;
    }
    return 0;

error:
    for (; i >= 0; i--)
        device_remove_file(dev, gyro_attr + i);
    dev_err(dev, "Unable to create interface\n");
    return err;
}

static int remove_gyro_sysfs_interfaces(struct device *dev)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(gyro_attr); i++)
        device_remove_file(dev, gyro_attr + i);
    return 0;
}



static int mpu6050_accel_enable(struct mpu6050_sensor *sensor, bool on)
{
    int ret;
    u8 data;

    if (sensor->cfg.is_asleep)
        return -EINVAL;

    ret = i2c_smbus_read_byte_data(sensor->client,
            sensor->reg.pwr_mgmt_1);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to get sensor power state, ret=%d\n", ret);
        return ret;
    }

    data = (u8)ret;
    if (on) {
        ret = mpu6050_switch_engine(sensor, true,
                BIT_PWR_ACCEL_STBY_MASK);
        if (ret)
            return ret;
        sensor->cfg.accel_enable = 1;

        data &= ~BIT_SLEEP;
        ret = i2c_smbus_write_byte_data(sensor->client,
                sensor->reg.pwr_mgmt_1, data);
        if (ret < 0) {
            dev_err(&sensor->client->dev,
                    "Fail to set sensor power state, ret=%d\n",
                    ret);
            return ret;
        }

        sensor->cfg.enable = 1;
    } else {
        ret = mpu6050_switch_engine(sensor, false,
                BIT_PWR_ACCEL_STBY_MASK);
        if (ret)
            return ret;
        sensor->cfg.accel_enable = 0;

        if (!sensor->cfg.gyro_enable) {
            data |=  BIT_SLEEP;
            ret = i2c_smbus_write_byte_data(sensor->client,
                    sensor->reg.pwr_mgmt_1, data);
            if (ret < 0) {
                dev_err(&sensor->client->dev,
                        "Fail to set sensor power state for accel, ret=%d\n",
                        ret);
                return ret;
            }
            sensor->cfg.enable = 0;
        }
    }
    
    mpu6050_gyro_enable(sensor, on);
    return 0;
}



static int mpu6050_accel_set_poll_delay(struct mpu6050_sensor *sensor,
        unsigned long delay)
{
    int ret;

    dev_dbg(&sensor->client->dev,
            "mpu6050_accel_set_poll_delay delay_ms=%ld\n", delay);
    if (delay < MPU6050_ACCEL_MIN_POLL_INTERVAL_MS)
        delay = MPU6050_ACCEL_MIN_POLL_INTERVAL_MS;
    if (delay > MPU6050_ACCEL_MAX_POLL_INTERVAL_MS)
        delay = MPU6050_ACCEL_MAX_POLL_INTERVAL_MS;

    mutex_lock(&sensor->op_lock);
    if (sensor->accel_poll_ms == delay)
        goto exit;

    sensor->accel_delay_change = true;
    sensor->accel_poll_ms = delay;

    if (!atomic_read(&sensor->accel_en))
        goto exit;


    if (sensor->use_poll) {
        ktime_t ktime;
        hrtimer_cancel(&sensor->accel_timer);
        ktime = ktime_set(0,
                sensor->accel_poll_ms * NSEC_PER_MSEC);
        hrtimer_start(&sensor->accel_timer, ktime, HRTIMER_MODE_REL);
    } else {
        ret = mpu6050_config_sample_rate(sensor);
        if (ret < 0)
            dev_err(&sensor->client->dev,
                    "Unable to set polling delay for accel!\n");
    }

exit:
    mutex_unlock(&sensor->op_lock);
    return 0;
}

static int mpu6050_accel_batching_enable(struct mpu6050_sensor *sensor)
{
    int ret = 0;
    u32 latency;

    if (!sensor->batch_gyro) {
        latency = sensor->accel_latency_ms;
    } else {
        cancel_delayed_work_sync(&sensor->fifo_flush_work);
        if (sensor->accel_latency_ms < sensor->gyro_latency_ms)
            latency = sensor->accel_latency_ms;
        else
            latency = sensor->gyro_latency_ms;
    }

    ret = mpu6050_set_fifo(sensor, true, sensor->cfg.gyro_enable);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to enable FIFO for accel, ret=%d\n", ret);
        return ret;
    }

    if (sensor->use_poll) {
        queue_delayed_work(sensor->data_wq,
                &sensor->fifo_flush_work,
                msecs_to_jiffies(latency));
    } else if (!sensor->cfg.int_enabled) {
        mpu6050_set_interrupt(sensor, BIT_FIFO_OVERFLOW, true);
        enable_irq(sensor->client->irq);
        sensor->cfg.int_enabled = true;
    }

    return ret;
}

static int mpu6050_accel_batching_disable(struct mpu6050_sensor *sensor)
{
    int ret = 0;
    u32 latency;

    ret = mpu6050_set_fifo(sensor, false, sensor->cfg.gyro_enable);
    if (ret < 0) {
        dev_err(&sensor->client->dev,
                "Fail to disable FIFO for accel, ret=%d\n", ret);
        return ret;
    }
    if (!sensor->use_poll) {
        if (sensor->cfg.int_enabled && !sensor->cfg.gyro_enable) {
            mpu6050_set_interrupt(sensor,
                    BIT_FIFO_OVERFLOW, false);
            disable_irq(sensor->client->irq);
            sensor->cfg.int_enabled = false;
        }
    } else {
        if (!sensor->batch_gyro) {
            cancel_delayed_work_sync(&sensor->fifo_flush_work);
        } else if (sensor->accel_latency_ms <
                sensor->gyro_latency_ms) {
            cancel_delayed_work_sync(&sensor->fifo_flush_work);
            latency = sensor->gyro_latency_ms;
            queue_delayed_work(sensor->data_wq,
                    &sensor->fifo_flush_work,
                    msecs_to_jiffies(latency));
        }
    }
    sensor->batch_accel = false;

    return ret;
}


static int mpu6050_accel_set_enable(struct mpu6050_sensor *sensor, bool enable)
{
    int ret = 0;

    dev_dbg(&sensor->client->dev,
            "mpu6050_accel_set_enable enable=%d\n", enable);
    if (enable) {

        ret = mpu6050_accel_enable(sensor, true);
        if (ret) {
            dev_err(&sensor->client->dev,
                    "Fail to enable accel engine ret=%d\n", ret);
            ret = -EBUSY;
            return ret;
        }

        ret = mpu6050_config_sample_rate(sensor);
        if (ret < 0)
            dev_info(&sensor->client->dev,
                    "Unable to update sampling rate! ret=%d\n",
                    ret);

        if (sensor->batch_accel) {
            ret = mpu6050_accel_batching_enable(sensor);
            if (ret) {
                dev_err(&sensor->client->dev,
                        "Fail to enable accel batching =%d\n",
                        ret);
                ret = -EBUSY;
                return ret;
            }
        } else {
            ktime_t ktime;
            ktime = ktime_set(0,
                    sensor->accel_poll_ms * NSEC_PER_MSEC);
            hrtimer_start(&sensor->accel_timer, ktime, HRTIMER_MODE_REL);
        }
        atomic_set(&sensor->accel_en, 1);
    } else {
        atomic_set(&sensor->accel_en, 0);
        if (sensor->batch_accel) {
            ret = mpu6050_accel_batching_disable(sensor);
            if (ret) {
                dev_err(&sensor->client->dev,
                        "Fail to disable accel batching =%d\n",
                        ret);
                ret = -EBUSY;
                return ret;
            }
        } else {
            hrtimer_cancel(&sensor->accel_timer);
        }

        ret = mpu6050_accel_enable(sensor, false);
        if (ret) {
            dev_err(&sensor->client->dev,
                    "Fail to disable accel engine ret=%d\n", ret);
            ret = -EBUSY;
            return ret;
        }

    }

    return ret;
}



static void mpu6050_acc_data_process(struct mpu6050_sensor *sensor)
{
    mpu6050_read_accel_data(sensor, &sensor->axis);
    mpu6050_remap_accel_data(&sensor->axis, sensor->pdata->place);
    if (sensor->acc_use_cal) {
        sensor->axis.x -= sensor->acc_cal_params[0];
        sensor->axis.y -= sensor->acc_cal_params[1];
        sensor->axis.z -= sensor->acc_cal_params[2];
    }
}


/**
 * mpu6050_accel_attr_get_polling_delay() - get the sampling rate
 */
static ssize_t mpu6050_accel_attr_get_polling_delay(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    int val;
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);

    val = sensor ? sensor->accel_poll_ms : 0;
    return snprintf(buf, 8, "%d\n", val);
}

/**
 * mpu6050_accel_attr_set_polling_delay() - set the sampling rate
 */
static ssize_t mpu6050_accel_attr_set_polling_delay(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t size)
{
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);
    unsigned long interval_ms;
    int ret;

    if (kstrtoul(buf, 10, &interval_ms))
        return -EINVAL;

    ret = mpu6050_accel_set_poll_delay(sensor, interval_ms);

    return ret ? -EBUSY : size;
}

static ssize_t mpu6050_accel_attr_get_enable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);

    return snprintf(buf, 4, "%d\n", sensor->cfg.accel_enable);
}

/**
 * mpu6050_accel_attr_set_enable() -
 *    Set/get enable function is just needed by sensor HAL.
 */

static ssize_t mpu6050_accel_attr_set_enable(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct mpu6050_sensor *sensor = dev_get_drvdata(dev);
    unsigned long enable;
    int ret;

    if (kstrtoul(buf, 10, &enable))
        return -EINVAL;

    if (enable)
        ret = mpu6050_accel_set_enable(sensor, true);
    else
        ret = mpu6050_accel_set_enable(sensor, false);

    return ret ? -EBUSY : count;
}



static struct device_attribute accel_attr[] = {
    __ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
            mpu6050_accel_attr_get_polling_delay,
            mpu6050_accel_attr_set_polling_delay),
    __ATTR(enable, S_IRUGO | S_IWUSR,
            mpu6050_accel_attr_get_enable,
            mpu6050_accel_attr_set_enable),		    
};

static int create_accel_sysfs_interfaces(struct device *dev)
{
    int i;
    int err;
    for (i = 0; i < ARRAY_SIZE(accel_attr); i++) {
        err = device_create_file(dev, accel_attr + i);
        if (err)
            goto error;
    }
    return 0;

error:
    for (; i >= 0; i--)
        device_remove_file(dev, accel_attr + i);
    dev_err(dev, "Unable to create interface\n");
    return err;
}

static int remove_accel_sysfs_interfaces(struct device *dev)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(accel_attr); i++)
        device_remove_file(dev, accel_attr + i);
    return 0;
}







static int mpu6050_device_stop(struct i2c_client *client) {
    LOG_FUNC;

    return 0;
}






static int mpu6050_detect(struct i2c_client *client,
        struct i2c_board_info *info) {
    struct i2c_adapter *adapter = client->adapter;
    int chip_id;

    LOG_FUNC;

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_READ_WORD_DATA))
        return -ENODEV;
    chip_id = i2c_smbus_read_byte_data(client, REG_WHOAMI);

    printk(KERN_INFO "check MPU6050 i2c address 0x%x \n",
            client->addr);
    strlcpy(info->type, "mpu6050", I2C_NAME_SIZE);
    return 0;
}
static int mpu6050_accel_open(struct inode *inode, struct file *file) {
    int err;
    err = nonseekable_open(inode, file);

//    LOG_FUNC;

    if (err)
        return err;
    file->private_data = p_mpu6050_sensor;

    return 0;
}
static ssize_t mpu6050_accel_read(struct file *file, char __user *buf, size_t size, loff_t *ppos) {
    struct mpu6050_sensor *sensor = file->private_data;
    int ret = 0;
    int shift = 0;

//    LOG_FUNC;
   

//    if (!(file->f_flags & O_NONBLOCK)) {
//        ret = wait_event_interruptible(sensor->accel_wq, (atomic_read(&sensor->accel_wkp_flag) != 0));
//        if (ret)
//            return ret;
//    }

//    LOG_LINE;
    if(size < sizeof(struct axis_data)){
        printk(KERN_ERR "the buffer length less than need\n");
        return -ENOMEM;
    }
    
    mpu6050_acc_data_process(sensor);
    mpu6050_read_gyro_data(sensor, &sensor->axis);
    mpu6050_remap_gyro_data(&sensor->axis, sensor->pdata->place);

//    printk(KERN_INFO "mpu6050_accel_read x=0x%x,y=0x%x,z=0x%x rx=0x%x,ry=0x%x,rz=0x%x \n",
//    		sensor->axis.x,
//			sensor->axis.y,
//			sensor->axis.z,
//    		sensor->axis.rx,
//			sensor->axis.ry,
//			sensor->axis.rz
//			);

    shift = mpu_accel_fs_shift[sensor->cfg.accel_fs];
    sensor->axis.x = sensor->axis.x >> shift;
    sensor->axis.y = sensor->axis.y >> shift;
    sensor->axis.z = sensor->axis.z >> shift;

    shift = mpu_gyro_fs_shift[sensor->cfg.fsr];
    sensor->axis.rx = sensor->axis.rx >> shift;
    sensor->axis.ry = sensor->axis.ry >> shift;
    sensor->axis.rz = sensor->axis.rz >> shift;

//    printk(KERN_INFO "mpu6050_accel_read after shift x=0x%x,y=0x%x,z=0x%x rx=0x%x,ry=0x%x,rz=0x%x \n",
//    		sensor->axis.x,
//			sensor->axis.y,
//			sensor->axis.z,
//    		sensor->axis.rx,
//			sensor->axis.ry,
//			sensor->axis.rz
//			);

//    LOG_LINE;
    if(!copy_to_user(buf,&sensor->axis,sizeof(struct axis_data))){
//        atomic_set(&sensor->accel_wkp_flag,0);
        return sizeof(struct axis_data);
    }
    
//    LOG_LINE;
#if 0
    if(size < sizeof(struct axis_data)){
        printk(KERN_ERR "the buffer length less than need\n");
        return -ENOMEM;
    }

    if(!copy_to_user(buf,&sensor->accel_report_data,sizeof(struct report_axis_data))){
        atomic_set(&sensor->accel_wkp_flag,0);
        return size;
    }
    
#endif

    return -ENOMEM;
}
static unsigned int mpu6050_accel_poll(struct file * file,
        struct poll_table_struct * wait) {

    struct mpu6050_sensor *sensor = file->private_data;
//    LOG_FUNC;
    return 1;
//    poll_wait(file, &sensor->accel_wq, wait);
//    if (atomic_read(&sensor->accel_wkp_flag))
//        return POLLIN | POLLRDNORM;
//
//    return 0;
}

static const struct file_operations mpu6050_accel_fops = {
    .owner = THIS_MODULE,
    .open = mpu6050_accel_open,
    .read = mpu6050_accel_read,
    .poll = mpu6050_accel_poll,
};

static struct miscdevice mpu6050_accel_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mpu6050_accel",
    .fops = &mpu6050_accel_fops,
};



static int mpu6050_gyro_open(struct inode *inode, struct file *file) {
    int err;
    err = nonseekable_open(inode, file);

    LOG_FUNC;

    if (err)
        return err;
    file->private_data = p_mpu6050_sensor;

    return 0;
}
static ssize_t mpu6050_gyro_read(struct file *file, char __user *buf, size_t size, loff_t *ppos) {
    struct mpu6050_sensor *sensor = file->private_data;
    int ret = 0;

    LOG_FUNC;

    if (!(file->f_flags & O_NONBLOCK)) {
        ret = wait_event_interruptible(sensor->gyro_wq, (atomic_read(&sensor->gyro_wkp_flag) != 0));
        if (ret)
            return ret;
    }

    if(size < sizeof(struct axis_data)){
        printk(KERN_ERR "the buffer length less than need\n");
        return -ENOMEM;
    }

    if(!copy_to_user(buf,&sensor->gyro_report_data,sizeof(struct report_axis_data))){
        atomic_set(&sensor->gyro_wkp_flag,0);
        return size;
    }

    return -ENOMEM;
}
static unsigned int mpu6050_gyro_poll(struct file * file,
        struct poll_table_struct * wait) {

    struct mpu6050_sensor *sensor = file->private_data;
    LOG_FUNC;
    poll_wait(file, &sensor->gyro_wq, wait);
    if (atomic_read(&sensor->gyro_wkp_flag))
        return POLLIN | POLLRDNORM;

    return 0;
}

static const struct file_operations mpu6050_gyro_fops = {
    .owner = THIS_MODULE,
    .open = mpu6050_gyro_open,
    .read = mpu6050_gyro_read,
    .poll = mpu6050_gyro_poll,
};

static struct miscdevice mpu6050_gyro_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mpu6050_gyro",
    .fops = &mpu6050_gyro_fops,
};





















static int __devinit mpu6050_probe(struct i2c_client *client,
        const struct i2c_device_id *id)
{
    int result, chip_id;
    struct mpu6050_sensor *sensor;
    struct mpu6050_platform_data *pdata;
    struct i2c_adapter *adapter;

    adapter = to_i2c_adapter(client->dev.parent);
    result = i2c_check_functionality(adapter,
            I2C_FUNC_SMBUS_BYTE |
            I2C_FUNC_SMBUS_BYTE_DATA);
    if (!result)
        goto err_out;

    chip_id = i2c_smbus_read_byte_data(client, REG_WHOAMI);

    printk(KERN_INFO "mpu6050 read REG_WHOAMI:%02x\n",chip_id);

    sensor = kzalloc(sizeof(struct mpu6050_sensor), GFP_KERNEL);
    if (!sensor) {
        result = -ENOMEM;
        dev_err(&client->dev, "alloc data memory error!\n");
        goto err_out;
    }
    /* Initialize the MPU6050 chip */
    memset(sensor,0,sizeof(struct mpu6050_sensor));
    sensor->client = client;
    sensor->chip_id = chip_id;
    sensor->fifo_wakeup = 0;
    sensor->fifo_timeout = 0;
    sensor->position = *(int *)client->dev.platform_data;
    p_mpu6050_sensor = sensor;
    mutex_init(&sensor->data_lock);
    mutex_init(&sensor->op_lock);
    sensor->dev = &client->dev;
    i2c_set_clientdata(client, sensor);


    pdata = kzalloc(sizeof(struct mpu6050_platform_data), GFP_KERNEL);

    if (!pdata) {
        result = -ENOMEM;
        dev_err(&client->dev, "alloc data memory error!\n");
        goto err_out;
    }

    sensor->pdata = pdata;
    sensor->pdata->int_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_LOW;
    sensor->pdata->use_int = false;

    result = mpu6050_device_init(sensor);
    if (result) {
        dev_err(&client->dev, "Cannot get invalid chip type\n");
        goto err_alloc_input_device;
    }

    result = mpu6050_init_engine(sensor);
    if (result) {
        dev_err(&client->dev, "Failed to init chip engine\n");
        goto err_alloc_input_device;
    }

    result = mpu6050_set_lpa_freq(sensor, MPU6050_LPA_5HZ);
    if (result) {
        dev_err(&client->dev, "Failed to set lpa frequency\n");
        goto err_alloc_input_device;
    }

    sensor->cfg.is_asleep = false;                  //sensor->cfg.is_asleep=1
    atomic_set(&sensor->accel_en, 0);
    atomic_set(&sensor->gyro_en, 0);

    result = mpu6050_init_config(sensor);     //you may change the rate in this place

    LOG_LINE;

    if (result) {
        dev_err(&client->dev, "Failed to set default config\n");
        goto err_alloc_input_device;
    }

    sensor->accel_dev = input_allocate_device();
    if (!sensor->accel_dev) {
        result = -ENOMEM;
        dev_err(&client->dev, "alloc accelerometer input device failed!\n");
        goto err_alloc_input_device;
    }

    sensor->gyro_dev = input_allocate_device();
    if (!sensor->gyro_dev) {
        result = -ENOMEM;
        dev_err(&client->dev, "alloc gyroscope input device failed!\n");
        goto err_alloc_input_device;
    }

    sensor->accel_dev->name = MPU6050_DEV_NAME_ACCEL;
    sensor->gyro_dev->name = MPU6050_DEV_NAME_GYRO;
    sensor->accel_dev->uniq = "MPU6050 acc";
    sensor->gyro_dev->uniq = "MPU6050 gyro";

    sensor->accel_dev->id.bustype = BUS_I2C;
    sensor->gyro_dev->id.bustype = BUS_I2C;

//    sensor->accel_dev->evbit[0] = BIT_MASK(EV_ABS);
//    sensor->gyro_dev->evbit[0] = BIT_MASK(EV_ABS);

	input_set_capability(sensor->accel_dev, EV_ABS, ABS_MISC);
	input_set_capability(sensor->gyro_dev, EV_ABS, ABS_MISC);
	
    input_set_abs_params(sensor->accel_dev, ABS_X,
            MPU6050_ACCEL_MIN_VALUE, MPU6050_ACCEL_MAX_VALUE,
            0, 0);
    input_set_abs_params(sensor->accel_dev, ABS_Y,
            MPU6050_ACCEL_MIN_VALUE, MPU6050_ACCEL_MAX_VALUE,
            0, 0);
    input_set_abs_params(sensor->accel_dev, ABS_Z,
            MPU6050_ACCEL_MIN_VALUE, MPU6050_ACCEL_MAX_VALUE,
            0, 0);
    
//    input_set_abs_params(sensor->accel_dev, ABS_RX,
//            MPU6050_GYRO_MIN_VALUE, MPU6050_GYRO_MAX_VALUE,
//            0, 0);
//    input_set_abs_params(sensor->accel_dev, ABS_RY,
//            MPU6050_GYRO_MIN_VALUE, MPU6050_GYRO_MAX_VALUE,
//            0, 0);
//    input_set_abs_params(sensor->accel_dev, ABS_RZ,
//            MPU6050_GYRO_MIN_VALUE, MPU6050_GYRO_MAX_VALUE,
//            0, 0);
    
    
    
//    input_set_abs_params(sensor->gyro_dev, ABS_RX,
//            MPU6050_GYRO_MIN_VALUE, MPU6050_GYRO_MAX_VALUE,
//            0, 0);
//    input_set_abs_params(sensor->gyro_dev, ABS_RY,
//            MPU6050_GYRO_MIN_VALUE, MPU6050_GYRO_MAX_VALUE,
//            0, 0);
//    input_set_abs_params(sensor->gyro_dev, ABS_RZ,
//            MPU6050_GYRO_MIN_VALUE, MPU6050_GYRO_MAX_VALUE,
//            0, 0);

//    dev_set_drvdata(&sensor->accel_dev->dev,sensor);
    dev_set_drvdata(&sensor->gyro_dev->dev,sensor);
	input_set_drvdata(sensor->accel_dev, sensor);
//	input_set_drvdata(sensor->gyro_dev, sensor);
    

    sensor->use_poll = 1;
    sensor->delay = 1;

    sensor->data_wq = create_freezable_workqueue("mpu6050_data_work");
    if (!sensor->data_wq) {
        dev_err(&client->dev, "Cannot create workqueue!\n");
        goto err_alloc_input_device;
    }

    INIT_DELAYED_WORK(&sensor->fifo_flush_work, mpu6050_fifo_flush_fn);

//    hrtimer_init(&sensor->gyro_timer, CLOCK_BOOTTIME, HRTIMER_MODE_REL);
//    sensor->gyro_timer.function = gyro_timer_handle;                                                  //relate with rate
    hrtimer_init(&sensor->accel_timer, CLOCK_BOOTTIME, HRTIMER_MODE_REL);
    sensor->accel_timer.function = accel_timer_handle;

    init_waitqueue_head(&sensor->gyro_wq);
    init_waitqueue_head(&sensor->accel_wq);
//    atomic_set(&sensor->gyro_wkp_flag,0);
    atomic_set(&sensor->accel_wkp_flag,1);

//    sensor->gyr_task = kthread_run(gyro_poll_thread, sensor, "sns_gyro");                //relate with rate
//    sensor->accel_task = kthread_run(accel_poll_thread, sensor, "sns_accel");

    result = input_register_device(sensor->accel_dev);
    if (result) {
        dev_err(&client->dev, "register accelerometer input device failed!\n");
        goto err_register_input_device;
    }

//    result = input_register_device(sensor->gyro_dev);
//    if (result) {
//        dev_err(&client->dev, "register gyroscope input device failed!\n");
//        goto err_register_input_device;
//    }

    result = create_accel_sysfs_interfaces(&sensor->accel_dev->dev);
    if (result < 0) {
        dev_err(&client->dev, "failed to create sysfs for accel\n");
        goto err_create_sysfs;
    }
//    result = create_gyro_sysfs_interfaces(&sensor->gyro_dev->dev);
//    if (result < 0) {
//        dev_err(&client->dev, "failed to create sysfs for gyro\n");
//        goto err_create_sysfs;
//    }

    result = misc_register(&mpu6050_accel_dev);
    if (result) {
        dev_err(&client->dev,"register fifo device error for accel\n");
        goto err_reigster_dev;
    }
    
//    result = misc_register(&mpu6050_gyro_dev);
//    if (result) {
//        dev_err(&client->dev,"register fifo device error for accel\n");
//        goto err_reigster_dev;
//    }
    
    mpu6050_accel_set_poll_delay(sensor, 50);	
    mpu6050_gyro_set_poll_delay(sensor, 50);

    printk(KERN_INFO"mpu6050 device driver probe successfully\n");
    return 0;
err_reigster_dev:
    free_irq(client->irq,sensor);
err_create_sysfs:
    input_unregister_device(sensor->accel_dev);
err_register_input_device:
    input_free_device(sensor->accel_dev);
err_alloc_input_device:
    kfree(sensor);
err_out:
    return result;
}
static int __devexit mpu6050_remove(struct i2c_client *client)
{
    struct mpu6050_sensor *sensor = i2c_get_clientdata(client);
    mpu6050_device_stop(client);

    LOG_FUNC;

    if (sensor) {
//    	remove_gyro_sysfs_interfaces(&sensor->gyro_dev->dev);
    	remove_accel_sysfs_interfaces(&sensor->accel_dev->dev);
    	
        input_unregister_device(sensor->accel_dev);
//        input_unregister_device(sensor->gyro_dev);
        
    	
        input_free_device(sensor->accel_dev);
//        input_free_device(sensor->gyro_dev);

        kfree(sensor);
    }
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int mpu6050_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mpu6050_sensor *sensor = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&sensor->op_lock);
	if ((sensor->batch_accel) || (sensor->batch_gyro)) {
		mpu6050_set_interrupt(sensor,
				BIT_FIFO_OVERFLOW, false);
		cancel_delayed_work_sync(&sensor->fifo_flush_work);
		goto exit;
	}
	if (sensor->motion_det_en) {

		/* keep accel on and config motion detection wakeup */
		ret = mpu6050_set_interrupt(sensor,
				BIT_DATA_RDY_EN, false);
		if (ret == 0)
			ret = mpu6050_set_motion_det(sensor, true);
		if (ret == 0) {
			irq_set_irq_wake(client->irq, 1);

			dev_dbg(&client->dev,
				"Enable motion detection success\n");
			goto exit;
		}
		/*  if motion detection config does not success,
		  *  not exit suspend and sensor will be power off.
		  */
	}

	if (!sensor->use_poll) {
		disable_irq(client->irq);
	} else {
		if (sensor->cfg.gyro_enable)
			hrtimer_cancel(&sensor->gyro_timer);

		if (sensor->cfg.accel_enable)
			hrtimer_cancel(&sensor->accel_timer);
	}

	mpu6050_set_power_mode(sensor, false);
	ret = mpu6050_power_ctl(sensor, false);
	if (ret < 0) {
		dev_err(&client->dev, "Power off mpu6050 failed\n");
		goto exit;
	}

exit:
	mutex_unlock(&sensor->op_lock);
	dev_dbg(&client->dev, "Suspend completed, ret=%d\n", ret);
	
    return 0;
}

static int mpu6050_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mpu6050_sensor *sensor = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&sensor->op_lock);
	if ((sensor->batch_accel) || (sensor->batch_gyro)) {
		mpu6050_set_interrupt(sensor,
				BIT_FIFO_OVERFLOW, true);
		mpu6050_sche_next_flush(sensor);
	}

	if (sensor->cfg.mot_det_on) {
		/* keep accel on and config motion detection wakeup */
		irq_set_irq_wake(client->irq, 0);
		mpu6050_set_motion_det(sensor, false);
		mpu6050_set_interrupt(sensor,
				BIT_DATA_RDY_EN, true);
		dev_dbg(&client->dev, "Disable motion detection success\n");
		goto exit;
	}

	/* Keep sensor power on to prevent bad power state */
	ret = mpu6050_power_ctl(sensor, true);
	if (ret < 0) {
		dev_err(&client->dev, "Power on mpu6050 failed\n");
		goto exit;
	}
	/* Reset sensor to recovery from unexpected state */
	mpu6050_reset_chip(sensor);

	ret = mpu6050_restore_context(sensor);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to restore context\n");
		goto exit;
	}

	/* Enter sleep mode if both accel and gyro are not enabled */
	ret = mpu6050_set_power_mode(sensor, sensor->cfg.enable);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to set power mode enable=%d\n",
					sensor->cfg.enable);
		goto exit;
	}

	if (sensor->cfg.gyro_enable) {
		ret = mpu6050_gyro_enable(sensor, true);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to enable gyro\n");
			goto exit;
		}

		if (sensor->use_poll) {
			ktime_t ktime;
			ktime = ktime_set(0,
					sensor->gyro_poll_ms * NSEC_PER_MSEC);
			hrtimer_start(&sensor->gyro_timer, ktime, HRTIMER_MODE_REL);

		}
	}

	if (sensor->cfg.accel_enable) {
		ret = mpu6050_accel_enable(sensor, true);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to enable accel\n");
			goto exit;
		}

		if (sensor->use_poll) {
			ktime_t ktime;
			ktime = ktime_set(0,
					sensor->accel_poll_ms * NSEC_PER_MSEC);
			hrtimer_start(&sensor->accel_timer, ktime, HRTIMER_MODE_REL);
		}
	}

	if (!sensor->use_poll)
		enable_irq(client->irq);

exit:
	mutex_unlock(&sensor->op_lock);
	dev_dbg(&client->dev, "Resume complete, ret = %d\n", ret);
	return ret;
}
#endif

static const struct i2c_device_id mpu6050_id[] = { { "mpu6050", 0 }, { } };
MODULE_DEVICE_TABLE(i2c, mpu6050_id);

static SIMPLE_DEV_PM_OPS(mpu6050_pm_ops, mpu6050_suspend, mpu6050_resume);
static struct i2c_driver mpu6050_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name = "mpu6050",
        .owner = THIS_MODULE,
        .pm = &mpu6050_pm_ops,
    },
    .probe = mpu6050_probe,
    .remove = __devexit_p(mpu6050_remove),
    .id_table = mpu6050_id,
    .detect = mpu6050_detect,
    .address_list = normal_i2c,
};

static int __init mpu6050_init(void)
{
    /* register driver */
    int res;

    res = i2c_add_driver(&mpu6050_driver);
    if (res < 0) {
        printk(KERN_INFO "add mpu6050 i2c driver failed\n");
        return -ENODEV;
    }

    printk(KERN_INFO "add mpu6050 i2c driver success!\n");
    return res;
}

static void __exit mpu6050_exit(void)
{
    i2c_del_driver(&mpu6050_driver);
}

MODULE_DESCRIPTION("MPU6050 Tri-axis gyroscope driver");
MODULE_LICENSE("GPL v2");

module_init (mpu6050_init);
module_exit (mpu6050_exit);
