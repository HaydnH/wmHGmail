/*
 *
 *  	wmHGmail-1.0
 * 
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation; either version 2, or (at your option)
 * 	any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program (see the file COPYING); if not, write to the
 * 	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 *      Boston, MA  02111-1307, USA
 *
 *
 *      Changes:
 *
 *	Version 1.00  -	released May 4th, 2017
 *
 */


// Includes  
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <X11/xpm.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <dirent.h>
#include "xutils_cairo.h"
#include "goauth.h"
#include "wmHGmail_master.xpm"
#include "wmHGmail_mask.xbm"


// Delay between refreshes (in microseconds) 
#define DELAY 10000L
#define WMHGMAIL_VERSION "1.00"

// Color struct, h = hex (000000), rgb components in range 0-1.
struct patCol {
  char  h[7];
  float r,g,b;
};
typedef struct patCol PatCol;

void ParseCMDLine(int argc, char *argv[]);
void ButtonPressEvent(XButtonEvent *);
void print_usage();
void h2dCol(PatCol *col);

int        winH = 64, winW = 64;
int        GotFirstClick1, GotDoubleClick1;
int        GotFirstClick2, GotDoubleClick2;
int        GotFirstClick3, GotDoubleClick3;
int        DblClkDelay;
int        HasExecute = 0;		/* controls perf optimization */
char	   ExecuteCommand[1024];
char       TimeColor[30] = "#ffff00";
char       BackgroundColor[30] = "#181818";
char       unreadStr[10] = "?";
int        uCount;
int        uc2 = 1, uc3 = 3, uc4 = 5, uc5 = 10, bfA = 90, cfA = 90, pfA = 90;
int        aState = 1;
char	   fileConf[PATH_MAX] = "";
char       eFont[50] = "Arial";
int	   fSize = 14;	


pthread_t  tid[2], ttid[2];

// Colour pointers
PatCol  bg, tf, rf, lf, bf;
PatCol  mls, mls1, mls2, mls3, mls4, mls5;
PatCol  mle, mle1, mle2, mle3, mle4, mle5;
PatCol  mrs, mrs1, mrs2, mrs3, mrs4, mrs5;
PatCol  mre, mre1, mre2, mre3, mre4, mre5;
PatCol  cb, cb1, cb2, cb3, cb4, cb5;
PatCol  cf, cf1, cf2, cf3, cf4, cf5;


// Function to convert hex color to rgb values
void h2dCol(PatCol *col) {
  char hCol[3]; 
  strncpy(hCol, col->h, 2);
  hCol[2] = '\0';
  col->r = strtol(hCol, NULL, 16)/255.0;
  strncpy(hCol, col->h+2, 2);
  hCol[2] = '\0';
  col->g = strtol(hCol, NULL, 16)/255.0;
  strncpy(hCol, col->h+4, 2);
  hCol[2] = '\0';
  col->b = strtol(hCol, NULL, 16)/255.0;
}


// Function to get unread gmail count
long getUCount() {
  long errCode = 0;
  errCode = doCurl(3);
  if (errCode == 401) {
    errCode = doCurl(4); 

  } else if (errCode == 200) {
    if (uCount > 99) {
      strcpy(unreadStr, "+");
    } else {
      sprintf(unreadStr, "%d", uCount);
    }

  } else { 
    strcpy(unreadStr, "?");
  }
  return errCode;
}


