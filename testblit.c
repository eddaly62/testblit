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

#define MAX_5X7_CHARS 10
#define MAX_IN  20
#define SEXIT   "q"

#define BLACK al_map_rgb(0, 0, 0)
#define WHITE al_map_rgb(255, 255, 255)
#define RED al_map_rgb(255, 0, 0)
#define BLUE al_map_rgb(0, 0, 255)
#define GREEN al_map_rgb(0, 255, 0)

#define RESOURCES_DIR "resources"
#define TEST_TEXT "123"
//#define FONT_FILE "YARDSALE.TTF"
#define FONT_SIZE 18
#define LINE_WIDTH  1
#define DROP    2

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

char bmpfnt[] =
"............."
". 1 .222.333."
". 1 .  2.  3."
". 1 .222.333."
". 1 .2  .  3."
". 1 .222.333."
".............";

#define MAX_FONT_INDEX  150

// display this character when there is no match in the font index array
#define DEFAULT_ERR_CHAR_INDEX 0

extern char *font_design_5_7;

// style flags
const unsigned char INVERT = 1;
const unsigned char UNDER_SCORE  = 2;
const unsigned char STRIKE_THRU  = 4;
const unsigned char BLINK = 8;

struct FONT_REC_INDEX {
    char c;     // ascii character display
    int index;  // start of pattern
    int rowcnt; // number of rows
    int colcnt; // number of columns
};

struct FONT_INDEX {
    char *fp;               // pointer to font array
    int numofchars;         // number of chars in font array
    struct FONT_REC_INDEX rec[MAX_FONT_INDEX];
};

struct FONT_GET_RESULT {
    char *fp;                   // pointer to font array
    struct FONT_REC_INDEX rec;  // results of search
};


struct FONT_CHAR_PARAM {
    float scale;            // min value of ratio of display resolution to screen resolution 
    ALLEGRO_COLOR bgcolor;  // background color
    ALLEGRO_COLOR fgcolor;  // foreground color
    unsigned char style;    // invert, under-line, strike-thru, blink
};

struct FONT_INDEX font5x7index; 
struct FONT_REC_INDEX fontrec;
struct FONT_GET_RESULT fresult;

// builds a font index table that the bitmap functions will use
// returns -1 if error, otherwise the number of characters found in font array
int build_font_index(struct FONT_INDEX *fi, char *font, size_t size) {

    int n, rcnt, ccnt, nrec, idx;

    nrec = 0;
    idx = -1;

    // save pointer to font
    fi->fp = font;

    for (n = 0; n < size; n++) {
        if (font[n] == '@') {
            
            // start of new character, adjust counts and indexes
            rcnt = 0;
            ccnt = 0;
            nrec ++;
            fi->numofchars = nrec;
            idx++;

            // store character
            fi->rec[idx].c = font[n+1];
            
            // store index to character pattern
            fi->rec[idx].index = n+2;

            if ((font[n+2] != '-') && (font[n+2] != 'x')){
                // error, malformed font table
                return -1;
            }
        }
        else if ((font[n] == '-') || (font[n] == 'x')) {
            // count columns in current row
            ccnt++;
            fi->rec[idx].colcnt = ccnt;
        }
        else if ((font[n] == '=') || (font[n] == '*')) {
            // end of row
            ccnt++;
            fi->rec[idx].colcnt = ccnt;
            ccnt = 0;
            rcnt++;
            fi->rec[idx].rowcnt = rcnt;
        }
    }

    return fi->numofchars;
}



int get_font_record(char c, struct FONT_INDEX *fi, struct FONT_GET_RESULT *fr) {

    int i;

    for (i = 0; i < fi->numofchars; i++) {
        if (c == fi->rec[i].c) {
            // found character
            fr->fp = fi->fp;
            memcpy(&fr->rec, &fi->rec[i], sizeof(struct FONT_REC_INDEX));
            return 0;
        }
    }
    // did not find a match
    // return a error and a default char
    fr->fp = fi->fp;
    memcpy(&fr->rec, &fi->rec[DEFAULT_ERR_CHAR_INDEX], sizeof(struct FONT_REC_INDEX));
    return -1;
}


struct FONT_CHAR_PARAM fcparam;

int set_font_parms(struct FONT_CHAR_PARAM *fcp, float scale, ALLEGRO_COLOR bgc, ALLEGRO_COLOR fgc) {
    
    if (fcp == NULL) {
        return -1;
    }
    
    fcp->scale = scale;
    fcp->bgcolor = bgc;
    fcp->fgcolor = fgc;
    return 0;
}

