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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "gpio.h"

/**
 * Ask the kernel to export control of a GPIO to userspace by writing its number to file
 * /sys/class/gpio/export.
 */
void gpioExportPin(int pin) {
   static const char* fn = "/sys/class/gpio/export";
   int fd = open(fn, O_WRONLY);
   if(fd < 0) {
      perror(fn);
      exit(1);
   }

   char str_pin[16];
   snprintf(str_pin, 16, "%d", pin);

   size_t rc = write(fd, str_pin, strlen(str_pin));
   if(rc != strlen(str_pin)) {
      perror("exportPin");
      exit(1);
   }

   close(fd);
}

/**
 * Reverses the effect of exporting a GPIO to userspace by writing its number to file
 * /sys/class/gpio/unexport.
 */
void gpioUnexportPin(int pin) {
   static const char* fn = "/sys/class/gpio/unexport";
   int fd = open(fn, O_WRONLY);
   if(fd < 0) {
      perror(fn);
      exit(1);
   }

   char str_pin[16];
   snprintf(str_pin, 16, "%d", pin);

   size_t rc = write(fd, str_pin, strlen(str_pin));
   if(rc != strlen(str_pin)) {
      // perror("gpioUnexportPin");
      // exit(1);
   }
	
   close(fd);
}

/**
 * Sets the direction of a GPIO pin.
 * Reads as either "in" or "out".  This value may normally be written.  
 * Writing as "out" defaults to initializing the value as low.  
 * To ensure glitch free operation, values "low" and "high" may be written to
 * configure the GPIO as an output with that initial value.
 */
void gpioSetPinDirection(int pin, const char* direction) {

   const int BUFSIZE = 64;
   char fn[BUFSIZE];
   snprintf(fn, BUFSIZE, "/sys/class/gpio/gpio%d/direction", pin);
   int fd = open(fn, O_WRONLY);
   if(fd < 0)  {
      perror(fn);
      exit(1);
   }

   size_t rc = write(fd, direction, strlen(direction));
   if(rc != strlen(direction)) {
      perror("gpioSetPinDirection");
      exit(1);
   }

   close(fd);
}

/**
 * Sets the signal edge(s) that will make poll(2) on the "value" file return.
 * Reads as either "none", "rising", "falling", or "both". 
 * This file exists only if the pin can be configured as an interrupt generating input pin.
 */
void gpioSetPinEdge(int pin, void* edge) {

   const int BUFSIZE = 64;
   char fn[BUFSIZE];
   snprintf(fn, BUFSIZE, "/sys/class/gpio/gpio%d/edge", pin);
   int fd = open(fn, O_WRONLY);
   if(fd < 0)  {
      perror(fn);
      exit(EXIT_FAILURE);
   }

   size_t rc = write(fd, edge, strlen(edge));
   if(rc != strlen(edge)) {
      perror("setPinEdge");
      exit(EXIT_FAILURE);
   }


   close(fd);
}

int gpioWaitForInterrupt(const int *pins, size_t npins, int timeout_ms) {

   struct pollfd pl[npins];

   const int BUFSIZE = 64;
   char fn[BUFSIZE];

   for (int i=0 ; i<npins ; i++) {
      snprintf(fn, BUFSIZE, "/sys/class/gpio/gpio%d/value", pins[i]);
      int fd = open(fn, O_RDONLY);
      if(fd < 0)  {
         perror(fn);
         exit(EXIT_FAILURE);
      }

      // Clear the interrupt by reading
      char c;
      int rc = read(fd, &c, 1);
      if (rc < 0) {
         perror("Clear interrupt.");
         exit(EXIT_FAILURE);
      }

      pl[i].fd = fd;
      pl[i].events = POLLPRI | POLLERR;
   }

   int rc = poll(pl, npins, timeout_ms);
   if(rc < 0) {
      fprintf(stderr, "HAL: Cannot poll: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
   }

   // Timeout
   if (rc == 0) {
      for (int i=0 ; i<npins ; i++) {
         close(pl[i].fd);
      }

      return 0;
   }

   rc = 0;
   for (int i=0 ; i<npins ; i++) {
      close(pl[i].fd);

      if (pl[i].revents > 0) {
         rc |= (1<<i);
      }
   }

   return rc;
}
