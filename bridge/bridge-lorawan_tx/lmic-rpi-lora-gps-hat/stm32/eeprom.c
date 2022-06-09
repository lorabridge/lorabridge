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

#include "hw.h"
#include "lmic.h"

// write 32-bit word to EEPROM memory
void eeprom_write (u4_t* addr, u4_t val) {
    // check previous value
    if( *addr != val ) {
        // unlock data eeprom memory and registers
        FLASH->PEKEYR = 0x89ABCDEF; // FLASH_PEKEY1
        FLASH->PEKEYR = 0x02030405; // FLASH_PEKEY2

        // only auto-erase if neccessary (when content is non-zero)
        FLASH->PECR &= ~FLASH_PECR_FTDW; // clear FTDW

        // write value
        *addr = val;

        // check for end of programming
        while(FLASH->SR & FLASH_SR_BSY); // loop while busy

        // lock data eeprom memory and registers
        FLASH->PECR |= FLASH_PECR_PELOCK;

        // verify value
        while( *(volatile u4_t*)addr != val ); // halt on mismatch
    }
}

void eeprom_copy (void* dst, const void* src, u2_t len) {
    while(((u4_t)dst & 3) || ((u4_t)src & 3) || (len & 3)); // halt if not multiples of 4
    u4_t* d = (u4_t*)dst;
    u4_t* s = (u4_t*)src;
    u2_t  l = len/4;

    while(l--) {
        eeprom_write(d++, *s++);
    }
}
