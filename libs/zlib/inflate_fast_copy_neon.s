#; Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
#;
#; Redistribution and use in source and binary forms, with or without
#; modification, are permitted provided that the following conditions are
#; met:
#;     * Redistributions of source code must retain the above copyright
#;       notice, this list of conditions and the following disclaimer.
#;     * Redistributions in binary form must reproduce the above
#;       copyright notice, this list of conditions and the following
#;       disclaimer in the documentation and/or other materials provided
#;       with the distribution.
#;     * Neither the name of Code Aurora Forum, Inc. nor the names of its
#;       contributors may be used to endorse or promote products derived
#;       from this software without specific prior written permission.
#;
#; THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
#; WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
#; ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#; BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
#; BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#; WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
#; OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
#; IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#;============================================================================
#;  Code Section
        .code 32                                         @; Code is ARM ISA
#;============================================================================

        .global     inflate_fast_copy_neon


#;============================================================================
#;       INPUTS:    r0       len:     number of bytes to transfer
#;                  r1       **out:   pointer to pointer to ``out'' buffer
#;                  r2       *from:   pointer to ``from'' buffer
#;       OUTPUTS:   r1       **out:   pointer to pointer to ``out'' buffer
#;============================================================================
.balign 32
.type inflate_fast_copy_neon, %function
inflate_fast_copy_neon:
       push       {r4-r11}             @; push r4-r11 onto stack

       cmp        r0,#16               @;
       bge        inflate_fast_copy_vectorized

       #;; transfer bytes one by one
       #;; only if len < 16 bytes