// Draw the appIcon
void drawMail(cairo_t *ctx) {
  cairo_pattern_t *mlPat, *mrPat;
  int mailH = 32, mailW = 42, mS = 6, sfl = 14;
  int msY = (winH - mailH)/2 + 4, msX = (winW - mailW)/2+1, meY = msY + mailH, meX = msX + mailW;
  double degrees = M_PI / 180.0;

  // Check which colour to use
  if (uCount >= uc5 || aState > 0) {
    mls = mls5;
    mle = mle5;
    mrs = mre5;
    mre = mre5;
    cb = cb5;
    cf = cf5;
  } else if (uCount >= uc4) {
    mls = mls4;
    mle = mle4;
    mrs = mre4;
    mre = mre4;
    cb = cb4;
    cf = cf4;
  } else if (uCount >= uc3) {
    mls = mls3;
    mle = mle3;
    mrs = mre3;
    mre = mre3;
    cb = cb3;
    cf = cf3;
  } else if (uCount >= uc2) {
    mls = mls2;
    mle = mle2;
    mrs = mre2;
    mre = mre2;
    cb = cb2;
    cf = cf2;
  } else {
    mls = mls1;
    mle = mle1;
    mrs = mre1;
    mre = mre1;
    cb = cb1;
    cf = cf1;
  }


  // Re-enable antialiasing
  cairo_set_antialias(ctx, CAIRO_ANTIALIAS_BEST);

  // Clear Window
  cairo_set_source_rgb(ctx, bg.r, bg.g, bg.b);
  cairo_rectangle(ctx, 5, 5, 54, 54);
  cairo_fill(ctx);

  // Draw top flap
  cairo_set_source_rgb(ctx, tf.r, tf.g, tf.b);
  cairo_move_to(ctx, msX + mS, msY);
  cairo_line_to(ctx, meX - mS, msY);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl);
  cairo_line_to(ctx, msX + mS, msY);
  cairo_fill(ctx);

  // Draw right flap
  cairo_set_source_rgb(ctx, rf.r, rf.g, rf.b);
  cairo_move_to(ctx, meX - mS, msY + mS);
  cairo_line_to(ctx, meX - mS, meY);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl + mS);
  cairo_line_to(ctx, meX - mS, msY + mS);
  cairo_fill(ctx);

  // Draw bottom flap
  cairo_set_source_rgb(ctx, bf.r, bf.g, bf.b);
  cairo_move_to(ctx, meX - mS, meY);
  cairo_line_to(ctx, msX + mS, meY);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl + mS);
  cairo_line_to(ctx, meX - mS, meY);
  cairo_fill(ctx);

  // Draw left flap
  cairo_set_source_rgb(ctx, lf.r, lf.g, lf.b);
  cairo_move_to(ctx, msX + mS, meY);
  cairo_line_to(ctx, msX + mS, msY + mS);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl + mS);
  cairo_line_to(ctx, msX + mS, meY);
  cairo_fill(ctx);

  // draw line between bottom and left flap
  cairo_set_line_width(ctx, 0.5);
  cairo_set_source_rgb(ctx, rf.r, rf.g, rf.b);
  cairo_move_to(ctx, msX + mS, meY);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl + mS);
  cairo_stroke(ctx);

  // Create a gradient pattern for M left leg & shoulders
  mlPat = cairo_pattern_create_linear(meX, msY, msX, msY);
  cairo_pattern_add_color_stop_rgb(mlPat, 0, mls.r, mls.g, mls.b);
  cairo_pattern_add_color_stop_rgb(mlPat, 1, mle.r, mle.g, mle.b);
  cairo_set_source(ctx, mlPat);
 
  // Draw "M" left leg and shoulders
  cairo_new_sub_path (ctx);
  cairo_arc(ctx, msX + mS, meY - mS, mS, 90 * degrees, 180 * degrees);
  cairo_line_to(ctx, msX, msY + mS);
  cairo_arc(ctx, msX + mS, msY + mS, mS, 180 * degrees, 270 * degrees);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl);
  cairo_line_to(ctx, meX - mS, msY);
  cairo_line_to(ctx, meX - mS, msY + mS);
  cairo_line_to(ctx, msX + mailW/2, msY + sfl + mS);
  cairo_line_to(ctx, msX + mS, msY + mS);
  cairo_line_to(ctx, msX + mS, meY);
  cairo_close_path (ctx);
  cairo_fill(ctx);

  // Create a gradient pattern for M right leg
  mrPat = cairo_pattern_create_linear(msX, msY, msX, meY);
  cairo_pattern_add_color_stop_rgb(mrPat, 0, mrs.r, mrs.g, mrs.b);
  cairo_pattern_add_color_stop_rgb(mrPat, 1, mre.r, mre.g, mre.b);
  cairo_set_source(ctx, mrPat);

  // Draw "M" right leg
  cairo_new_sub_path (ctx);
  cairo_arc(ctx, meX - mS, msY + mS, mS, 270 * degrees, 0 * degrees);
  cairo_line_to(ctx, meX, meY - mS);
  cairo_arc(ctx, meX - mS, meY - mS, mS, 0 * degrees, 90 * degrees);
  cairo_line_to(ctx, meX - mS, msY);
  cairo_close_path (ctx);
  cairo_fill(ctx);

  // Free Patterns
  cairo_pattern_destroy(mlPat);
  cairo_pattern_destroy(mrPat);
}

