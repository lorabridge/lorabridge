/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


//
// driver functions to access peripherals on the LoRaMote (Blipper) board
//

#include "hw.h"

//////////////////////////////////////////////////////////////////////
// Altimeter, Thermometer and Pressure sensor MPL3115A2
//////////////////////////////////////////////////////////////////////

#define MPL3115A2_ADDR                  (0x60<<1)
#define MPL3115A2_CTRL_REG1              0x26
#define MPL3115A2_CTRL_REG1_SBYB_ACTIVE  0x81 // (ALT|ACTIVE)
#define MPL3115A2_OUT_P_MSB              0x01
#define MPL3115A2_OUT_T_MSB              0x04

// init temperature reading on Freescale's MPL3115A2 sensor
void blipper_temp_init() {
    u1_t buf[2] = { MPL3115A2_CTRL_REG1, MPL3115A2_CTRL_REG1_SBYB_ACTIVE };
    i2c_xfer(MPL3115A2_ADDR, buf, 2, 0); // write 2, read 0
}

// read temperature from Freescale's MPL3115A2 sensor 
//
// The DR_STATUS register, OUT_P_MSB, OUT_P_CSB, OUT_P_LSB, OUT_T_MSB,
// and OUT_T_LSB are stored in the auto-incrementing address range of
// 0x00 to 0x05. This allows the host controller to read the status
// register followed by the 20-bit Pressure/Altitude and 12-bit
// Temperature in a 6 byte I2C transaction.
u2_t blipper_temp_read() {
    u1_t buf[2] = { MPL3115A2_OUT_T_MSB }; // read reg OUT_T_MSB, OUT_T_LSB
    i2c_xfer(MPL3115A2_ADDR, buf, 1, 2); // write 1, read 2
    return buf[0]<<8 | buf[1];
}

// read altitude from Freescale's MPL3115A2 sensor
//
// The Altitude data is arranged as 20-bit 2’s complement value in meters.
// The data is stored as meters with the 16 bits of OUT_P_MSB and OUT_P_CSB
// and with fractions of a meter stored in bits 7-4 of OUT_P_LSB.
// Be aware that the fractional bits are not signed, therefore,
// they are not represented in 2’s complement.
s2_t blipper_alt_read() {
    u1_t buf[2] = { MPL3115A2_OUT_P_MSB }; // read reg OUT_P_MSB, OUT_P_CSB
    i2c_xfer(MPL3115A2_ADDR, buf, 1, 2); // write 1, read 2
    return buf[0]<<8 | buf[1];
}

// format temp value as 5-digit string "+99.9"
//
// The Temperature data is arranged as 12-bit 2's complement
// value in degrees C. The 8 bits of OUT_T_MSB representing
// degrees and with fractions of a degree are stored in 4 bits
// in position 7-4 of OUT_T_LSB. Be aware that the fractional
// bits are not signed, therefore, they are not represented in
// 2's complement. When RAW is selected then the RAW value is
// stored in all 16 bits of OUT_T_MSB and OUT_T_LSB
void blipper_temp_str(u2_t temp, u1_t* str) {
    u1_t t1 = temp >> 8;
    u1_t t2 = temp & 0xFF;
    if(t1 & 0x80) {
        str[0] = '-';
        t1 = 256 - t1;
    } else {
        str[0] = '+';
    }
    str[2] = (t1 % 10) + '0';
    t1 /= 10;
    str[1] = (t1!=0) ? (t1 + '0') : ' ';
    str[3] = '.';
    str[4] = (t2>>4)*10/16 + '0';
}


//////////////////////////////////////////////////////////////////////
// 3-Axis Accelerometer sensor MMA8451Q
//////////////////////////////////////////////////////////////////////

#define MMA8451Q_ADDR             (0x1C<<1)
#define MMA8451Q_CTRL_REG1         0x2A
#define MMA8451Q_CTRL_REG1_ACTIVE  0x01
#define MMA8451Q_STATUS            0x00

// init accelerometer reading from Freescale's MMA8451Q sensor
void blipper_accel_init (void) {
    u1_t buf[2] = { MMA8451Q_CTRL_REG1, MMA8451Q_CTRL_REG1_ACTIVE };
    i2c_xfer(MMA8451Q_ADDR, buf, 2, 0); // write 2, read 0
}

