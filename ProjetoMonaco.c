#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

int main() {

    const int largura_t = 640;
    const int altura_t = 400;

    int pos_x = 100;
    int pos_y = 100;

    bool fim = false;
    ALLEGRO_EVENT_QUEUE* fila_eventos = NULL;


    ALLEGRO_DISPLAY* display = NULL;

    if (!al_init()) {
        al_show_native_message_box(NULL, "Erro!", "Erro ao inicializar", "Aviso!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }

    display = al_create_display(largura_t, altura_t);

    if (!display) {
        al_show_native_message_box(NULL, "Erro!", "Erro ao inicializar", "Aviso!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    }

    // inicializando addons e instalações
    al_install_keyboard();

    // criação de filas e dispositivos
    fila_eventos = al_create_event_queue();
    al_init_primitives_addon();

    // sources
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_display_event_source(display));

    // loop principal
    while (!fim) {
        ALLEGRO_EVENT evento;

        al_wait_for_event(fila_eventos, &evento);

        if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                fim = true;
            }

            switch (evento.keyboard.keycode) 
            {
            case ALLEGRO_KEY_UP:
                pos_y -= 10;
                break;
            case ALLEGRO_KEY_DOWN:
                pos_y += 10;
                break;
            case ALLEGRO_KEY_RIGHT:
                pos_x += 10;
                break;
            case ALLEGRO_KEY_LEFT:
                pos_x -= 10;
                break;
            }
        }

        else if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            fim = true;
        }


        // desenho
        al_draw_filled_rectangle(pos_x, pos_y, pos_x + 30, pos_y + 30, al_map_rgb(255, 255, 0));
        al_flip_display();
    }

    // finalização
    al_destroy_display(display);
    al_destroy_event_queue(fila_eventos);

    return 0;
}