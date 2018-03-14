/*
FHT for arduino - hartley transform
guest openmusiclabs.com 9.1.12
this is a speed optimized program
for calculating an N point FHT on a block of data
please read the read_me file for more info

modded 7.7.14 - fixed progmem for new avr-gcc (thanks to forum user kirill9617)
11.15.16 - guest - fixed STRINGIFY issue by adding some spaces, fixed progmem change by adding __attribute__((used)), and fixed clobbers to eliminate non-clobbered registers.
*/

#ifndef _fht_h // include guard
#define _fht_h

#define STRINGIFY_(a) #a
#define STRINGIFY(a) STRINGIFY_(a)

#ifndef FHT_N // number of samples
  #define FHT_N 256
#endif

#ifndef SCALE // scaling factor for lin8 output function
  #define SCALE 1
#endif

#ifndef WINDOW // wether using the window function or not
  #define WINDOW 1
#endif

#ifndef OCT_NORM // wether using the octave normilization
  #define OCT_NORM 1
#endif

#ifndef REORDER // wether using the reorder function or not
  #define REORDER 1
#endif

#ifndef LOG_OUT // wether using the log output function or not
  #define LOG_OUT 0
#endif

#ifndef LIN_OUT // wether using the linear output function or not
  #define LIN_OUT 0
#endif

#ifndef LIN_OUT8 // wether using the linear output function or not
  #define LIN_OUT8 0
#endif

#ifndef OCTAVE // wether using the octave output function or not
  #define OCTAVE 0
#endif

#if FHT_N == 256
  #define LOG_N 8
  #define _R_V 8 // reorder value - used for reorder list
#elif  FHT_N == 128
  #define LOG_N 7
  #define _R_V 8
#elif FHT_N == 64
  #define LOG_N 6
  #define _R_V 4
#elif FHT_N == 32
  #define LOG_N 5
  #define _R_V 4
#elif FHT_N == 16
  #define LOG_N 4
  #define _R_V 2
#else
  #error FHT_N value not defined
#endif

#include <avr/pgmspace.h>

extern const int16_t __attribute__((used)) _cas_constants[] PROGMEM = {
#if (FHT_N ==  256)
  #include <cas_lookup_256.inc>
#elif (FHT_N ==  128)
  #include <cas_lookup_128.inc>
#elif (FHT_N ==  64)
  #include <cas_lookup_64.inc>
#elif (FHT_N ==  32)
  #include <cas_lookup_32.inc>
#elif (FHT_N ==  16)
  #include <cas_lookup_16.inc>
#endif
};

#if (REORDER == 1)
  extern const uint8_t __attribute__((used)) _reorder_table[] PROGMEM = {
  #if (FHT_N == 256)
    #include <256_reorder.inc>
  #elif (FHT_N == 128)
    #include <128_reorder.inc>
  #elif (FHT_N == 64)
    #include <64_reorder.inc>
  #elif (FHT_N == 32)
    #include <32_reorder.inc>
  #elif (FHT_N == 16)
    #include <16_reorder.inc>
  #endif
  };
#endif

#if ((LOG_OUT == 1)||(OCTAVE == 1))
  extern const uint8_t __attribute__((used)) _log_table[] PROGMEM = {
    #include <decibel.inc>
  };
#endif

#if (LOG_OUT == 1)
  uint8_t __attribute__((used)) fht_log_out[(FHT_N/2)]; // FHT log output magintude buffer
#endif

#if (LIN_OUT == 1)
  extern const uint8_t __attribute__((used)) _lin_table[] PROGMEM = {
    #include <sqrtlookup16.inc>
  };
  uint16_t __attribute__((used)) fht_lin_out[(FHT_N/2)]; // FHT linear output magintude buffer
#endif

#if (LIN_OUT8 == 1)
  extern const uint8_t __attribute__((used)) _lin_table8[] PROGMEM = {
    #include <sqrtlookup8.inc>
  };
  uint8_t __attribute__((used)) fht_lin_out8[(FHT_N/2)]; // FHT linear output magintude buffer
#endif

#if (OCTAVE == 1)
  uint8_t __attribute__((used)) fht_oct_out[(LOG_N)]; // FHT octave output magintude buffer
#endif

#if (WINDOW == 1) // window functions are in 16b signed format
  extern const int16_t __attribute__((used)) _window_func[] PROGMEM = {
  #if (FHT_N ==  256)
    #include <hann_256.inc>
  #elif (FHT_N ==  128)
    #include <hann_128.inc>
  #elif (FHT_N ==  64)
    #include <hann_64.inc>
  #elif (FHT_N ==  32)
    #include <hann_32.inc>
  #elif (FHT_N ==  16)
    #include <hann_16.inc>
  #endif
  };
#endif


int __attribute__((used)) fht_input[(FHT_N)]; // FHT input data buffer