// read accelaration from Freescale's MMA8451Q sensor (0xSSXXYYZZ)
//
// The measured acceleration data is stored in the OUT_X_MSB,
// OUT_X_LSB, OUT_Y_MSB, OUT_Y_LSB, OUT_Z_MSB, and OUT_Z_LSB registers
// as 2's complement 14-bit numbers. The most significant 8-bits of
// each axis are stored in OUT_X (Y , Z)_MSB, so applications needing
// only 8-bit results can use these 3 registers and ignore OUT_X,Y ,
// Z_LSB. To do this, the F_READ bit in CTRL_REG1 must be set. When
// the F_READ bit is cleared, the fast read mode is disabled.
u4_t blipper_accel_read (void) {
    u1_t buf[7] = { MMA8451Q_STATUS }; // read reg STATUS, X, X, Y, Y, Z, Z
    i2c_xfer(MMA8451Q_ADDR, buf, 1, 7); // write 1, read 7
    return buf[0]<<24 | buf[1]<<16 | buf[3]<<8 | buf[5]; // (0xSSXXYYZZ)
}

// format acceleration value as 5-digit string "+0.00"
//
// When the full-scale is set to 2g, the measurement range is -2g to
// +1.99975g, and each count corresponds to 1g/4096 (0.25 mg) at 14-bits
// resolution. When the full-scale is set to 8g, the measurement range
// is -8g to +7.999g, and each count corresponds to 1g/1024 (0.98 mg)
// at 14-bits resolution. The resolution is reduced by a factor of 64
// if only the 8-bit results are used. For more information on the
// data manipulation between data formats and modes, refer to
// Freescale application.
void blipper_accel_str (u1_t acc, u1_t* str) {
    if(acc & 0x80) {
        str[0] = '-';
        acc = 256 - acc;
    } else {
        str[0] = '+';
    }
    u2_t g = acc*200/128; // map values 0..127 to 0..200 (0..2g)
    str[4] = (g % 10) + '0';
    g /= 10;
    str[3] = (g % 10) + '0';
    g /= 10;
    str[2] = '.';
    str[1] = (g % 10) + '0';
}


//////////////////////////////////////////////////////////////////////
// IO Expander SX1509
//////////////////////////////////////////////////////////////////////

//  RegDirB                  RegDirA
//  B7 B6 B5 B4 B3 B2 B1 B0  A7 A6 A5 A4 A3 A2 A1 A0
//  15 14 13 12 11 10 09 08  07 06 05 04 03 02 01 00
//  IN YE GR RE IN IN IN IN  IN IN IN IN IN IN IN IN  0x8F 0xFF

#define SX1509_ADDR         (0x3E<<1)
#define SX1509_RegDirB       0x0E
#define SX1509_RegDirA       0x0F
#define SX1509_RegDataB      0x10
#define SX1509_RegDataA      0x11

// initialize SX1509 IO expander
void blipper_ioexp_init (u2_t dir, u2_t data) {
    u1_t buf[5] = { SX1509_RegDirB };
    
    if(dir == 0x0000) { // use default
        dir  = 0x8FFF; // direction of LEDs OUT, all other IN
        data = 0x7000; // output values for LEDs HIGH (OFF)
    }

    buf[1] = dir >> 8;
    buf[2] = dir;
    buf[3] = data >> 8;
    buf[4] = data;

    i2c_xfer(SX1509_ADDR, buf, 5, 0); // write 5, read 0
}

// get pin states of IO expander
u2_t blipper_ioexp_read (void) {
    u1_t buf[2] = { SX1509_RegDataB };
    i2c_xfer(SX1509_ADDR, buf, 1, 2); // write 1, read 2
    return buf[0]<<8 | buf[1];
}

// set pin states of IO expander
void blipper_ioexp_write (u2_t data) {
    u1_t buf[3] = { SX1509_RegDataB, data >> 8, data };
    i2c_xfer(SX1509_ADDR, buf, 3, 0); // write 3, read 0
}

// get LED state (no 0-2)
u1_t blipper_led_get (u1_t no) {
    return ((blipper_ioexp_read() & (1 << 12+no)) == 0);
}

// set LED state (no 0-2, mode 0=off, 1=on, 2=toggle)
void blipper_led_set (u1_t no, u1_t mode) {
    u2_t state = blipper_ioexp_read();
    u2_t mask = 1 << 12+no;
    switch(mode) {
      case 0: state |= mask;  break; // off, set to high
      case 1: state &= ~mask; break; // on, set to low
      case 2: state ^= mask;  break; // toggle, invert
    }
    blipper_ioexp_write(state);
}


//////////////////////////////////////////////////////////////////////
// 3-Axis Magnetometer sensor MAG3110
//////////////////////////////////////////////////////////////////////

// TBD


//////////////////////////////////////////////////////////////////////
// GPS module UP501
//////////////////////////////////////////////////////////////////////

// TBD
