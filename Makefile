CC=gcc
CFLAGS=-ggdb -O0
ALLEGRO_FLAGS=-I/usr/local/include/allegro5 -L/usr/local/lib/ -Wl,-R/usr/local/lib -lallegro_primitives -lallegro_image -lallegro -lallegro_color -lallegro_main -lallegro_font -lallegro_ttf

testblit: testblit.c textproc.c font_5x7.c font_5x7r.c font_7x9.c font_7x9r.c font_10x14.c font_10x14r.c
	$(CC) $(CFLAGS) -o $@ $^ $(ALLEGRO_FLAGS)
	
clean:
	rm testblit