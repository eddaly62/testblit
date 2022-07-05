// testblit.c

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#include "textproc.h"

#define MAX_5X7_CHARS 10
#define MAX_IN  20
#define SEXIT   "q"

#define WIN_WIDTH   512
#define WIN_HEIGHT  256
#define HOME_X      0
#define HOME_Y      0

// Normal
#define NX  20
#define NY  20
#define YSPACE  40

// Inverted
#define IX  NX
#define IY  NY+YSPACE

// Strike thru
#define SX  NX
#define SY  IY+YSPACE

// Under-line
#define UX  NX
#define UY  SY+YSPACE

// Blinking
#define BX  NX
#define BY  UY+YSPACE

#define FONT5X7_UNDERLINE_OFFSET    9
#define FONT5X7_STRIKETHRU_OFFSET   4

#define FONT5X7R_UNDERLINE_OFFSET   0
#define FONT5X7R_STRIKETHRU_OFFSET  0

#define FONT7X9_UNDERLINE_OFFSET    0
#define FONT7X9_STRIKETHRU_OFFSET   0

#define FONT7X9R_UNDERLINE_OFFSET   0
#define FONT7X9R_STRIKETHRU_OFFSET  0

#define FONT10X14_UNDERLINE_OFFSET  0
#define FONT10X14_STRIKETHRU_OFFSET 0

#define FONT10X14R_UNDERLINE_OFFSET 0
#define FONT10X14R_STRIKETHRU_OFFSET    0


extern char *font_design_5_7;
extern char *font_design_7_9_rulings;
extern char *font_design_5_7_rulings;
extern char *font_design_7_9;
extern char *font_design_10_14;
extern char *font_design_10_14_rulings;

struct FONT_LUT font5x7lut;
struct FONT_LUT font5x7rlut; 
struct FONT_LUT font7x9lut; 
struct FONT_LUT font7x9rlut; 
struct FONT_LUT font10x14lut; 
struct FONT_LUT font10x14rlut; 

struct FONT_LUT_REC fontrec;
struct FONT_REC fresult;
struct FONT_CHAR_PARAM fcparam;
struct WINDOW *win;


int main()  {
    
    int i;
    int r;
    int toggle=1;
    bool running = true;
    char in[MAX_IN];

    ALLEGRO_DISPLAY *display;
    ALLEGRO_DISPLAY *display1;

    ALLEGRO_FONT *bmpfnt;
    ALLEGRO_BITMAP *bmp;


    al_init();
    al_init_font_addon();
    al_init_primitives_addon();
    al_init_image_addon();

    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_append_path_component(path, RESOURCES_DIR);
    al_change_directory(al_path_cstr(path, '/'));
    al_destroy_path(path);

    // build font look-up tables
    r = build_font_lut(&font5x7lut, font_design_5_7, strlen(font_design_5_7),
                        FONT5X7_STRIKETHRU_OFFSET, FONT5X7_UNDERLINE_OFFSET);
    if (r == -1) {
        printf("malformed font index table\n");
    }
    r = build_font_lut(&font5x7rlut, font_design_5_7_rulings, strlen(font_design_5_7_rulings),
                        FONT5X7R_STRIKETHRU_OFFSET, FONT5X7R_UNDERLINE_OFFSET);
    if (r == -1) {
        printf("malformed font index table\n");
    }
    r = build_font_lut(&font7x9lut, font_design_7_9, strlen(font_design_7_9),
                        FONT7X9_STRIKETHRU_OFFSET, FONT7X9_UNDERLINE_OFFSET);
    if (r == -1) {
        printf("malformed font index table\n");
    }    
    r = build_font_lut(&font7x9rlut, font_design_7_9_rulings, strlen(font_design_7_9_rulings),
                        FONT7X9R_STRIKETHRU_OFFSET, FONT7X9R_UNDERLINE_OFFSET);
    if (r == -1) {
        printf("malformed font index table\n");
    }
    r = build_font_lut(&font10x14lut, font_design_10_14, strlen(font_design_10_14),
                        FONT10X14_STRIKETHRU_OFFSET, FONT10X14_UNDERLINE_OFFSET);
    if (r == -1) {
        printf("malformed font index table\n");
    }
    r = build_font_lut(&font10x14rlut, font_design_10_14_rulings, strlen(font_design_10_14_rulings),
                        FONT10X14R_STRIKETHRU_OFFSET, FONT10X14R_UNDERLINE_OFFSET);
    if (r == -1) {
        printf("malformed font index table\n");
    }


    //display = al_create_display(512,256); // todo remove

    win = create_window(display1, WIN_WIDTH, WIN_HEIGHT);
    r = set_window_defaults(win);
    if (r == -1) {
        fprintf(stderr, "could not set window defaults\n");
    }
    r = set_window_colors(win, BLACK, WHITE);
    if (r == -1) {
        fprintf(stderr, "could not set window color\n");
    }
    r = set_window_cursor_pos(win, HOME_X, HOME_Y);
    if (r == -1) {
        fprintf(stderr, "could not set cursor position\n");
    }
    r = set_window_font(win, &font5x7lut);
    if (r == -1) {
        fprintf(stderr, "could not set font\n");
    }


    while (running) {

        #if 0
        al_clear_to_color(RED);
        
        bmp = al_create_bitmap(6*2, 10*2);


        r = get_font_record('3', &font5x7lut, &fresult);
        if (r == -1) {
            printf("char not found in font index array\n");
        }

        r = set_font_color(&fcparam, BLACK, WHITE);
        if (r == -1) {
            printf("could not set font parmaters\n");
        }   
        r = set_font_scale(&fcparam, 2);
        if (r == -1) {
            printf("could not set font parmaters\n");
        }   
        r = set_font_style(&fcparam, (INVERT | UNDER_SCORE | STRIKE_THRU | BLINK));
        if (r == -1) {
            printf("could not set font parmaters\n");
        }   

        r = make_character(&fresult, &fcparam, bmp);
        if (r == -1) {
            printf("could not make character\n");
        }

        #endif

        r = dprint(win, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    UNDER_SCORE | STRIKE_THRU | INVERT | BLINK);
        if (r == -1) {
            fprintf(stderr, "error with dprint\n");
        }   

            
        while (1) {
            r = window_update(win);
            if (r == -1) {
                fprintf(stderr, "error with window_update\n");
            } 

            usleep(250*1000);
        }
        //al_set_target_backbuffer(display);

        //al_draw_bitmap(bmp, NX, NY, 0);


        //al_flip_display();
        
        #if 0
        // 2 Hz rate
        usleep(250*1000);
        #endif

        if(fgets(in, sizeof(in), stdin) > 0){
            if (strcmp(SEXIT, in) == 0) {
                // quit
                //al_destroy_font(bmpfnt);
                //al_destroy_bitmap(bmp);
                //al_destroy_display(display);
                destroy_window(win);

                return 0;
            }
        }

    }
}