inflate_fast_copy_default:

       cmp        r0,#0
       beq        inflate_fast_copy_exit

       ldr        r3,[r1,#0]           @; r3 = pointer to out

inflate_fast_copy_default_loop:

       ldrb       r12,[r2,#1]!         @; r12 = *(++from)
       subs       r0,r0,#1             @; len--
       strb       r12,[r3,#1]!         @; *(++out) = r12

       bne        inflate_fast_copy_default_loop

       str        r3,[r1,#0]           @; r1 = updated pointer to pointer
                                       @;      to out
       b          inflate_fast_copy_exit

       #;; vectorized copy routines
       #;; only if len > 16 bytes
inflate_fast_copy_vectorized:

      ldr        r3,[r1,#0]            @; r3 = pointer to out
                                       @; DON'T TOUCH r1 UNTIL FINAL
                                       @;  UPDATE OF r1 WITH ADDRESS OF r3
      cmp        r3,r2                 @
      sublo      r4,r2,r3              @
      subhs      r4,r3,r2              @;r4 = gap = |out-from|

      cmp        r4,#0
      beq        inflate_fast_copy_exit

      cmp        r4,#1
      beq        inflate_fast_copy_gap1b_proc

      cmp        r4,#2
      beq        inflate_fast_copy_gap2b_proc

      cmp        r4,#3
      beq        inflate_fast_copy_gap3b_proc

      cmp        r4,#4
      beq        inflate_fast_copy_gap4b_proc

      cmp        r4,#8
      blo        inflate_fast_copy_gap5to7b_proc
      beq        inflate_fast_copy_gap8b_proc

      cmp        r4,#16
      blo        inflate_fast_copy_gap9to15b_proc
      bhs        inflate_fast_copy_gap16b_proc


      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is 1 byte
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap1b_proc:

      add        r3,r3,#1                  @; out++
                                           @
      ldrb       r12,[r2,#1]!              @; r12 = *(++from)
      vdup.8     q0, r12                   @; duplicate r12 16 times in q0
                                           @
      lsrs       r4,r0,#4                  @; r4 = floor(len/16)
                                           @;    = iteration count for loop16
      beq        inflate_fast_copy_gap1b_proc_16bytes_loop_done

inflate_fast_copy_gap1b_proc_16bytes_loop:

      vst1.8     {q0},[r3]!                @; store 16 bytes in out and
                                           @;  increment out pointer
      sub        r0,r0,#16                 @; subtract 16 from len
      subs       r4,r4,#1                  @; decrement iteration count
      bne        inflate_fast_copy_gap1b_proc_16bytes_loop

inflate_fast_copy_gap1b_proc_16bytes_loop_done:

      cmp        r0,#0
      subeq      r3,r3,#1                  @; out--
      streq      r3,[r1,#0]                @; r1 = updated pointer to pointer
                                           @;      to out
      beq        inflate_fast_copy_exit

inflate_fast_copy_gap1b_proc_lastfewbytes_loop:

      strb       r12,[r3],#1               @; *out = r12, out++
      subs       r0,r0,#1                  @; len--
      bne        inflate_fast_copy_gap1b_proc_lastfewbytes_loop

      sub        r3,r3,#1                  @; out--
      str        r3,[r1,#0]                @; r1 = updated pointer to pointer
                                           @;      to out
      b          inflate_fast_copy_exit

      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is 2 bytes
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap2b_proc:

      add        r2,r2,#1                @; from++
      add        r3,r3,#1                @; out++
                                         @
      vld1.16    {d0[0]},[r2]            @; load 2 bytes into d0[0]
      vdup.16    q0,d0[0]                @; duplicate those 2 bytes 8 times
                                         @;  to fill up q0
                                         @
      lsrs       r4,r0,#4                @; r4 = floor(len/16)
                                         @;    = iteration count for loop16
      beq        inflate_fast_copy_gap2b_proc_16bytes_loop_done

inflate_fast_copy_gap2b_proc_16bytes_loop:

      vst1.8     {q0},[r3]!              @; store 16 bytes in out and
                                         @;  increment out pointer
      sub        r0,r0,#16               @; subtract 16 from len
      subs       r4,r4,#1                @; decrement iteration count
      bne        inflate_fast_copy_gap2b_proc_16bytes_loop

inflate_fast_copy_gap2b_proc_16bytes_loop_done:

      cmp        r0,#0
      subeq      r3,r3,#1                @; out--
      streq      r3,[r1,#0]              @; r1 = updated pointer to pointer
                                         @;      to out
      beq        inflate_fast_copy_exit

inflate_fast_copy_gap2b_proc_lastfewbytes_loop:

      ldrb       r12,[r2],#1             @; r12 = *from, from++
      subs       r0,r0,#1                @; len--
      strb       r12,[r3],#1             @; *out = r12, out++
                                         @
      bne        inflate_fast_copy_gap2b_proc_lastfewbytes_loop

      sub        r3,r3,#1                @; out--
      str        r3,[r1,#0]              @; r1 = updated pointer to pointer
                                         @;      to out
      b          inflate_fast_copy_exit

      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is 3 bytes
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;;  r4 = 3
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap3b_proc:

      add        r2,r2,#1                @; from++
      add        r3,r3,#1                @; out++
                                         @
      vld1.32    {d0[0]},[r2]            @; load 4 bytes into d0[0]

inflate_fast_copy_gap3b_proc_3bytes_loop:

      cmp        r0,#3                   @; exit loop if len < 3
      blt        inflate_fast_copy_gap3b_proc_3bytes_loop_done

      vst1.32    {d0[0]},[r3],r4         @; store 4 bytes in out
                                         @; out+=3

      sub        r0,r0,#3                @; len-=3
      b          inflate_fast_copy_gap3b_proc_3bytes_loop

inflate_fast_copy_gap3b_proc_3bytes_loop_done:

      cmp        r0,#0
      subeq      r3,r3,#1                @; out--
      streq      r3,[r1,#0]              @; r1 = updated pointer to pointer
                                         @;      to out
      beq        inflate_fast_copy_exit

inflate_fast_copy_gap3b_proc_lastfewbytes_loop:

      ldrb       r12,[r2],#1             @; r12 = *from, from++
      subs       r0,r0,#1                @; len--
      strb       r12,[r3],#1             @; *out = r12, out++

      bne        inflate_fast_copy_gap3b_proc_lastfewbytes_loop

      sub        r3,r3,#1                @; out--
      str        r3,[r1,#0]              @; r1 = updated pointer to pointer
                                         @;      to out
      b          inflate_fast_copy_exit

      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is 4 bytes
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap4b_proc:

      add        r2,r2,#1               @; from++
      add        r3,r3,#1               @; out++
                                        @
      vld1.32    {d0[0]},[r2]           @; load 4 bytes into d0[0]
      vdup.32    q0,d0[0]               @; duplicate those 4 bytes 4 times
                                        @;  to fill up q0
                                        @
      lsrs       r4,r0,#4               @; r4 = floor(len/16)
                                        @;    = iteration count for loop16
      beq        inflate_fast_copy_gap4b_proc_16bytes_loop_done

inflate_fast_copy_gap4b_proc_16bytes_loop:

      vst1.32    {q0},[r3]!             @; store 16 bytes in out and
                                        @;  increment out pointer
      sub        r0,r0,#16              @; subtract 16 from len
      subs       r4,r4,#1               @; decrement iteration count
      bne        inflate_fast_copy_gap4b_proc_16bytes_loop

inflate_fast_copy_gap4b_proc_16bytes_loop_done:

      cmp        r0,#0
      subeq      r3,r3,#1               @; out--
      streq      r3,[r1,#0]             @; r1 = updated pointer to pointer
                                        @;      to out
      beq        inflate_fast_copy_exit

inflate_fast_copy_gap4b_proc_lastfewbytes_loop:

      ldrb       r12,[r2],#1            @; r12 = *from, from++
      subs       r0,r0,#1               @; len--
      strb       r12,[r3],#1            @; *out = r12, out++

      bne        inflate_fast_copy_gap4b_proc_lastfewbytes_loop

      sub        r3,r3,#1               @; out--
      str        r3,[r1,#0]             @; r1 = updated pointer to pointer
                                        @;      to out
      b          inflate_fast_copy_exit

      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is {5-7} bytes
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;;  r4 = {5-7}
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap5to7b_proc:

      add        r2,r2,#1                @; from++
      add        r3,r3,#1                @; out++
                                         @
      vld1.8     {d0},[r2]               @; load 8 bytes into d0

inflate_fast_copy_gap5to7b_proc_5to7bytes_loop:

      cmp        r0,r4                   @; exit loop if len < {5-7}
      blt        inflate_fast_copy_gap5to7b_proc_5to7bytes_loop_done

      vst1.8     {d0},[r3],r4            @; store 8 bytes in out
                                         @; out+={5-7}

      sub        r0,r0,r4                @; len-={5-7}
      b          inflate_fast_copy_gap5to7b_proc_5to7bytes_loop

inflate_fast_copy_gap5to7b_proc_5to7bytes_loop_done:

      cmp        r0,#0
      subeq      r3,r3,#1                @; out--
      streq      r3,[r1,#0]              @; r1 = updated pointer to pointer
                                         @;      to out
      beq        inflate_fast_copy_exit

inflate_fast_copy_gap5to7b_proc_lastfewbytes_loop:

      ldrb       r12,[r2],#1             @; r12 = *from, from++
      subs       r0,r0,#1                @; len--
      strb       r12,[r3],#1             @; *out = r12, out++

      bne        inflate_fast_copy_gap5to7b_proc_lastfewbytes_loop

      sub        r3,r3,#1                @; out--
      str        r3,[r1,#0]              @; r1 = updated pointer to pointer
                                         @;      to out
      b          inflate_fast_copy_exit

      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is 8 bytes
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap8b_proc:

      add        r2,r2,#1              @; from++
      add        r3,r3,#1              @; out++
                                       @
      vld1.8     {d0},[r2]             @; load 8 bytes into d0
      vmov       d1,d0                 @; duplicate the 8 bytes to fill up
                                       @;  q0
                                       @
      lsrs       r4,r0,#4              @; r4 = floor(len/16)
                                       @;    = iteration count for loop16
      beq        inflate_fast_copy_gap8b_proc_16bytes_loop_done

inflate_fast_copy_gap8b_proc_16bytes_loop:

      vst1.8     {q0},[r3]!           @; store 16 bytes in out and
                                      @;  increment out pointer
      sub        r0,r0,#16            @; subtract 16 from len
      subs       r4,r4,#1             @; decrement iteration count
      bne        inflate_fast_copy_gap8b_proc_16bytes_loop

inflate_fast_copy_gap8b_proc_16bytes_loop_done:

      cmp        r0,#0
      subeq      r3,r3,#1             @; out--
      streq      r3,[r1,#0]           @; r1 = updated pointer to pointer
                                      @;      to out
      beq        inflate_fast_copy_exit

inflate_fast_copy_gap8b_proc_lastfewbytes_loop:

      ldrb       r12,[r2],#1          @; r12 = *from, from++
      subs       r0,r0,#1             @; len--
      strb       r12,[r3],#1          @; *out = r12, out++

      bne        inflate_fast_copy_gap8b_proc_lastfewbytes_loop

      sub        r3,r3,#1             @; out--
      str        r3,[r1,#0]           @; r1 = updated pointer to pointer
                                      @;      to out
      b          inflate_fast_copy_exit

      #;; ------------------------------------------------------------------
      #;; vectorized copy routine when gap between ``from'' and ``out''
      #;;  buffers is {9-15} bytes
      #;; INPUTS:
      #;;  r0 = len
      #;;  r2 = pointer to from
      #;;  r3 = pointer to out
      #;;  r4 = {9-15}
      #;; OUTPUTS:
      #;;  r1 = pointer to pointer to out
      #;; ------------------------------------------------------------------
inflate_fast_copy_gap9to15b_proc:

      add        r2,r2,#1            @; from++
      add        r3,r3,#1            @; out++
                                     @
      vld1.8     {q0},[r2]           @; load 16 bytes into q0

inflate_fast_copy_gap9to15b_proc_9to15bytes_loop:

      cmp        r0, r4              @; exit loop if len < {9-15}
      blt        inflate_fast_copy_gap9to15b_proc_9to15bytes_loop_done

      vst1.8     {q0},[r3],r4        @; store 16 bytes in out
                                     @; out+={9-15}

      sub        r0,r0,r4            @; len-={9-15}
      b          inflate_fast_copy_gap9to15b_proc_9to15bytes_loop

inflate_fast_copy_gap9to15b_proc_9to15bytes_loop_done:

     cmp        r0,#0
     subeq      r3,r3,#1             @; out--
     streq      r3,[r1,#0]           @; r1 = updated pointer to pointer
                                     @;      to out
     beq        inflate_fast_copy_exit

inflate_fast_copy_gap9to15b_proc_lastfewbytes_loop:

     ldrb       r12,[r2],#1          @; r12 = *from, from++
     subs       r0,r0,#1             @; len--
     strb       r12,[r3],#1          @; *out = r12, out++

     bne        inflate_fast_copy_gap9to15b_proc_lastfewbytes_loop

     sub        r3,r3,#1             @; out--
     str        r3,[r1,#0]           @; r1 = updated pointer to pointer
                                     @;      to out
     b          inflate_fast_copy_exit

     #;; ------------------------------------------------------------------
     #;; vectorized copy routine when gap between ``from'' and ``out''
     #;;  buffers is 16 bytes or more
     #;; INPUTS:
     #;;  r0 = len
     #;;  r2 = pointer to from
     #;;  r3 = pointer to out
     #;; OUTPUTS:
     #;;  r1 = pointer to pointer to out
     #;; ------------------------------------------------------------------
inflate_fast_copy_gap16b_proc:

     add        r2,r2,#1             @; from++
     add        r3,r3,#1             @; out++
                                     @
     lsrs       r4,r0,#4             @; r4 = floor(len/16)
                                     @;    = iteration count for loop16
     beq        inflate_fast_copy_gap16b_proc_16bytes_loop_done

inflate_fast_copy_gap16b_proc_16bytes_loop:

     vld1.8     {q0},[r2]!           @; load 16 bytes into q0 and
                                     @;  increment from pointer
     vst1.8     {q0},[r3]!           @; store 16 bytes in out and
                                     @;  increment out pointer
     sub        r0,r0,#16            @; subtract 16 from len
     subs       r4,r4,#1             @; decrement iteration count
     bne        inflate_fast_copy_gap16b_proc_16bytes_loop

inflate_fast_copy_gap16b_proc_16bytes_loop_done:

     cmp        r0,#0
     subeq      r3,r3,#1             @; out--
     streq      r3,[r1,#0]           @; r1 = updated pointer to pointer
                                     @;      to out
     beq        inflate_fast_copy_exit

inflate_fast_copy_gap16b_proc_lastfewbytes_loop:

     ldrb       r12,[r2],#1          @; r12 = *from, from++
     subs       r0,r0,#1             @; len--
     strb       r12,[r3],#1          @; *out = r12, out++

     bne        inflate_fast_copy_gap16b_proc_lastfewbytes_loop

     sub        r3,r3,#1             @; out--
     str        r3,[r1,#0]           @; r1 = updated pointer to pointer
                                     @;      to out

inflate_fast_copy_exit:

      pop        {r4-r11}            @; pop r4-r11 from stack
      bx         lr                  @; return

.size inflate_fast_copy_neon,  .-inflate_fast_copy_neon


        .END
