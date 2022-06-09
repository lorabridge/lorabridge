//
// BSD 3-Clause License
// 
// Hardware Abstraction Layer (HAL) targeted to Raspberry Pi and 
// Dragino LoRa/GPS HAT
// 
// Copyright (c) 2017, Wolfgang Klenk
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#ifndef GPIO_H
#define GPIO_H

//
// Implement some functionalities that WiringPi does not provide.
//
// Enables us to use OS call "poll" to _blocking_ check if there
// was a transition on of several input pins.
//

static char *GPIO_DIRECTION_IN = "in";
static char *GPIO_DIRECTION_OUT = "out";
static char *GPIO_DIRECTION_OUT_HIGH = "high";
static char *GPIO_DIRECTION_OUT_LOW = "low";

static void* GPIO_EDGE_NONE = "none";
static void* GPIO_EDGE_RISING = "rising";
static void* GPIO_EDGE_FALLING = "falling";
static void* GPIO_EDGE_BOTH = "both";

void gpioExportPin(int pin);
void gpioUnexportPin(int pin);
void gpioSetPinDirection(int pin, const char *direction);
void gpioSetPinEdge(int pin, void* edge);
int gpioWaitForInterrupt(const int *pins, size_t npins, int timeout_ms);

#endif