int make_character(struct FONT_GET_RESULT *fr, struct FONT_CHAR_PARAM *fcp, ALLEGRO_BITMAP *b) {

    int i;
    char s;
    float r, c;
    float x0, x1;
    float y0, y1;
    ALLEGRO_COLOR color;

    al_set_target_bitmap(b);

    i = fr->rec.index;
    for (r = 0; r < fr->rec.rowcnt; r++) {
        for (c = 0; c < fr->rec.colcnt; c++) {
            x0 = fcp->scale*(c);
            x1 = fcp->scale*(c+1);
            y0 = fcp->scale*(r);
            y1 = fcp->scale*(r+1);

            switch (fr->fp[i]) {

                case '-':
                case '=':
                color = fcp->bgcolor;
                al_draw_filled_rectangle(x0, y0, x1, y1, color);
                break;

                case 'x':
                case '*':
                color = fcp->fgcolor;
                al_draw_filled_rectangle(x0, y0, x1, y1, color);
                break;

                default:
                // print unknown pixel type, fill with background color
                fprintf(stderr,"unknown pixel type, function(%s)\n", __FUNCTION__);
                color = fcp->bgcolor;
                al_draw_filled_rectangle(x0, y0, x1, y1, color);
                break;
            }
            // index to the next pixel
            i++;
        }
    }
}


int main()  {
    
    int i;
    int r;
    int toggle=1;
    bool running = true;
    int bbx;
    int bby;
    int bbw;
    int bbh;
    char in[MAX_IN];

    ALLEGRO_DISPLAY *display;
    ALLEGRO_FONT *bmpfnt;
    ALLEGRO_BITMAP *bmp;


    al_init();
    al_init_font_addon();
    //al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();

    //bmp = al_create_bitmap(10,18);
    //if (bmp == NULL){
    //    fprintf(stderr,"bmp pointer is NULL");
    //}

    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_append_path_component(path, RESOURCES_DIR);
    al_change_directory(al_path_cstr(path, '/'));
    al_destroy_path(path);

    //bmp = al_load_bitmap("testpbm.bmp");
  
    // bitmap font set up
    //int ranges[] = {49,50};
    //bmpfnt = al_grab_font_from_bitmap(bmp, 1, ranges);
    //if (bmpfnt == NULL) {
     //   fprintf(stderr, "font pointer is NULL\n");
    //}


    display = al_create_display(640, 480);
    
    //font = al_load_ttf_font(FONT_FILE, FONT_SIZE, 0);

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

#if 0
        for (i = 0; i < MAX_5X7_CHARS; i++) {
            bmp[i] = al_create_bitmap(6*2, 10*2);
        }
        make_character(bmp, 2, 5, 7);
        al_set_target_backbuffer(display);

        al_draw_bitmap(bmp, NX, NY, 0);

#endif

        //al_draw_line(0, 0, al_get_display_width(display), al_get_display_height(display), RED, 5.0);
        //al_draw_rectangle(100, 100, 300, 300, GREEN, 1);
        //al_draw_ellipse(300, 300, 120, 50, al_map_rgb(0, 0, 255), 3);
        //al_draw_spline(points, al_map_rgb(128, 128, 0), 8);
        //al_draw_polygon(polygon, 8, ALLEGRO_LINE_JOIN_BEVEL, al_map_rgb(255, 15, 15),3,1);
        //al_draw_filled_polygon(polygon, 8, al_map_rgb(255, 0, 0));

        //al_get_text_dimensions(font, TEST_TEXT, &bbx, &bby, &bbw, &bbh);

        //printf("bbx=%d, bby=%d, bbw=%d, bbh=%d\n", bbx, bby, bbw, bbh);
        //al_draw_rectangle(bbx, bby, bbx+bbw, bby+bbh, GREEN, 1);

        // Normal
        //al_draw_text(bmpfnt, BLUE, NX, NY, 0, "2");
        //al_draw_glyph(bmpfnt, BLACK, NX, NY, 1);

        //al_draw_bitmap(bmp, NX, NY, 0);

        //al_draw_bitmap(bmp, NX, NY,0);
#if 0
        // Inverted
        al_get_text_dimensions(font, INVERTED_TEXT, &bbx, &bby, &bbw, &bbh);
        al_draw_filled_rectangle(IX+bbx, IY+bby-DROP, IX+bbx+bbw, IY+bby+bbh+DROP, WHITE);
        al_draw_text(font, BLACK, IX, IY, 0, INVERTED_TEXT);

        // Strike thru
        al_get_text_dimensions(font, STRIKE_THRU_TEXT, &bbx, &bby, &bbw, &bbh);
        al_draw_text(font, WHITE, SX, SY, 0, STRIKE_THRU_TEXT);
        al_draw_line(SX+bbx, SY+bby+(bbh/2)+DROP, SX+bbx+bbw, SY+bby+(bbh/2)+DROP, WHITE, LINE_WIDTH);

        // Under-line
        al_get_text_dimensions(font, UNDER_LINE_TEXT, &bbx, &bby, &bbw, &bbh);
        al_draw_text(font, WHITE, UX, UY, 0, UNDER_LINE_TEXT);
        al_draw_line(UX+bbx, UY+bby+bbh+DROP, UX+bbx+bbw, UY+bby+bbh+DROP, WHITE, LINE_WIDTH);        

        // blinking
        if (toggle) {
            al_draw_text(font, WHITE, BX, BY, 0, BLINKING_TEXT);
            toggle=0;
        }
        else {
            al_draw_text(font, BLACK, BX, BY, 0, BLINKING_TEXT);
            toggle=1;
        }
#endif        
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

