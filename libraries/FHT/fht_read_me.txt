fht_read_me.txt
explanation of how to use the arduino fht (hartley transform) libarary
guest openmusiclabs.com 9.4.12
guest openmusiclabs.com 10.25.12 -> register saving added - local data was
getting clobbered by the functions.  this will change the run times from
what is shown below.

this fht runs on 16b real inputs, and returns either 8b linear,
16b linear, or 8b logarithmic outputs.  it can handle an fht with anywhere
from 16 -> 256 samples, and gives back N/2 magnitudes. it is optmized
for speed, but still has a pretty good noise floor of around 12b, and an
SNR of around 10b.  it only operates on real data, and only returns the first
N/2 bins. this fht was based on the fft, which was limited by the 2kB SRAM in
the arduino.  this fht could go to 512 samples but a few variables would need to
be transferred from bytes to ints to make it work.

REFERENCES:

1. Stanislav's FHT implementation:

http://www.elektronika.kvalitne.cz/ATMEL/necoteorie/transformation/AVRFHT/AVRFHT.html

Stanislav has a great website with a lot of audio applications for AVR.  he introduced
me to the FHT, and got this whole mess started.  there isnt too much info on the web
about the fht, and his webiste does a good job of explaining how it works.  a bit
of background in the fft is very helpful for understanding it.

2. see the ArduinoFFT read_me for more information on FFTs and window functions.


INDEX:
-----
0. resource usage
1. included files
2. function calls
3. example
4. #define explanations


0. resource usage 

A. SPEED CHARACTERISTICS:

--------------------------------------------------------
Func: run  : reorder : window : lin : lin8 : log : oct :
--------------------------------------------------------
 N  | (ms) :   (us)  :  (us)  : (us): (us)*: (us): (us):
--------------------------------------------------------
256 : 3.18 :   263   :  576   : 566 : 441  : 508 : 425 :
128 : 1.30 :   123   :  288   : 287 : 220  : 258 : 223 :
64  : 0.51 :   61    :  144   : 148 : 111  : 130 : 119 :
32  : 0.18 :   27    :  72    : 77  : 56   : 66  : 67  :
16  : 0.12 :   13    :  36    : 44  : 28   : 39  : 39  :
--------------------------------------------------------

* Note: the lin8 values are approximate, as they vary a small amount due
to SCALE factor.  see #define section for more detials.

B. MEMORY CHARACTERISTICS

these numbers assume you are going to be using fht_run(), so the values
listed for the other functions are the memory usage in addition to that
already consumed by fht_run.  they are given for SRAM (S) and FLASH (F)
in bytes (B).

----------------------------------------------------------------
Func:   run   : reorder : window :   lin   :  lin8   :   log   :
----------------------------------------------------------------
 N  |  S/F(B) :   F(B)  :  F(B)  :  S/F(B) :  S/F(B) :  S/F(B) :
----------------------------------------------------------------
256 : 512/476 :   120   :  512   : 256/768 : 128/640 : 128/256 :
128 : 256/224 :   56    :  256   : 128/768 :  64/640 :  64/256 :
64  : 128/100 :   28    :  128   :  64/768 :  32/640 :  32/256 :
32  :  64/40  :   12    :  64    :  32/768 :  16/640 :  16/256 :
16  :  32/12  :    6    :  32    :  16/768 :   8/640 :   8/256 :
----------------------------------------------------------------


1. the following files should be included with the fht library:

FHT.h - header file with all the code
keywords.txt - color coding for keywords in sketch
fht_codec.pde - example sketch using codecshield
fht_adc.pde - example sketch using adc
arduinofft_display.pd - puredata fft data display patch for example sketches
reorder_table_creator.pde - arduino sketch for generating the reorder lookup table

lookup tables for reordering the input data
---------------
16_reorder.inc
32_reorder.inc
64_reorder.inc
128_reorder.inc
256_reorder.inc

window functions for windowing the data
------------
hann_16.inc
hann_32.inc
hann_64.inc
hann_128.inc
hann_256.inc

cos and sin tables for fht multiplication
----------------
cas_lookup_16.inc
cas_lookup_32.inc
cas_lookup_64.inc
cas_lookup_128.inc
cas_lookup_256.inc

log and sqrt tables for calculating output magnitude
----------------
sqrtlookup8.inc
sqrtlookup16.inc
decibel.inc

2. there are multiple functions you can call to operate the fht.  the
reason they are broken up, is so you can tailor the fht to your needs.
if you dont neet particular parts, you can not run them and save time.

A. fht_run() - this is the main fht function call.  takes no variables and
returns no variables.  it assumes there is a block of data already in sram,
and that it is already reordered.  the data is stored in array called:

fht_input[]

which contains 1 16b value per fht data point.  for example:

fht_input[0] = real sample 1
fht_input[1] = real sample 2

the final output is kept in fht_input[], with the 0 -> N/2 bins having the
sum of the real and imaginary components, and the N/2 -> N having the
difference between the real and imaginary components. these two parts need
to be squared and summed to get the overall magnitude at a particular
frequency.  as a result, you will have to run one of the magnitude functions
to get useful data from these bins.

B. fht_reorder() - this reorders the fht inputs to get them ready for the
special way in which the fht algorithm processes data.  unless you do this
yourself with another piece of code, you have to call this before running
fht_run().  it takes no variables and returns no variables.  this runs on
the array fht_input[], so the data must be first filled into that array
before this function is called.

C. fht_window() - this function multiplies the input data by a window function
to help increase the frequency resolution of the fht data.  this takes no
variables, and returns no variables.  this processes the data in fht_input[],
so that data must first be placed in that array before it is called.  it must
be called before fht_reorder() or fht_run().