void drawCount(cairo_t *ctx) {
  cairo_text_extents_t extents;
  int mailH = 32, mailW = 42, ccR = 10;
  int msY = (winH - mailH)/2 + 4, msX = (winW - mailW)/2+1, meX = msX + mailW, ccX = meX - 8;

  // Draw Circle
  cairo_set_line_width(ctx, 3);
  cairo_set_source_rgb(ctx, bg.r, bg.g, bg.b);
  cairo_arc(ctx, ccX, msY, ccR, 0, 2* M_PI);
  cairo_stroke_preserve(ctx);
  cairo_set_source_rgb(ctx, cb.r, cb.g, cb.b);
  cairo_fill(ctx);

  // Draw mail count text
  cairo_text_extents(ctx, unreadStr, &extents);
  cairo_set_source_rgb(ctx, cf.r, cf.g, cf.b);
  cairo_move_to(ctx, ccX - extents.width/2, msY + 5);
  cairo_show_text(ctx, unreadStr);
}

void drawLock(cairo_t *ctx) {
  int mailH = 34, mailW = 48, bodyH = 12, bodyW = 18, shackleH = 7, shackleW = 10, bcRad = 2;
  int msY = (winH - mailH)/2 + 4, msX = (winW - mailW)/2+1, meX = msX + mailW;
  int bsX = meX - 8 - (bodyW/2), bsY = msY - (bodyH + shackleH)/2 + shackleH, beX = bsX + bodyW, beY = bsY + bodyH;
  int bmX = (bodyW - shackleW)/2, sRad = shackleW/2, toeH = shackleH - (shackleW / 2);
  int ssX = bsX + bmX, ssY = bsY - shackleH, seX = beX - bmX, seY = bsY;
  double degrees = M_PI / 180.0;

  // Draw lock body
  cairo_set_line_width(ctx, 3);
  cairo_set_source_rgb(ctx, bg.r, bg.g, bg.b);
  cairo_move_to(ctx, bsX + bcRad, bsY);
  cairo_line_to(ctx, beX - bcRad, bsY);
  cairo_arc(ctx, beX - bcRad, bsY + bcRad, bcRad, 270 * degrees, 0);
  cairo_line_to(ctx, beX, beY - bcRad);
  cairo_arc(ctx, beX - bcRad, beY - bcRad, bcRad, 0, 90 * degrees);
  cairo_line_to(ctx, bsX + bcRad, beY);
  cairo_arc(ctx, bsX + bcRad, beY - bcRad, bcRad, 90 * degrees, 180 * degrees);
  cairo_line_to(ctx, bsX, bsY + bcRad);
  cairo_arc(ctx, bsX + bcRad, bsY + bcRad, bcRad, 180 * degrees, 270 * degrees);
  cairo_stroke_preserve(ctx);
  cairo_set_source_rgb(ctx, cb.r, cb.g, cb.b);
  cairo_fill(ctx);

  // Draw shackle
  cairo_set_line_width(ctx, 2);
  cairo_move_to(ctx, ssX, seY);
  cairo_line_to(ctx, ssX, seY - toeH);
  cairo_arc(ctx, ssX + sRad, ssY + sRad, sRad, 180 * degrees, 0);
  cairo_line_to(ctx, seX, seY);
  cairo_stroke(ctx);

  // Draw keyhole
  int lcX = bsX + bodyW/2;
  cairo_set_source_rgb(ctx, bg.r, bg.g, bg.b);
  cairo_arc(ctx, lcX, bsY + 4, 2, 0, 360 * degrees); 
  cairo_fill(ctx);
  cairo_move_to(ctx, lcX, bsY + 4);
  cairo_line_to(ctx, lcX, beY - 2);
  cairo_stroke(ctx);
}


// Draw logo for no network
void drawNet(cairo_t *ctx) {
  int mailH = 32, mailW = 42, ccR = 10, ccW = 5, cOff = 0;
  int msY = (winH - mailH)/2 + 4, msX = (winW - mailW)/2+1, meX = msX + mailW, ccX = meX - 8;

  // Draw Circle
  cairo_set_line_width(ctx, 3);
  cairo_set_source_rgb(ctx, bg.r, bg.g, bg.b);
  cairo_arc(ctx, ccX, msY, ccR, 0, 2* M_PI);
  cairo_stroke_preserve(ctx);
  cairo_set_source_rgb(ctx, cb.r, cb.g, cb.b);
  cairo_fill(ctx);
  cairo_set_source_rgb(ctx, tf.r, tf.g, tf.b);
  cairo_arc(ctx, ccX, msY, ccR - ccW, 0, 2* M_PI);
  cairo_stroke_preserve(ctx);
  cairo_fill(ctx);
  cairo_set_source_rgb(ctx, cb.r, cb.g, cb.b);

  cOff = ccR * cos(0.79);
  cairo_move_to(ctx, ccX + cOff, msY - cOff);
  cairo_line_to(ctx, ccX - cOff, msY + cOff);
  cairo_stroke(ctx);
}

