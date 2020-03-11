// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <signal.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"
#include "z_zone.h"
#include "w_wad.h"

//#define SIM_BUILD 1
#define COLOR 1
#if COLOR == 1
#define COLOR_SIZE sizeof(unsigned int)
#else 
#define COLOR_SIZE sizeof(unsigned char)
#endif

#define REAL_SCREENWIDTH 640
#define REAL_SCREENHEIGHT 480

#define BIT_SET(IN, NUM) ( ((IN) & (1<< NUM)) )

#define VEARS_BASEADDR ((uint32_t) 0x43c00000)
#define BTNS_BASEADDR  ((uint32_t) 0x40000000)
#define SWTS_BASEADDR  ((uint32_t) 0x40010000)

union COLORTYPE {
  byte b[4];
  unsigned int i;
};

//
// Global implementation specific variables
//
byte *i_screen;
unsigned int *i_palette;
byte i_upscale = 1;

const char btn_to_key[4] = { KEY_RIGHTARROW, KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW };
const char swt_to_key[4] = { KEY_ENTER, KEY_ESCAPE, ' ', 0 };


// 
// Helpers
//
static inline void as_reg_write (uint32_t *addr, uint32_t val) { *addr = val; }
static inline uint32_t as_reg_read (volatile uint32_t *addr) { return *addr; }

#define VEARS_WORD_BYTES 4

/* Internal register definitions */
#define VEARS_REG_CONTROL      0 * VEARS_WORD_BYTES
#define VEARS_REG_STATUS       1 * VEARS_WORD_BYTES
#define VEARS_REG_IMAGE_BASE   2 * VEARS_WORD_BYTES
#define VEARS_REG_OVERLAY_BASE 3 * VEARS_WORD_BYTES
#define VEARS_REG_COLOR_1      4 * VEARS_WORD_BYTES
#define VEARS_REG_COLOR_2      5 * VEARS_WORD_BYTES
#define VEARS_REG_COLOR_3      6 * VEARS_WORD_BYTES

/* Bit offsets */
#define VEARS_CONTROL_REG_RESET_BIT           0
#define VEARS_CONTROL_REG_ENABLE_BIT          1
#define VEARS_CONTROL_REG_OVERLAY_ENABLE_BIT  2
#define VEARS_CONTROL_REG_INTR_FRAME_EN_BIT   6
#define VEARS_CONTROL_REG_INTR_LINE_EN_BIT    7

/* Bit masks */
#define VEARS_CONTROL_REG_RESET_MASK          (1<<VEARS_CONTROL_REG_RESET_BIT)
#define VEARS_CONTROL_REG_ENABLE_MASK         (1<<VEARS_CONTROL_REG_ENABLE_BIT)
#define VEARS_CONTROL_REG_OVERLAY_ENABLE_MASK (1<<VEARS_CONTROL_REG_OVERLAY_ENABLE_BIT)
#define VEARS_CONTROL_REG_INTR_FRAME_EN_MASK  (1<<VEARS_CONTROL_REG_INTR_FRAME_EN_BIT)
#define VEARS_CONTROL_REG_INTR_LINE_EN_MASK   (1<<VEARS_CONTROL_REG_INTR_LINE_EN_BIT)

int8_t I_VearsInit(uint32_t vears_iobase, uint8_t *image_base) {
  // Stored control register contents...
  static uint32_t ctrlReg = 0;

  // vears_reset (vears_iobase);
  as_reg_write ((uint32_t*)(vears_iobase + VEARS_REG_CONTROL), 0);
  as_reg_write ((uint32_t*)(vears_iobase + VEARS_REG_CONTROL), VEARS_CONTROL_REG_RESET_MASK);
  as_reg_write ((uint32_t*)(vears_iobase + VEARS_REG_CONTROL), 0);
  ctrlReg = 0;

  if (!image_base) {
    return -1;
  } else {
    // vears_image_show(vears_iobase, image_base);
    as_reg_write((uint32_t*)(vears_iobase + VEARS_REG_IMAGE_BASE), (uint32_t) image_base); 
    // vears_enable (vears_iobase);
    ctrlReg |= VEARS_CONTROL_REG_ENABLE_MASK; 
    as_reg_write ((uint32_t*)(vears_iobase + VEARS_REG_CONTROL), ctrlReg);
  }

  return 0;
}


//
// I_ShutdownGraphics
//
void I_ShutdownGraphics(void)
{
  // Do nothing
}


//
// I_StartFrame
//
void I_StartFrame (void)
{
  // Called by the core before a visual frame is rendered -> Do nothing
}


