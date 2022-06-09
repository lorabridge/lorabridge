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


//////////////////////////////////////////////////////////////////////
// Altimeter, Thermometer and Pressure sensor MPL3115A2
//////////////////////////////////////////////////////////////////////

// init temperature reading on Freescale's MPL3115A2 sensor
void blipper_temp_init (void);

// read temperature from Freescale's MPL3115A2 sensor 
u2_t blipper_temp_read (void);

// read altitude from Freescale's MPL3115A2 sensor 
s2_t blipper_alt_read (void);

// format temp value as 5-digit string "+99.9"
void blipper_temp_str(u2_t temp, u1_t* str);


//////////////////////////////////////////////////////////////////////
// 3-Axis Accelerometer sensor MMA8451Q
//////////////////////////////////////////////////////////////////////

// init accelerometer reading from Freescale's MMA8451Q sensor
void blipper_accel_init (void);

// read accelaration from Freescale's MMA8451Q sensor (0xSSXXYYZZ)
u4_t blipper_accel_read (void);

// format acceleration value as 5-digit string "+0.00"
void blipper_accel_str (u1_t acc, u1_t* str);


//////////////////////////////////////////////////////////////////////
// IO Expander SX1509
//////////////////////////////////////////////////////////////////////

// initialize SX1509 IO expander
void blipper_ioexp_init (u2_t dir, u2_t data);

// get pin states of IO expander
u2_t blipper_ioexp_read (void);

// set pin states of IO expander
void blipper_ioexp_write (u2_t data);

enum { LED_RED=0, LED_GREEN, LED_YELLOW };

// get LED state (no 0-2)
void blipper_led_get (u1_t no);

// set LED state (no 0-2, mode 0=off, 1=on, 2=toggle)
void blipper_led_set (u1_t no, u1_t mode);



