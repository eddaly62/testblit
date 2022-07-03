// testproc.c

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

// style flags
const unsigned char INVERT = 1;
const unsigned char UNDER_SCORE  = 2;
const unsigned char STRIKE_THRU  = 4;
const unsigned char BLINK = 8;

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
    
    // return the number of character glyphs forund in font file
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


