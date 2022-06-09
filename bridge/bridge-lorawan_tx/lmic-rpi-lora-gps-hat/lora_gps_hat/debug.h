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

// intialize debug library
void debug_init (void);

// set LED state
void debug_led (int val);

// write character to USART
void debug_char (char c);

// write byte as two hex digits to USART
void debug_hex (u1_t b);

// write buffer as hex dump to USART
void debug_buf (const u1_t* buf, int len);

// write 32-bit integer as eight hex digits to USART
void debug_uint (u4_t v);

// write 32-bit integer as signed decimal digits to USART
void debug_int (s4_t v);

// write nul-terminated string to USART
void debug_str (const char* str);

// write LMiC event name to USART
void debug_event (int ev);

// write label and 32-bit value as hex to USART
void debug_val (const char* label, u4_t val);

// write label and 32-bit value as signed decimal to USART
void debug_valdec (const char* label, s4_t val);

// convert integer 'val' to ASCII string (bin/oct/dec/hex/base36)
// store string at 'buf', return number of characters written
int debug_fmt (char* buf, int max, s4_t val, int base, int width, char pad);