D. fht_mag_lin8() - this gives the magnitude of each bin in from the fht.  it
sums the squares of the imaginary and real, and then takes the square root,
rounding the answer to 8b precision (it uses a lookup table, and scales the 
values to fit the full 8b range.  scale factor = 255/(181*256)).  this takes
no variables, and returns no variables. it operates on fht_input, and returns
the data in an array called fht_lin_out8[]. you can then use the data in
fht_lin_out8[]. the magnitude is only calculated for the first N/2 bins, as
the second half of an fht is identical to the first half for all real inputs.
so fht_lin_out8[] has N/2 8b values, with each index equaliing the bin order.
for example:

fht_lin_out8[0] = first bin magnitude (0hz -> Fs/N)
fht_lin_out8[1] = second bin magnitude (Fs/N -> 2Fs/N)

E. fht_mag_lin() - this gives the magnitude of each bin in the fht.  it sums
the squares of the imaginary and real, and then takes the square root.  it uses
a lookup table to calculate the square root, so it has limited precision.  you
can think of it as 8b of value times 4b of exponent.  so it covers the full 16b
range, but only has 8b of precision at any point in that range.  the data is
taken in on fht_input[] and returned on fht_lin_out[].  the values are in
sequential order, and there are only N/2 values total, as the fht of a real
signal is symetric about the center frequency.

F. fht_mag_log() - this gives the magnitude of each bin in the fht.  it sums
the squares of the imaginary and real, and then takes the square root, and then
takes the log base 2 of that value.  so the output is compressed in a logrithmic
fashion, and is essentially in decibels (times a scaling factor).  it takes no
variables, and returns no variables.  it uses a lookup table to calculate the 
log of the square root, and scales the output over the full 8b range {the
equation is 16*(log2((img^2 + real^2)^0.5))}.  it is only an 8b value, and the
values are taken from fht_input[], and returned at fht_log_out[].  the output
values are in sequential order of fht frequency bins, and there are only N/2
total bins, as the second half of the fht result is redundant for real inputs.

G. fht_mag_octave() - this outputs the RMS value of the bins in an octave
(doubling of frequencies) format. this is more useful in some ways, as it is
closer to how humans percieve sound.  it doesnt take any variables, and doesnt
return any variables.  the input is taken from fht_output[] and returned on
fht_oct_out[].  the data is represented in and 8b value of 16*log2(sqrt(mag)).
there are LOG_N bins. and they are given as follows:

FHT_N = 256 :: bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 3:64, 65:128]
FHT_N = 128 :: bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 3:64]

where (5:8) is a summation of all bins, 5 through 8.  the data for each bin is
squared, imaginary and real parts, and then added with all the squared magnitudes
for the range.  it is then divided down by the numbe of bins (which can be turned
off - see #defines below), and then the square root is taken, and then the log is
taken.

3. EXAMPLE: 256 point FHT

1. fill up fht_input[] with a sample at the even indices, and 0 at the odd
indices.  do this until youve filled up to fht_input[511].

2. call fht_window() to window the data

3. call fht_reorder() to reorder the data

4. call fht_run() to process the fht

5. call fht_mag_log() to get the magnitudes of the bins

6. take data out of fht_log_out[] and display them in sequence to make a
spectrum analyzer.  do this for fht_log_out[0] -> fht_log_out[127]. the
output data will be for 0Hz -> Fs/(2*N), with Fs/N spacing.

4. #DEFINES

these values allow you to modify the fht code to fit your needs.  for the most
part, they just turn off stuff you arent using. by default everything is on,
so its best to use them to turn off the extra resource hogs.

A. FHT_N - sets the fht size.  possible options are 16, 32, 64, 128, 256.
256 is the defualt.

B. SCALE - sets the scaling factor for fht_mag_lin8().  since 8b resolution
is pretty poor, you will want to scale the values to max out the full range.
setting SCALE multiplies the output by a constant beefore doing the square
root, so you have maximum resolution.  it does consume slightly more
resources, but is pretty minimal.  SCALE can be any number from 1 -> 255. by
default it is 1.  1, 2, 4, 128, and 256 consume the least resources.

C. WINDOW - turns on or off the window function resources.  if you are not using
fht_window(), then you should set WINDOW 0 (off). by default its 1 (on).

D. REORDER - turns on or off the reorder function resources.  if you are not using
fht_reorder(), then you should set REORDER 0 (off). by default its 1 (on).

E. LOG_OUT - turns on or off the log function resources.  if you are using
fht_mag_log(), then you should set LOG_OUT 1 (on). by default its 0 (off).

F. LIN_OUT - turns on or off the lin output function resources.  if you are using
fht_mag_lin(), then you should set LIN_OUT 1 (on). by default its 0 (off).

G. LIN_OUT8 - turns on or off the lin8 output function resources.  if you are using
fht_mag_lin8(), then you should set LIN_OUT8 1 (on). by default its 0 (off).

H. OCTAVE - this turns on or off the octave output function resources. if you are
using fht_mag_octave(), then you should set OCTAVE 1 (on).  by default it is 0 (off).

I. OCT_NORM - this turns on or off the octave normilisation feature.  this is the
part of fht_mat_octave() that divides each bin grouping by the number of bins.
since a lot of sound sources are pink noise (they drop off in amplitude as the
frequency increases), the scale tends to slope off rather quickly.  this artificially
boosts the higher frequencies when off (OCT_NORM 0).  by default, the normilisation
is on (OCT_NORM 1).

 