void *checkLock(void *arg) {
  pthread_t pid = pthread_self();

 if (pthread_equal(pid, ttid[0])) {
   gAuth(fileConf);
   readConf();
    if (getUCount() == 200)
      aState = 0;
 }
 return NULL;
}


// main  
int main(int argc, char *argv[]) {
  //char *pathConf = getenv("HOME"), fileConf[PATH_MAX] = "";
  char *pathConf = getenv("HOME");
  strcat(pathConf, "/.credentials");
  strcpy(fileConf, pathConf);
  strcat(fileConf, "/wmHGmail_client_secret.json");


  XEvent  event;
  int	  n = 1000;
  // Lots of colour pointers
  PatCol  *bgptr = &bg, *tfptr = &tf, *rfptr = &rf, *bfptr = &bf, *lfptr = &lf;
  PatCol  *mlsptr = &mls, *mls1ptr = &mls1, *mls2ptr = &mls2, *mls3ptr = &mls3, *mls4ptr = &mls4;
  PatCol  *mls5ptr = &mls5, *mleptr = &mle, *mle1ptr = &mle1, *mle2ptr = &mle2, *mle3ptr = &mle3;
  PatCol  *mle4ptr = &mle4, *mle5ptr = &mle5, *mrsptr = &mrs, *mrs1ptr = &mrs1, *mrs2ptr = &mrs2;
  PatCol  *mrs3ptr = &mrs3, *mrs4ptr = &mrs4, *mrs5ptr = &mrs5, *mreptr = &mre, *mre1ptr = &mre1;
  PatCol  *mre2ptr = &mre2, *mre3ptr = &mre3, *mre4ptr = &mre4, *mre5ptr = &mre5;
  PatCol  *cbptr = &cb, *cb1ptr = &cb1, *cb2ptr = &cb2, *cb3ptr = &cb3, *cb4ptr = &cb4, *cb5ptr = &cb5;
  PatCol  *cfptr = &cf, *cf1ptr = &cf1, *cf2ptr = &cf2, *cf3ptr = &cf3, *cf4ptr = &cf4, *cf5ptr = &cf5;

  cairo_surface_t       *sfc;
  cairo_t               *ctx;
  cairo_font_options_t  *fopts;

  // Init X window & Icon
  initXwindow(argc, argv);
  openXwindow(argc, argv, wmHGmail_master, wmHGmail_mask_bits, wmHGmail_mask_width, wmHGmail_mask_height);

  // Create a Cairo surface and context in the icon window 
  sfc = cairo_create_x11_surface0(winW, winH);
  ctx = cairo_create(sfc);

  // Default Colors
  strcpy(bg.h, "d2dae4");
  strcpy(tf.h, "f7f5ed");
  strcpy(rf.h, "b7b6ad");
  strcpy(bf.h, "e7e4d7");
  strcpy(lf.h, "e7e4d7");
  strcpy(mle5.h, "d63c3c");
  strcpy(mle4.h, "d59a21");
  strcpy(mle3.h, "cfb505");
  strcpy(mle2.h, "7ea336");
  strcpy(mle1.h, "218c22");
  strcpy(mls5.h, "e99494");
  strcpy(mls4.h, "ebc984");
  strcpy(mls3.h, "dcc200");
  strcpy(mls2.h, "bcd788");
  strcpy(mls1.h, "72ce46");
  strcpy(mre5.h, "d63c3c");
  strcpy(mre4.h, "d59a21");
  strcpy(mre3.h, "cfb505");
  strcpy(mre2.h, "7ea336");
  strcpy(mre1.h, "218c22");
  strcpy(mrs5.h, "e99494");
  strcpy(mrs4.h, "ebc984");
  strcpy(mrs3.h, "dcc200");
  strcpy(mrs2.h, "bcd788");
  strcpy(mrs1.h, "72ce46");
  strcpy(cb1.h, "218c22");
  strcpy(cb2.h, "7ea336");
  strcpy(cb3.h, "cfb505");
  strcpy(cb4.h, "d59a21");
  strcpy(cb5.h, "d63c3c");
  strcpy(cf1.h, "ffffff");
  strcpy(cf2.h, "ffffff");
  strcpy(cf3.h, "ffffff");
  strcpy(cf4.h, "ffffff");
  strcpy(cf5.h, "ffffff");

  // Parse any command line arguments.
  ParseCMDLine(argc, argv);

  // Set font options (should be outside the while loop?)
  fopts = cairo_font_options_create();
  cairo_get_font_options(ctx, fopts);
  cairo_select_font_face(ctx, eFont, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(ctx, fSize);
  cairo_font_options_set_hint_style(fopts, CAIRO_HINT_STYLE_NONE);
  cairo_font_options_set_antialias(fopts, CAIRO_ANTIALIAS_SUBPIXEL);
  cairo_font_options_set_subpixel_order(fopts, CAIRO_SUBPIXEL_ORDER_RGB);
  cairo_set_font_options(ctx, fopts);
  cairo_font_options_destroy(fopts);

  // Set colors rgb elements
  h2dCol(bgptr);
  h2dCol(tfptr);
  h2dCol(rfptr);
  h2dCol(lfptr);
  h2dCol(bfptr);
  h2dCol(mlsptr);
  h2dCol(mls1ptr);
  h2dCol(mls2ptr);
  h2dCol(mls3ptr);
  h2dCol(mls4ptr);
  h2dCol(mls5ptr);
  h2dCol(mleptr);
  h2dCol(mle1ptr);
  h2dCol(mle2ptr);
  h2dCol(mle3ptr);
  h2dCol(mle4ptr);
  h2dCol(mle5ptr);
  h2dCol(mrsptr);
  h2dCol(mrs1ptr);
  h2dCol(mrs2ptr);
  h2dCol(mrs3ptr);
  h2dCol(mrs4ptr);
  h2dCol(mrs5ptr);
  h2dCol(mreptr);
  h2dCol(mre1ptr);
  h2dCol(mre2ptr);
  h2dCol(mre3ptr);
  h2dCol(mre4ptr);
  h2dCol(mre5ptr);
  h2dCol(cbptr);
  h2dCol(cb1ptr);
  h2dCol(cb2ptr);
  h2dCol(cb3ptr);
  h2dCol(cb4ptr);
  h2dCol(cb5ptr);
  h2dCol(cfptr);
  h2dCol(cf1ptr);
  h2dCol(cf2ptr);
  h2dCol(cf3ptr);
  h2dCol(cf4ptr);
  h2dCol(cf5ptr);

  checkConf(pathConf);
  if (readConf() != 0 || getUCount() == 401) {
    aState = 2;
    int terr, ttCount = 0;
    while (ttCount < 2) {
      terr = pthread_create(&(ttid[ttCount]), NULL, &checkLock, NULL);
      if (terr != 0 )
        fprintf(stderr, "Couldn't create thread: %s\n", strerror(terr));
      ttCount++;
    }
  }


  // Loop until we die
  while(1) {
    if (n > 10) {
      // Only recalculate every Nth loop
      n = 0;

      if (getUCount() == 200)
        aState = 0;

      drawMail(ctx);
      if (aState == 1) {
        drawNet(ctx);
      } else if (aState == 2) {
        drawLock(ctx);
      } else {
        drawCount(ctx);
      }
    }

    // Increment counter
    ++n;

    // Keep track of click events. If Delay too long, set GotFirstClick's to False
    if (DblClkDelay > 1500) {
      DblClkDelay = 0;
      GotFirstClick1 = 0; GotDoubleClick1 = 0;
      GotFirstClick2 = 0; GotDoubleClick2 = 0;
      GotFirstClick3 = 0; GotDoubleClick3 = 0;

    } else {
      ++DblClkDelay;
    }

    // Process any pending X events.
    while(XPending(display)){
      XNextEvent(display, &event);
      switch(event.type){
        case ButtonPress:
          ButtonPressEvent(&event.xbutton);
          break;
        case ButtonRelease:
          break;
      }
    }

    // Redraw and wait for next update 
  
    RedrawWindow();
    usleep(500000L);

  }
 cairo_destroy(ctx);
 cairo_close_x11_surface(sfc);
}


// Function to check a valid 000000 color is provided
void valid_color(char *argv, char ccol[6]) {
  if (strcmp(ccol, "missing") == 0 ||ccol[0] == '-' ) {
    fprintf(stderr, "ERROR: No color found following %s flag.\n", argv);
    print_usage();
    exit(-1);
  }

  if (strlen(ccol) != 6 || ccol[strspn(ccol, "0123456789abcdefABCDEF")] != 0) {
    fprintf(stderr, "ERROR: Invalid color following %s flag.\n", argv);
    print_usage();
    exit(-1);
  }
}


// Parse command line arguments
void ParseCMDLine(int argc, char *argv[]) {
  int  i;
  char ccol[8] = "missing";

  for (i = 1; i < argc; i++) {
    if (argc > i+1 && strlen(argv[i+1])<=7)
      strcpy(ccol, argv[i+1]);

    if (!strcmp(argv[i], "-e")){
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No command found following -e flag.\n");
        print_usage();
        exit(-1);
      }
      strcpy(ExecuteCommand, argv[++i]);
      HasExecute = 1;

    } else if (!strcmp(argv[i], "-f")) {
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No font found following -f flag.\n");
        print_usage();
        exit(-1);
      }

      if (strlen(argv[i+1]) >= 50) {
        fprintf(stderr, "ERROR: Font name following -f flag is too long (max 50 chars), please file a bug if you require longer.\n"); 
        print_usage();
        exit(-1);

      } else {
        strcpy(eFont, argv[++i]);
      }

    } else if (!strcmp(argv[i], "-fs")) {
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No font size found following -fs flag.\n");
        print_usage();
        exit(-1);
      }

      fSize = strtol(argv[++i], NULL, 10);


    } else if (!strcmp(argv[i], "-s2")) {
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No mail count provided after -s2 flag.\n");
        print_usage();
        exit(-1);
      }
      uc2 = strtol(argv[++i], NULL, 10);

    } else if (!strcmp(argv[i], "-s3")) {
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No mail count provided after -s3 flag.\n");
        print_usage();
        exit(-1);
      }
      uc3 = strtol(argv[++i], NULL, 10);

    } else if (!strcmp(argv[i], "-s4")) {
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No mail count provided after -s4 flag.\n");
        print_usage();
        exit(-1);
      }
      uc4 = strtol(argv[++i], NULL, 10);

    } else if (!strcmp(argv[i], "-s5")) {
      if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
        fprintf(stderr, "ERROR: No mail count provided after -s5 flag.\n");
        print_usage();
        exit(-1);
      }
      uc5 = strtol(argv[++i], NULL, 10);

    } else if (!strcmp(argv[i], "-bg")) {
      valid_color(argv[i], ccol);
      strcpy(bg.h, argv[++i]);

    } else if (!strcmp(argv[i], "-tf")) {
      valid_color(argv[i], ccol);
      strcpy(tf.h, argv[++i]);

    } else if (!strcmp(argv[i], "-rf")) {
      valid_color(argv[i], ccol);
      strcpy(rf.h, argv[++i]);

    } else if (!strcmp(argv[i], "-bf")) {
      valid_color(argv[i], ccol);
      strcpy(bf.h, argv[++i]);

    } else if (!strcmp(argv[i], "-lf")) {
      valid_color(argv[i], ccol);
      strcpy(lf.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mle5")) {
      valid_color(argv[i], ccol);
      strcpy(mle5.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mle4")) {
      valid_color(argv[i], ccol);
      strcpy(mle4.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mle3")) {
      valid_color(argv[i], ccol);
      strcpy(mle3.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mle2")) {
      valid_color(argv[i], ccol);
      strcpy(mle2.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mle1")) {
      valid_color(argv[i], ccol);
      strcpy(mle1.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mls5")) {
      valid_color(argv[i], ccol);
      strcpy(mls5.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mls4")) {
      valid_color(argv[i], ccol);
      strcpy(mls4.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mls3")) {
      valid_color(argv[i], ccol);
      strcpy(mls3.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mls2")) {
      valid_color(argv[i], ccol);
      strcpy(mls2.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mls1")) {
      valid_color(argv[i], ccol);
      strcpy(mls1.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mre5")) {
      valid_color(argv[i], ccol);
      strcpy(mre5.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mre4")) {
      valid_color(argv[i], ccol);
      strcpy(mre4.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mre3")) {
      valid_color(argv[i], ccol);
      strcpy(mre3.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mre2")) {
      valid_color(argv[i], ccol);
      strcpy(mre2.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mre1")) {
      valid_color(argv[i], ccol);
      strcpy(mre1.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mrs5")) {
      valid_color(argv[i], ccol);
      strcpy(mrs5.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mrs4")) {
      valid_color(argv[i], ccol);
      strcpy(mrs4.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mrs3")) {
      valid_color(argv[i], ccol);
      strcpy(mrs3.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mrs2")) {
      valid_color(argv[i], ccol);
      strcpy(mrs2.h, argv[++i]);

    } else if (!strcmp(argv[i], "-mrs1")) {
      valid_color(argv[i], ccol);
      strcpy(mrs1.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cb1")) {
      valid_color(argv[i], ccol);
      strcpy(cb1.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cb2")) {
      valid_color(argv[i], ccol);
      strcpy(cb2.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cb3")) {
      valid_color(argv[i], ccol);
      strcpy(cb3.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cb4")) {
      valid_color(argv[i], ccol);
      strcpy(cb4.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cb5")) {
      valid_color(argv[i], ccol);
      strcpy(cb5.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cf1")) {
      valid_color(argv[i], ccol);
      strcpy(cf1.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cf2")) {
      valid_color(argv[i], ccol);
      strcpy(cf2.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cf3")) {
      valid_color(argv[i], ccol);
      strcpy(cf3.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cf4")) {
      valid_color(argv[i], ccol);
      strcpy(cf4.h, argv[++i]);

    } else if (!strcmp(argv[i], "-cf5")) {
      valid_color(argv[i], ccol);
      strcpy(cf5.h, argv[++i]);

    } else {
      print_usage();
      exit(1);
    }
  }
}


// Print usage instructions
void print_usage(){
  printf("\nwmHGmail version: %s\n\n", WMHGMAIL_VERSION);
  printf("usage: wmHGmail [-e <command>] [-f <font>] [-fs <fontSize> ]\n");
  printf("  [-s2 <count>] [-s3 <count>] [-s4 <count>] [-s5 <count>]\n");
  printf("  [-bg <color>] [-tf <color>] [-rf <color>] [-bf <color>] [-lf <color>]\n");
  printf("  [-mle5 <color>] [-mle4 <color>] [-mle3 <color>] [-mle2 <color>] [-mle1 <color>]\n");
  printf("  [-mls5 <color>] [-mls4 <color>] [-mls3 <color>] [-mls2 <color>] [-mls1 <color>]\n");
  printf("  [-mre5 <color>] [-mre4 <color>] [-mre3 <color>] [-mre2 <color>] [-mre1 <color>]\n");
  printf("  [-mrs5 <color>] [-mrs4 <color>] [-mrs3 <color>] [-mrs2 <color>] [-mrs1 <color>]\n");
  printf("  [-cb1 <color>] [-cb2 <color>] [-cb3 <color>] [-cb4 <color>] [-cb5 <color>]\n");
  printf("  [-cf1 <color>] [-cf2 <color>] [-cf3 <color>] [-cf4 <color>] [-cf5 <color>]\n\n");
  printf("   -e <command>    Command to execute via double click of mouse button 1\n");
  printf("   -f              Cairo Font to use for mail count (Default: Arial)\n");
  printf("   -fs             Font size to use for mail count (Default: 14)\n");
  printf("   -s2 <count>     Number of mails at which to change mail icon color (Default: 1)\n");
  printf("   -s3 <count>     Number of mails at which to change mail icon color (Default: 3)\n");
  printf("   -s4 <count>     Number of mails at which to change mail icon color (Default: 5)\n");
  printf("   -s5 <count>     Number of mails at which to change mail icon color (Default: 10)\n");
  printf("   -bg <color>     Background color (default: d2dae4)\n");
  printf("   -tf <color>     Mail icon top flap color (default: f7f5ed)\n");
  printf("   -rf <color>     Mail icon right flap color (default: b7b6ad)\n");
  printf("   -bf <color>     Mail icon bottom flap color (default: e7e4d7)\n");
  printf("   -lf <color>     Mail icon left flap color (default: e7e4d7)\n");
  printf("   -mls1 <color>   Left leg gradient start color when unread > 0 (default: 72ce46)\n");
  printf("   -mls2 <color>   Left leg gradient start color when unread > s2 (default: bcd788)\n");
  printf("   -mls3 <color>   Left leg gradient start color when unread > s3 (default: dcc200)\n");
  printf("   -mls4 <color>   Left leg gradient start color when unread > s4 (default: ebc984)\n");
  printf("   -mls5 <color>   Left leg gradient start color when unread > s5 (default: e99494)\n");
  printf("   -mle1 <color>   Left leg gradient end color when unread > 0 (default: 218c22)\n");
  printf("   -mle2 <color>   Left leg gradient end color when unread > s2 (default: 7ea336)\n");
  printf("   -mle3 <color>   Left leg gradient end color when unread > s3 (default: cfb505)\n");
  printf("   -mle4 <color>   Left leg gradient end color when unread > s4 (default: d59a21)\n");
  printf("   -mle5 <color>   Left leg gradient end color when unread > s5 (default: d63c3c)\n");
  printf("   -mrs1 <color>   Right leg gradient start color when unread > 0 (default: 72ce46)\n");
  printf("   -mrs2 <color>   Right leg gradient start color when unread > s2 (default: bcd788)\n");
  printf("   -mrs3 <color>   Right leg gradient start color when unread > s3 (default: dcc200)\n");
  printf("   -mrs4 <color>   Right leg gradient start color when unread > s4 (default: ebc984)\n");
  printf("   -mrs5 <color>   Right leg gradient start color when unread > s5 (default: e99494)\n");
  printf("   -mre1 <color>   Right leg gradient end color when unread > 0 (default: 218c22)\n");
  printf("   -mre2 <color>   Right leg gradient end color when unread > s2 (default: 7ea336)\n");
  printf("   -mre3 <color>   Right leg gradient end color when unread > s3 (default: cfb505)\n");
  printf("   -mre4 <color>   Right leg gradient end color when unread > s4 (default: d59a21)\n");
  printf("   -mre5 <color>   Right leg gradient end color when unread > s5 (default: d63c3c)\n");
  printf("   -cb1 <color>    Count circle background color when unread > 0 (default: 218c22)\n");
  printf("   -cb2 <color>    Count circle background color when unread > s2 (default: 7ea336)\n");
  printf("   -cb3 <color>    Count circle background color when unread > s3 (default: cfb505)\n");
  printf("   -cb4 <color>    Count circle background color when unread > s4 (default: d59a21)\n");
  printf("   -cb5 <color>    Count circle background color when unread > s5 (default: d63c3c)\n");
  printf("   -cf1 <color>    Count font color when unread > 0 (default: ffffff)\n");
  printf("   -cf2 <color>    Count font color when unread > s2 (default: ffffff)\n");
  printf("   -cf3 <color>    Count font color when unread > s3 (default: ffffff)\n");
  printf("   -cf4 <color>    Count font color when unread > s4 (default: ffffff)\n");
  printf("   -cf5 <color>    Count font color when unread > s5 (default: ffffff)\n");
  printf("   -h              Show this usage message\n");
  printf("\nExample: wmHGmail -bg 0000FF\n\n");

}


 // This routine handles button presses.
void ButtonPressEvent(XButtonEvent *xev){
  char Command[512];

  if( HasExecute == 0) return; /* no command specified.  Ignore clicks. */
  DblClkDelay = 0;
  if ((xev->button == Button1) && (xev->type == ButtonPress)){
    if (GotFirstClick1) GotDoubleClick1 = 1;
    else GotFirstClick1 = 1;
  } else if ((xev->button == Button2) && (xev->type == ButtonPress)){
    if (GotFirstClick2) GotDoubleClick2 = 1;
    else GotFirstClick2 = 1;
  } else if ((xev->button == Button3) && (xev->type == ButtonPress)){
    if (GotFirstClick3) GotDoubleClick3 = 1;
    else GotFirstClick3 = 1;
  }

  // We got a double click on Mouse Button1 (i.e. the left one)
  if (GotDoubleClick1) {
    GotFirstClick1 = 0;
    GotDoubleClick1 = 0;
    sprintf(Command, "%s &", ExecuteCommand);
    system(Command);
  }

  // We got a double click on Mouse Button2 (i.e. the left one)
  if (GotDoubleClick2) {
    GotFirstClick2 = 0;
    GotDoubleClick2 = 0;
  }

  // We got a double click on Mouse Button3 (i.e. the left one)
  if (GotDoubleClick3) {
    GotFirstClick3 = 0;
    GotDoubleClick3 = 0;
  }

  return;
}