//
// I_StartTic
//
void I_StartTic (void)
{
  static unsigned int old_btn = 0;
  static unsigned int old_swt = 0;
  unsigned int btn, swt;
  static event_t btn_event = {0, 0, 0, 0};
  static event_t swt_event = {0, 0, 0, 0};
  static event_t mouse_event = {0, 0, 0, 0};
  
  btn = as_reg_read((uint32_t*)BTNS_BASEADDR);
  if (btn != old_btn) {
    //printf("I_StartTic: btn: 0x%04x != 0x%04x\n", btn, old_btn);
    for (int i = 0; i < 4; i++) {
      if (BIT_SET(btn ^ old_btn, i)) {
        btn_event.type = BIT_SET(btn, i) ? ev_keydown : ev_keyup;
        btn_event.data1 = btn_to_key[i];
        //printf("I_StartTic: Event key %d, type: %d, data: 0x%x\n", i, event.type, event.data1);
        D_PostEvent(&btn_event);
        break;
      }
    }
    old_btn = btn;  
  }
  
  swt = as_reg_read((uint32_t*)SWTS_BASEADDR);
  if (swt != old_swt) {
    if(BIT_SET(swt, 1) && BIT_SET(swt,2)) {
        // Space and Enter trigger change of upscale
        i_upscale = i_upscale ? 0 : 1;
        memset(i_screen, 0, REAL_SCREENWIDTH*REAL_SCREENHEIGHT*COLOR_SIZE);
    }
    //printf("I_StartTic: swt: 0x%04x != 0x%04x\n", swt, old_swt);
    for (int i = 0; i < 3; i++) {
      if (BIT_SET(swt ^ old_swt, i)) {
        swt_event.type = BIT_SET(swt, i) ? ev_keydown : ev_keyup;
        swt_event.data1 = swt_to_key[i];
        //printf("I_StartTic: Event key %d, type: %d, data: 0x%x\n", i, event.type, event.data1);
        D_PostEvent(&swt_event);
        break;
      }
    }
    // Special: mouse event
    if (BIT_SET(swt ^ old_swt, 3)) {
        mouse_event.type = ev_mouse;
        mouse_event.data1 = BIT_SET(swt, 3) ? 1 : 0; // Left mouse button on/off
        //printf("I_StartTic: Mouse event key %d, type: %d, data: 0x%x\n", 3, mouse_event.type, mouse_event.data1);
        D_PostEvent(&mouse_event);
    }
    
    old_swt = swt;  
  }
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
    // Send updated area of the screen to the VGA hardware -> Do nothing
}


//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
#ifndef SIM_BUILD
#if COLOR == 1
  unsigned char *src = screens[0];
  unsigned int val;

  if(i_upscale == 1) {
    unsigned int *dst_x = (unsigned int*) i_screen;
    unsigned int *dst_y = dst_x + SCREENWIDTH;

    // Upscale from 320x200 to 640x400 in color
    for (int j = 0; j < SCREENHEIGHT; j++) {
      for (int i = 0; i < SCREENWIDTH; i++) {
        val = i_palette[*src++];
        *dst_x = val;
        *dst_y = val;
        *(dst_x+1) = val;
        *(dst_y+1) = val;
        dst_x+=2;
        dst_y+=2;
      }
      /* jump dst_x and dst_y to next line */
      dst_x += REAL_SCREENWIDTH;
      dst_y = dst_x + REAL_SCREENWIDTH;
    }
  } else {
    unsigned int *dst = (unsigned int*)(i_screen) + SCREENWIDTH/2 + REAL_SCREENWIDTH*(REAL_SCREENHEIGHT-SCREENHEIGHT)/2;
   
    // No upscale from 320x200 in color
    for (int j = 0; j < SCREENHEIGHT; j++) {
      for (int i = 0; i < SCREENWIDTH; i++) {
        *dst = i_palette[*src++];
        dst++;
      }
      /* jump dst to next line (only 320 to get to next line)*/
      dst += SCREENWIDTH;
    }
  }

#else // COLOR == 0
  unsigned char *src = screens[0];
  unsigned short val;
  unsigned short *dst_x = (unsigned short*) i_screen;
  unsigned short *dst_y = dst_x + SCREENWIDTH;
  
  // Upscale from 320x200 to 640x400 for black and white
  for (int j = 0; j < SCREENHEIGHT; j++) {
    for (int i = 0; i < SCREENWIDTH; i++) {
      val = *src++;
      val = (val << 8) + val;
      *dst_x = val;
      *dst_y = val;
      dst_x++;
      dst_y++;
    }
    /* jump dst_x and dst_y to next line (only need half of the REAL_ width/height)*/
    dst_x += SCREENWIDTH;
    dst_y = dst_x + SCREENWIDTH;
  }
#endif // #if COLOR == 1
#endif // #ifndef SIM_BUILD
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* dst)
{
  memcpy (dst, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
  union COLORTYPE val; 
  
  //printf("I_SetPalette: %p\n", palette);
  val.b[3] = 0;
  for (int i=0; i < 256; i++) {
    val.b[0] = gammatable[usegamma][*palette++]; 
    val.b[1] = gammatable[usegamma][*palette++]; 
    val.b[2] = gammatable[usegamma][*palette++]; 
    i_palette[i] = val.i;
  }
}


//
// I_InitGraphics
//
void I_InitGraphics(void)
{
  printf("I_InitGraphics: Init implementation graphics\n");
  
  // Allocate memory for vears video buffer and color palette 
  i_screen = (byte*) I_AllocLow(REAL_SCREENWIDTH*REAL_SCREENHEIGHT*COLOR_SIZE);
  i_palette = (unsigned int*) I_AllocLow(256*sizeof(int));
  
  // Set color palette
  //printf("PLAYPAL size: %d\n", W_LumpLength(W_GetNumForName("PLAYPAL")));
  byte* doompal = W_CacheLumpName("PLAYPAL", PU_CACHE);
  I_SetPalette(doompal);
  
  // Init vears
#ifndef SIM_BUILD
  I_VearsInit(VEARS_BASEADDR, i_screen);  
#endif
}
