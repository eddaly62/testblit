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


extern char *font_design_5_7;
extern char *font_design_7_9_rulings;
extern char *font_design_5_7_rulings;
extern char *font_design_7_9;
extern char *font_design_10_14;
extern char *font_design_10_14_rulings;

struct FONT_INDEX font5x7index; 
struct FONT_REC_INDEX fontrec;
struct FONT_GET_RESULT fresult;
struct FONT_CHAR_PARAM fcparam;


int main()  {
    
    int i;
    int r;
    int toggle=1;
    bool running = true;
    char in[MAX_IN];

    ALLEGRO_DISPLAY *display;
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

    display = al_create_display(512,256);
    

    while (running) {

        al_clear_to_color(RED);
        
        bmp = al_create_bitmap(6*2, 10*2);

        r = build_font_index(&font5x7index, font_design_5_7, strlen(font_design_5_7));
        if (r == -1) {
            printf("malformed font index table\n");
        }

        r = get_font_record('3', &font5x7index, &fresult);
        if (r == -1) {
            printf("char not found in font index array\n");
        }

        r = set_font_parms(&fcparam, 2, BLACK, WHITE);
        if (r == -1) {
            printf("could not set font parmaters\n");
        }        

        r = make_character(&fresult, &fcparam, bmp);
        if (r == -1) {
            printf("could not make character\n");
        }

        al_set_target_backbuffer(display);

        al_draw_bitmap(bmp, NX, NY, 0);


        al_flip_display();
        
        #if 0
        // 2 Hz rate
        usleep(250*1000);
        #endif

            if(fgets(in, sizeof(in), stdin) > 0){
                if (strcmp(SEXIT, in) == 0) {
                    // quit
                    //al_destroy_font(bmpfnt);
                    //al_destroy_bitmap(bmp);
                    al_destroy_display(display);
                    return 0;
                }
            }

    }
}

