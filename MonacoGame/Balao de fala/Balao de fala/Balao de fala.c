
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>





int main(){
al_init();
al_init_primitives_addon();
al_init_font_addon();
al_init_ttf_addon();

	// Initialize Allegro and add-ons
	if (!al_init()) {
		printf("Failed to initialize Allegro.\n");
		return -1;
	}

	if (!al_init_primitives_addon()) {
		printf("Falha ao inicializar o addon de primitivas.\n");

		return -1;
	}
	if (!al_init_font_addon()) {
		printf("Falha ao inicializar o addon de fontes.\n");
		return -1;
	}
	if (!al_init_ttf_addon()) {
		printf("Falha ao inicializar o addon de fontes TTF.\n");
		return -1;
	}

	

	//Display
	ALLEGRO_DISPLAY* display = al_create_display(800, 600);
	if (!display) {
		printf("falha a criar display.\n");
		return -1;
		
	}

	//font
	ALLEGRO_FONT* font = al_create_builtin_font();
	if (!font) {
		printf("Falha a carregar font.\n");
		al_destroy_display(display);
		return -1;
	}

	//cor balao
	ALLEGRO_COLOR ballon_color = al_map_rgb(255,255,255);
	ALLEGRO_COLOR border_color = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR text_color = al_map_rgb(0, 0, 0);

	//tamanho e posiçao
	float x = 200, y = 100;

	float width = 300, height = 100;
	float border_radius = 20;

	float tip_x = 50, tip_y = y + height + 20;

	//limpa tela
	al_clear_to_color(al_map_rgb(255, 255, 255));

	//corpo balao
	al_draw_filled_rounded_rectangle(x, y, x + width, y + height, border_radius, border_radius, ballon_color);
	al_draw_rounded_rectangle(x, y, x + width, y + height, border_radius, border_radius, border_color, 2.0);

	//(triangulo balao)
	al_draw_filled_triangle(tip_x, tip_y, x + width / 2 - 20, y + height, x + width / 2 + 20, y + height, ballon_color);
	al_draw_triangle(tip_x, tip_y, x + width / 2 - 20, y + height, x + width / 2 + 20, y + height, border_color, 2.0);

	//bolha de fala
	const char* text = "Hello, world!";
	al_draw_text(font, text_color, x + 20, y + 30, 0, text);  // Adjust text position as needed

	//texto balao
	al_flip_display();
	al_rest(5.0);

	//finaliza

	al_destroy_font(font);
	al_destroy_display(display);
	al_shutdown_primitives_addon();

	

	return 0;
}

