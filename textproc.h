#ifndef _TEXTPROC_H
#define _TEXTPROC_H

#ifdef __cplusplus
extern "C" {
#endif

// textproc.h

#include <allegro5/allegro.h>

// color definitions
#define BLACK al_map_rgb(0, 0, 0)
#define WHITE al_map_rgb(255, 255, 255)
#define RED al_map_rgb(255, 0, 0)
#define BLUE al_map_rgb(0, 0, 255)
#define GREEN al_map_rgb(0, 255, 0)

// folder were all "graphic" resources will located
#define RESOURCES_DIR "resources"

// maximum number of character in a font file
#define MAX_FONT_INDEX  150

// display this character when there is no match in the font index array
#define DEFAULT_ERR_CHAR_INDEX 0

extern const unsigned char INVERT;
extern const unsigned char UNDER_SCORE;
extern const unsigned char STRIKE_THRU;
extern const unsigned char BLINK;

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

// prototypes
int build_font_index(struct FONT_INDEX *fi, char *font, size_t size);
int get_font_record(char c, struct FONT_INDEX *fi, struct FONT_GET_RESULT *fr);
int set_font_parms(struct FONT_CHAR_PARAM *fcp, float scale, ALLEGRO_COLOR bgc, ALLEGRO_COLOR fgc);
int make_character(struct FONT_GET_RESULT *fr, struct FONT_CHAR_PARAM *fcp, ALLEGRO_BITMAP *b);


#ifdef __cplusplus
}
#endif

#endif