static inline void fht_run(void) {
  // save registers that are getting clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r9 \n"
  "push r10 \n"
  "push r11 \n"
  "push r12 \n"
  "push r13 \n"
  "push r14 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );
 
  // do first 3 butterflies - only 1 multiply, minimizes data fetches
  // initialize
  asm volatile (
  "clr r15 \n" // clear the null register
  "ldi r16, " STRINGIFY(FHT_N/8) " \n" // prep loop counter
  "mov r14,r16 \n"
  "ldi r28, lo8(fht_input) \n" //set to beginning of data space
  "ldi r29, hi8(fht_input) \n"

  // first set: x0 and x1
  "1: \n"
  "ld r6,y \n" // fetch x0
  "ldd r7,y+1 \n"
  "ldd r4,y+2 \n" // fetch x1
  "ldd r5,y+3 \n"
  "asr r7 \n" // divide by 2 to keep from overflowing
  "ror r6 \n"
  "movw r8,r6 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r6,r4 \n" // x0 + x1
  "adc r7,r5 \n"
  "sub r8,r4 \n" // x0 - x1
  "sbc r9,r5 \n"

  // first set: x2 and x3
  "ldd r10,y+4 \n" // fetch x2
  "ldd r11,y+5 \n"
  "ldd r4,y+6 \n" // fetch x3
  "ldd r5,y+7 \n"
  "asr r11 \n" // divide by 2 to keep from overflowing
  "ror r10 \n"
  "movw r12,r10 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r10,r4 \n" // x2 + x3
  "adc r11,r5 \n"
  "sub r12,r4 \n" // x2 - x3
  "sbc r13,r5 \n"

  // first set: x4 and x5
  "ldd r22,y+8 \n" // fetch x4
  "ldd r23,y+9 \n"
  "ldd r4,y+10 \n" // fetch x5
  "ldd r5,y+11 \n"
  "asr r23 \n" // divide by 2 to keep from overflowing
  "ror r22 \n"
  "movw r16,r22 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r22,r4 \n" // x4 + x5
  "adc r23,r5 \n"
  "sub r16,r4 \n" // x4 - x5
  "sbc r17,r5 \n"

  // first set: x6 and x7
  "ldd r24,y+12 \n" // fetch x6
  "ldd r25,y+13 \n"
  "ldd r4,y+14 \n" // fetch x7
  "ldd r5,y+15 \n"
  "asr r25 \n" // divide by 2 to keep from overflowing
  "ror r24 \n"
  "movw r18,r24 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r24,r4 \n" // x6 + x7
  "adc r25,r5 \n"
  "sub r18,r4 \n" // x6 - x7
  "sbc r19,r5 \n"

  // second set: x0, x1, x2, and x3
  "asr r7 \n" // divide by 2 to keep from overflowing
  "ror r6 \n"
  "movw r4,r6 \n" // make backup
  "asr r11 \n" // divide by 2 to keep from overflowing
  "ror r10 \n"
  "add r6,r10 \n" // (x0 + x1) +  (x2 + x3)
  "adc r7,r11 \n"
  "sub r4,r10 \n" // (x0 + x1) - (x2 + x3)
  "sbc r5,r11 \n"
  "asr r9 \n" // divide by 2 to keep from overflowing
  "ror r8 \n"
  "movw r10,r8 \n" // make backup
  "asr r13 \n" // divide by 2 to keep from overflowing
  "ror r12 \n"
  "add r8,r12 \n" // (x0 - x1) +  (x2 - x3)
  "adc r9,r13 \n"
  "sub r10,r12 \n" // (x0 - x1) - (x2 - x3)
  "sbc r11,r13 \n"

  // second set: x4, x5, x6, and x7
  // negatives arent done as they cancel out in the next step
  "asr r23 \n" // divide by 2 to keep from overflowing
  "ror r22 \n"
  "movw r12,r22 \n" // make backup
  "asr r25 \n" // divide by 2 to keep from overflowing
  "ror r24 \n"
  "add r22,r24 \n" // (x4 + x5) +  (x6 + x7)
  "adc r23,r25 \n"
  "sub r12,r24 \n" // (x4 + x5) - (x6 + x7)
  "sbc r13,r25 \n"

  // third set: c0
  "asr r7 \n" // divide by 2 to keep from overflowing
  "ror r6 \n"
  "movw r2,r6 \n" // make backup
  "asr r23 \n" // divide by 2 to keep from overflowing
  "ror r22 \n"
  "add r6,r22 \n" // [(x0 + x1) + (x2 + x3)] + [(x4 + x5) + (x6 + x7)]
  "adc r7,r23 \n"
  "sub r2,r22 \n" // [(x0 + x1) + (x2 + x3)] - [(x4 + x5) + (x6 + x7)]
  "sbc r3,r23 \n"

  // third set: c1 - multiply by .707
  // process (x4 - x5)*cos - use mulsu to keep scaling
  "ldi r20,0x82 \n" // load multiply register with 0.707
  "ldi r21,0x5a \n"
  "mulsu r17,r21 \n"
  "movw r24,r0 \n"
  "mul r16,r20 \n"
  "movw r22,r0 \n"
  "mulsu r17,r20 \n"
  "sbc r25,r15 \n"
  "add r23,r0 \n"
  "adc r24,r1 \n"
  "adc r25,r15 \n"
  "mul r21,r16 \n"
  "add r23,r0 \n"
  "adc r24,r1 \n"
  "adc r25,r15 \n"

  // add result to other parts
  "asr r9 \n" // divide by 2 to keep from overflowing
  "ror r8 \n"
  "movw r16,r8 \n" // make backup
  "add r8,r24 \n" // [(x0 - x1) + (x2 - x3)] + 2*(x4 - x5)*C1
  "adc r9,r25 \n"
  "sub r16,r24 \n" // [(x0 - x1) + (x2 - x3)] - 2*(x4 - x5)*c1
  "sbc r17,r25 \n"

  // third set: c2
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "movw r26,r4 \n" // make backup
  "asr r13 \n" // divide by 2 to keep from overflowing
  "ror r12 \n"
  "add r4,r12 \n" // [(x0 + x1) - (x2 + x3)] + [(x4 + x5) - (x6 + x7)]
  "adc r5,r13 \n"
  "sub r26,r12 \n" // [(x0 + x1) - (x2 + x3)] - [(x4 + x5) - (x6 + x7)]
  "sbc r27,r13 \n"

  // third set: c3 - multiply by .707
  // process (x6 - x7)*cos - use mulsu to keep scaling
  "mulsu r19,r21 \n"
  "movw r24,r0 \n"
  "mul r18,r20 \n"
  "movw r22,r0 \n"
  "mulsu r19,r20 \n"
  "sbc r25,r15 \n"
  "add r23,r0 \n"
  "adc r24,r1 \n"
  "adc r25,r15 \n"
  "mul r21,r18 \n"
  "add r23,r0 \n"
  "adc r24,r1 \n"
  "adc r25,r15 \n"

  // add result to other parts
  "asr r11 \n" // divide by 2 to keep from overflowing
  "ror r10 \n"
  "movw r12,r10 \n" // make backup
  "add r10,r24 \n" // [(x0 - x1) - (x2 - x3)] + 2*(x6 - x7)*C1
  "adc r11,r25 \n"
  "sub r12,r24 \n" // [(x0 - x1) - (x2 - x3)] - 2*(x6 - x7)*c1
  "sbc r13,r25 \n"

  // restore all the data and repeat as necessary
  "st y+,r6 \n" // store x0
  "st y+,r7 \n"
  "st y+,r8 \n" // store x1
  "st y+,r9 \n"
  "st y+,r4 \n" // store x2
  "st y+,r5 \n"
  "st y+,r10 \n" // store x3
  "st y+,r11 \n"
  "st y+,r2 \n" // store x4
  "st y+,r3 \n"
  "st y+,r16 \n" // store x5
  "st y+,r17 \n"
  "st y+,r26 \n" // store x6
  "st y+,r27 \n"
  "st y+,r12 \n" // store x7
  "st y+,r13 \n"
  "dec r14 \n" // check if at end of data space
  "breq 4f \n"
  "rjmp 1b \n"
  );

  // remainder of the butterflies (fourth and higher)
  // initialize
  asm volatile (
  "4: \n"
  "ldi r16, 0x20 \n" // prep outer loop counter - full stride (x2 for 16b numbers)
  "mov r12,r16 \n"
  "clr r13 \n"
  "ldi r16, hi8((fht_input + " STRINGIFY(FHT_N*2) ")) \n" // prep end of dataspace register
  "mov r9, r16 \n"
  "ldi r30, lo8(_cas_constants) \n" // initialize lookup table address
  "ldi r31, hi8(_cas_constants) \n"
  "ldi r16, 0x10 \n" // prep half stride length (x2 for 16b numbers)
  "mov r10,r16 \n"
  "clr r11 \n"
  "ldi r16,0x04 \n" // load inner loop counter to quarter stride (x1 for counter)
  "mov r14,r16 \n"

  // outer_loop - reset variables for next pass through the butterflies
  "5: \n"
  "ldi r26, lo8(fht_input) \n" //set top pointer to beginning of data space
  "ldi r27, hi8(fht_input) \n"
  "movw r28,r26 \n" // set bottom pointer to top
  "add r28,r10 \n" // add half stride to the bottom pointer
  "adc r29,r11 \n"
  "mov r8,r14 \n" // make backup of inner loop counter

  // inner_loop - do butterflies
  // first one is wk = (1,0)
  "6: \n"
  "ld r2,x+ \n" // fetch top half
  "ld r3,x \n"
  "ld r4,y \n" // fetch bottom half
  "ldd r5,y+1 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r6,r4 \n" // add for top half
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom half
  "sbc r3,r5 \n"
  "st y+,r2 \n" // store bottom half
  "st y+,r3 \n"
  "st x,r7 \n" // store top half
  "st -x,r6 \n"
  "add r26,r12 \n"  // top pointer now becomes bottom pointer
  "adc r27,r13 \n"
  "sbiw r26,0x01 \n" // decrement to next butterfly
  "dec r14 \n" // weve done the first one

  // remainder are regular
  // muls replaced with mulsu to save a few cycles
  "7: \n"
  "ld r16,y \n" // fetch upper half
  "ldd r17,y+1 \n"
  "ld r19,x \n" // fetch lower half
  "ld r18,-x \n"
  "lpm r20,z+ \n" // fetch cosine
  "lpm r21,z+ \n"
  "lpm r22,z+ \n" // fetch sine
  "lpm r23,z+ \n"

   // process upper*cos
  "muls r17,r21 \n"
  "movw r4,r0 \n"
  "mul r16,r20 \n"
  "movw r2,r0 \n"
  "mulsu r17,r20 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "mul r21,r16 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process lower*sin and accumulate
  "muls r19,r23 \n"
  "movw r6,r0 \n"
  "mul r18,r22 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "mulsu r19,r22 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "mul r23,r18 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // do upper butterfly - faster than doing a seperate loop
  "movw r0,r28 \n" // backup pointer
  "sub r28,r10 \n" // index to top half - subtract half stride
  "sbc r29,r11 \n"
  "ld r2,y \n" // fetch top half
  "ldd r3,y+1 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "add r6,r4 \n" // add for top half
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom half
  "sbc r3,r5 \n"
  "st y,r6 \n" // store top half
  "std y+1,r7 \n"
  "movw r28,r0 \n" // restore pointer
  "st y+,r2 \n" // store bottom half
  "st y+,r3 \n"

  // process upper*sin
  "muls r17,r23 \n"
  "movw r4,r0 \n"
  "mul r16,r22 \n"
  "movw r2,r0 \n"
  "mulsu r17,r22 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "mul r23,r16 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process lower*cos and accumulate (subtract)
  "muls r19,r21 \n"
  "movw r6,r0 \n"
  "mul r18,r20 \n"
  "sub r2,r0 \n"
  "sbc r3,r1 \n"
  "sbc r4,r6 \n"
  "sbc r5,r7 \n"
  "mulsu r19,r20 \n"
  "adc r5,r15 \n"
  "sub r3,r0 \n"
  "sbc r4,r1 \n"
  "sbc r5,r15 \n"
  "mul r21,r18 \n"
  "sub r3,r0 \n"
  "sbc r4,r1 \n"
  "sbc r5,r15 \n"

  // do lower butterfly - faster than doing a seperate loop
  "movw r0,r26 \n" // backup pointer
  "sub r26,r10 \n" // index to top half - subtract half stride
  "sbc r27,r11 \n"
  "ld r2,x+ \n" // fetch top half
  "ld r3,x \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "add r6,r4 \n" // add for top half
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom half
  "sbc r3,r5 \n"
  "st x,r7 \n" // store top half
  "st -x,r6 \n"
  "movw r26,r0 \n" // restore pointer
  "st x+,r2 \n" // store bottom half
  "st x,r3 \n"
  "sbiw r26,0x02 \n" // decrement to next butterfly

  // check where we are in the process
  "dec r14 \n" // check which butterfly were doing
  "breq 8f \n" // finish off if last one
  "rjmp 7b \n" // go back and do it again if not done yet

  // last buttefly is wk = (0,1)
  "8: \n"
  "movw r26,r28 \n" // backup pointer
  "sub r26,r10 \n" // index to top half - subtract half stride
  "sbc r27,r11 \n"
  "ld r2,x+ \n" // fetch top half
  "ld r3,x \n"
  "ld r4,y \n" // fetch bottom half
  "ldd r5,y+1 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing 
  "ror r4 \n"
  "add r6,r4 \n" // add for top half
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom half
  "sbc r3,r5 \n"
  "st y,r2 \n" // store bottom half
  "std y+1,r3 \n"
  "st x,r7 \n" // store top half
  "st -x,r6 \n"

  // reset for next pass
  "movw r24,r10 \n" // make a quarter stride
  "lsr r25 \n"
  "ror r24 \n"
  "add r28,r24 \n" // lower is incremented by quarter stride to get to the next butterfly
  "adc r29,r25 \n"
  "cpi r28, lo8(fht_input + " STRINGIFY(FHT_N*2) ") \n" // check if at end of dataspace
  "cpc r29, r9 \n"
  "brsh 10f \n"
  "movw r26,r28 \n" // bottom is now top
  "add r28,r10 \n" // bottom is moved down half a stride
  "adc r29,r11 \n"
  "mov r14,r8 \n" // reset inner loop counter
  "sub r30,r10 \n" // reset Wk lookup table pointer
  "sbc r31,r11 \n"
  "adiw r30,0x04 \n" // number of butterflies minus 1 for the ones not done
  "rjmp 6b \n" // keep going

  // inner_done - reset for next set of butteflies
  "10: \n"
  "sbrc r8, " STRINGIFY(LOG_N - 2) " \n" // check if finished with all butteflies
  "rjmp 11f \n"
  "mov r14,r10 \n" // set inner loop count to half stride - works for N <= 256
  "lsr r14 \n" // divide inner loop counter by 2
  "movw r10,r12 \n" // set new half stride to old full stride
  "lsl r12 \n" // multiply full stride by 2
  "rol r13 \n"
  "rjmp 5b \n" // keep going
  "11: \n" // rest of code here
  : :
  : "r0", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r30", "r31" // clobber list for whole thing
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r14 \n"
  "pop r13 \n"
  "pop r12 \n"
  "pop r11 \n"
  "pop r10 \n"
  "pop r9 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}

static inline void fht_reorder(void) {
  // save registers that are getting clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r6 \n"
  "push r7 \n"
  "push r28 \n"
  "push r29 \n"
  );


  // move values to bit reversed locations
  asm volatile (
  "ldi r30, lo8(_reorder_table) \n" // initialize lookup table address
  "ldi r31, hi8(_reorder_table) \n"
  "ldi r20, " STRINGIFY((FHT_N/2) - _R_V) " \n" // set to first sample

  // get source sample
  "1: \n"
  "lpm r26,z+ \n" // fetch source address
  "clr r27 \n" // these next 3 lines could be optimized out
  "lsl r26 \n" // by chaging the lookup table
  "rol r27 \n" // only works for FHT_N <= 128
  "subi r26, lo8(-(fht_input)) \n" // pointer to offset
  "sbci r27, hi8(-(fht_input)) \n"
  "ld r2,x+ \n" // fetch source
  "ld r3,x \n"

  // find destination
  "lpm r28,z+ \n"
  "clr r29 \n" // same here
  "lsl r28 \n" // multiply offset by 2
  "rol r29 \n"
  "subi r28, lo8(-(fht_input)) \n" // add pointer to offset
  "sbci r29, hi8(-(fht_input)) \n"
  "ld r6,y \n" // fetch destination
  "ldd r7,y+1 \n"

  // swap source and destination samples
  "st x,r7 \n"
  "st -x,r6 \n"
  "st y,r2 \n"
  "std y+1,r3 \n"

  // check if done
  "dec r20 \n" // go to next sample
  "brne 1b \n" // finish off if last sample
  : :
  : "r20", "r26", "r27", "r30", "r31" // clobber list
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r3 \n"
  "pop r2 \n"
  );
}


static inline void fht_mag_log(void) {
  // save registers that are getting clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r9 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns an 8b unsigned value which is 16*log2((img^2 + real^2)^0.5)
  asm volatile (
  "ldi r26, lo8(fht_input) \n" // set to beginning of data space
  "ldi r27, hi8(fht_input) \n"
  "ldi r28, lo8(fht_log_out) \n" // set to beginning of result space
  "ldi r29, hi8(fht_log_out) \n"
  "ldi r30, lo8(fht_input + " STRINGIFY(FHT_N*2) ") \n" // set to end of data space
  "ldi r31, hi8(fht_input + " STRINGIFY(FHT_N*2) ") \n"
  "movw r8,r30 \n" // z register clobbered below
  "clr r15 \n" // clear null register
  "ldi r20, " STRINGIFY(FHT_N/2) " \n" // set loop counter
  "ld r16,x+ \n" // do zero frequency bin first
  "ld r17,x+ \n"
  "movw r18,r16 \n" // double zero frequency bin
  "rjmp 10f \n" // skip ahead

  "1: \n"
  "movw r30,r8 \n" // restore z register
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r19,-Z \n" // fetch imaginary
  "ld r18,-Z \n"
  "movw r8,r30 \n" // store z register

  // process real^2
  "10: \n"
  "muls r17,r17 \n"
  "movw r4,r0 \n"
  "mul r16,r16 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r6,r0 \n"
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // decibel of the square root via lookup table
  // scales the magnitude to an 8b value times an 8b exponent
  "clr r17 \n" // clear exponent register
  "tst r5 \n"
  "breq 3f \n"
  "ldi r17,0x0c \n"
  "mov r30,r5 \n"

  "2: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r4 \n"
  "rol r30 \n"
  "lsl r4 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 2b \n"

  "3: \n"
  "tst r4 \n"
  "breq 5f \n"
  "ldi r17,0x08 \n"
  "mov r30,r4 \n"

  "4: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r3 \n"
  "rol r30 \n"
  "lsl r3 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 4b \n"

  "5: \n"
  "tst r3 \n"
  "breq 7f \n"
  "ldi r17,0x04 \n"
  "mov r30,r3 \n"

  "6: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r2 \n"
  "rol r30 \n"
  "lsl r2 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 6b \n"

  "7: \n"
  "mov r30,r2 \n"

  "8: \n"
  "clr r31 \n"
  "subi r30, lo8(-(_log_table)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_log_table)) \n"
  "lpm r16,z \n" // fetch log compressed square root
  "swap r17 \n"  // multiply exponent by 16
  "add r16,r17 \n" // add for final value
  "st y+,r16 \n" // store value
  "dec r20 \n" // check if all data processed
  "breq 9f \n"
  "rjmp 1b \n"
  "9: \n" // all done
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20" // clobber list
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r9 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset c compiler null register
  );
}

static inline void fht_mag_lin(void) {
  // save registers that are getting clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns an 16b unsigned value which is 16*((img^2 + real^2)^0.5)
  asm volatile (
  "ldi r26, lo8(fht_input) \n" // set to beginning of data space
  "ldi r27, hi8(fht_input) \n"
  "ldi r28, lo8(fht_lin_out) \n" // set to beginning of result space
  "ldi r29, hi8(fht_lin_out) \n"
  "ldi r30, lo8(fht_input + " STRINGIFY(FHT_N*2) ") \n" // set to end of data space
  "ldi r31, hi8(fht_input + " STRINGIFY(FHT_N*2) ") \n"
  "movw r8,r30 \n" // z register clobbered below
  "clr r15 \n" // clear null register
  "ldi r20, " STRINGIFY(FHT_N/2) " \n" // set loop counter
  "ld r16,x+ \n" // do zero frequency bin first
  "ld r17,x+ \n"
  "movw r18,r16 \n" // double zero frequency bin
  "rjmp 20f \n" // skip ahead

  "1: \n"
  "movw r30,r8 \n" // restore z register
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r19,-Z \n" // fetch imaginary
  "ld r18,-Z \n"
  "movw r8,r30 \n" // store z register

  // process real^2
  "20: \n"
  "muls r17,r17 \n"
  "movw r4,r0 \n"
  "mul r16,r16 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r6,r0 \n"
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // square root via lookup table
  // first scales the magnitude to a 16b value times an 8b exponent
  "clr r17 \n" // clear exponent register
  "tst r5 \n"
  "breq 3f \n"
  "ldi r17,0x08 \n"
  "movw r30,r4 \n"

  "2: \n"
  "cpi r31,0x40 \n"
  "brsh 6f \n" // all values already known to be > 0x40
  "lsl r3 \n"
  "rol r30 \n"
  "rol r31 \n"
  "lsl r3 \n"
  "rol r30 \n"
  "rol r31 \n"
  "dec r17 \n"
  "rjmp 6f \n"

  "3: \n"
  "tst r4 \n"
  "breq 5f \n"
  "ldi r17,0x04 \n"
  "mov r31,r4 \n"
  "mov r30,r3 \n"

  "4: \n"
  "cpi r31,0x40 \n"
  "brsh 6f \n" // all values already known to be > 0x40
  "lsl r2 \n"
  "rol r30 \n"
  "rol r31 \n"
  "lsl r2 \n"
  "rol r30 \n"
  "rol r31 \n"
  "dec r17 \n"
  "rjmp 4b \n"

  // find sqrt via lookup table
  "5: \n"
  "movw r30,r2 \n"
  "cpi r31,0x40 \n"
  "brsh 6f \n"
  "cpi r31,0x10 \n"
  "brsh 12f \n"
  "cpi r31,0x01 \n"
  "brlo 10f \n"
  "swap r31 \n"
  "swap r30 \n"
  "andi r30,0x0f \n"
  "or r30,r31 \n"
  "lsr r30 \n"
  "ldi r31,0x01 \n"
  "rjmp 10f \n"
 
  "6: \n"
  "mov r30,r31 \n"
  "ldi r31,0x02 \n"
  "rjmp 10f \n"

  "12: \n"
  "lsl r30 \n"
  "rol r31 \n"
  "mov r30,r31 \n"
  "ori r30,0x80 \n"
  "ldi r31,0x01 \n"

  "10: \n"
  "subi r30, lo8(-(_lin_table)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_lin_table)) \n"
  "lpm r16,z \n" // fetch square root
  "clr r18 \n"

  "7: \n" // multiply by exponent
  "tst r17 \n"
  "breq 8f \n" // skip if no exponent
  "13: \n"
  "lsl r16 \n"
  "rol r18 \n"
  "dec r17 \n"
  "brne 13b \n"

  "8: \n"
  "st y+,r16 \n" // store value
  "st y+,r18 \n"
  "dec r20 \n" // check if all data processed
  "breq 9f \n"
  "rjmp 1b \n"
  "9: \n" // all done
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20"// clobber list
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset c compiler null register
  );
}

static inline void fht_mag_lin8(void) {
  // save registers that are getting clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns an 8b unsigned value which is (225/(181*256*256))*((img^2 + real^2)^0.5)
  asm volatile (
  "ldi r26, lo8(fht_input) \n" // set to beginning of data space
  "ldi r27, hi8(fht_input) \n"
  "ldi r28, lo8(fht_lin_out8) \n" // set to beginning of result space
  "ldi r29, hi8(fht_lin_out8) \n"
  "ldi r30, lo8(fht_input + " STRINGIFY(FHT_N*2) ") \n" // set to end of data space
  "ldi r31, hi8(fht_input + " STRINGIFY(FHT_N*2) ") \n"
  "movw r8,r30 \n" // z register clobbered below
  "clr r15 \n" // clear null register
  "ldi r20, " STRINGIFY(FHT_N/2) " \n" // set loop counter
  "ld r16,x+ \n" // do zero frequency bin first
  "ld r17,x+ \n"
  "movw r18,r16 \n" // double zero frequency bin
  "rjmp 20f \n" // skip ahead

  "1: \n"
  "movw r30,r8 \n" // restore z register
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r19,-Z \n" // fetch imaginary
  "ld r18,-Z \n"
  "movw r8,r30 \n" // store z register

  // process real^2
  "20: \n"
  "muls r17,r17 \n"
  "movw r4,r0 \n"
  "mul r16,r16 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r6,r0 \n"
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

#if (SCALE == 1)
  "movw r30,r4 \n"
#elif (SCALE == 2)
  "lsl r3 \n"
  "rol r4 \n"
  "rol r5 \n"
  "movw r30,r4 \n"
#elif (SCALE == 4)
  "lsl r3 \n"
  "rol r4 \n"
  "rol r5 \n"
  "lsl r3 \n"
  "rol r4 \n"
  "rol r5 \n"
  "movw r30,r4 \n"
#elif (SCALE == 128)
  "lsr r5 \n"
  "ror r4 \n"
  "ror r3 \n"
  "mov r31,r4 \n"
  "mov r30,r3 \n"
#elif (SCALE == 256)
  "mov r30,r3 \n"
  "mov r31,r4 \n"
#else
  "ldi r18, " STRINGIFY(SCALE) " \n"
  "mul r5,r18 \n"
  "mov r31,r0 \n"
  "mul r3,r18 \n"
  "mov r30,r1 \n"
  "mul r4,r18 \n"
  "add r30,r0 \n"
  "adc r31,r1 \n"
#endif

  // square root via lookup table
  // scales the magnitude to an 8b value
  "cpi r31,0x10 \n"
  "brsh 2f \n"
  "cpi r31,0x01 \n"
  "brsh 3f \n"
  "rjmp 6f \n"

  "2: \n"
  "lsl r30 \n"
  "rol r31 \n"
  "mov r30,r31 \n"
  "ldi r31,0x01 \n"
  "subi r30,0x80 \n"
  "sbci r31,0xff \n"
  "rjmp 6f \n"

  "3: \n"
  "swap r30 \n"
  "swap r31 \n"
  "andi r30,0x0f \n"
  "or r30,r31 \n"
  "lsr r30 \n"
  "ldi r31,0x01 \n"

  "6: \n"
  "subi r30, lo8(-(_lin_table8)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_lin_table8)) \n"
  "lpm r16,z \n" // fetch log compressed square root
  "st y+,r16 \n" // store value
  "dec r20 \n" // check if all data processed
  "breq 9f \n"
  "rjmp 1b \n"
  "9: \n" // all done
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20"// clobber list
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset c compiler null register
  );
}

static inline void fht_window(void) {
  // save registers that are getting clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this applies a window to the data for better frequency resolution
  asm volatile (
  "ldi r28, lo8(fht_input) \n" // set to beginning of data space
  "ldi r29, hi8(fht_input) \n"
  "ldi r30, lo8(_window_func) \n" // set to beginning of lookup table
  "ldi r31, hi8(_window_func) \n"
  "clr r15 \n" // prep null register
  "ldi r20, " STRINGIFY(((FHT_N)&(0xff))) " \n"

  "1: \n"
  "lpm r22,z+ \n" // fetch window value
  "lpm r23,z+ \n"
  "ld r16,y \n" // fetch data
  "ldd r17,y+1 \n"

  // multiply by window
  "fmuls r17,r23 \n"
  "movw r4,r0 \n"
  "fmul r16,r22 \n"
  "adc r4,r15 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r22 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "fmulsu r23,r16 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  "st y+,r4 \n" // restore data
  "st y+,r5 \n"
  "dec r20 \n" // check if done
  "brne 1b \n"
  : :
  : "r0", "r20", "r30", "r31", "r22", "r23" // clobber list
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset c compiler null register
  );
}


static inline void fht_mag_octave(void) {
  // save registers that are getting clobbered
  // avr-gcc only requires r2:r17,r28:r29, and r1 cleared
  // but im doing them all to be safe
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r9 \n"
  "push r10 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns the energy in the sum of bins within an octave (doubling of frequencies)
  asm volatile (
  "ldi r26, lo8(fht_input) \n" // set to beginning of data space
  "ldi r27, hi8(fht_input) \n"
  "ldi r28, lo8(fht_oct_out) \n" // set to beginning of result space
  "ldi r29, hi8(fht_oct_out) \n"
  "ldi r30, lo8(fht_input + " STRINGIFY(FHT_N*2) ") \n" // set to end of data space
  "ldi r31, hi8(fht_input + " STRINGIFY(FHT_N*2) ") \n"
  "movw r10,r30 \n" // z register clobbered below
  "clr r15 \n" // clear null register
  "ldi r20, 0x01 \n" // set first bin check (needed to make sequence 1-1-2-4-etc)
  "ldi r21, 0x01 \n" // set loop counter
  "mov r22,r21 \n" // make backup of counter for usage
  "clr r2 \n" // clear the accumulator
  "clr r3 \n"
  "movw r4,r2 \n"
  "clr r6 \n"
  "ld r16,x+ \n" // do zero frequency bin first
  "ld r17,x+ \n"
  "movw r18,r16 \n" // double zero frequency bin
  "rjmp 30f \n" // skip ahead

  "13: \n"
  "clr r20 \n"

  "10: \n"
  "mov r22,r21 \n" // make backup of counter for usage
  "clr r2 \n" // clear the accumulator
  "clr r3 \n"
  "movw r4,r2 \n"
  "clr r6 \n"

  "1: \n"
  "movw r30,r10 \n" // restore z register
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r19,-Z \n" // fetch imaginary
  "ld r18,-Z \n"
  "movw r10,r30 \n" // store z register

  // process real^2
  "30: \n"
  "muls r17,r17 \n"
  "movw r8,r0 \n" // dont need an sbc as the result is always positive
  "mul r16,r16 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r8 \n"
  "adc r5,r9 \n"
  "adc r6,r15 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "sbc r6,r15 \n" // need to carry, might overflow if r5 = 0
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "adc r6,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r8,r0 \n" // dont need an sbc as the result is always positive
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r8 \n"
  "adc r5,r9 \n"
  "adc r6,r15 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "sbc r6,r15 \n" // need to carry, might overflow if r5 = 0
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "adc r6,r15 \n"

  // check if summation done
  "dec r22 \n"
  "brne 1b \n"

#if (OCT_NORM == 1) // put normilisation code in if needed
  "mov r22,r21 \n"
  "lsr r22 \n" // check if done
  "brcs 12f \n"

  "11: \n"
  "lsr r6 \n"
  "ror r5 \n"
  "ror r4 \n"
  "ror r3 \n"
  "ror r2 \n"
  "lsr r22 \n" // check if done
  "brcc 11b \n"
#endif

  // decibel of the square root via lookup table
  // scales the magnitude to an 8b value times an 8b exponent
  "12: \n"
  "clr r17 \n" // clear exponent register
  "tst r5 \n"
  "breq 3f \n"
  "ldi r17,0x0c \n"
  "mov r30,r5 \n"

  "2: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r4 \n"
  "rol r30 \n"
  "lsl r4 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 2b \n"

  "3: \n"
  "tst r4 \n"
  "breq 5f \n"
  "ldi r17,0x08 \n"
  "mov r30,r4 \n"

  "4: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r3 \n"
  "rol r30 \n"
  "lsl r3 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 4b \n"

  "5: \n"
  "tst r3 \n"
  "breq 7f \n"
  "ldi r17,0x04 \n"
  "mov r30,r3 \n"

  "6: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r2 \n"
  "rol r30 \n"
  "lsl r2 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 6b \n"

  "7: \n"
  "mov r30,r2 \n"

  "8: \n"
  "clr r31 \n"
  "subi r30, lo8(-(_log_table)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_log_table)) \n"
  "lpm r16,z \n" // fetch log compressed square root
  "swap r17 \n"  // multiply exponent by 16
  "add r16,r17 \n" // add for final value
  "st y+,r16 \n" // store value
  "sbrc r20, 0x00 \n" // check if first 2 bins done
  "rjmp 13b \n"
  "lsl r21 \n"
  "sbrs r21, " STRINGIFY((LOG_N) - 1) " \n" // check if done
  "rjmp 10b \n"
  : :
  : "r0", "r26", "r27", "r30", "r31","r18", "r19", "r20","r21", "r22" // clobber list
  );

  // get the clobbers off the stack
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r10 \n"
  "pop r9 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset c compiler null register
  );
}

#endif // end include guard

