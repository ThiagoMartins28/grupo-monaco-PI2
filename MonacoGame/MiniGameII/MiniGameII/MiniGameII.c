#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

typedef struct {

    int w_original;
    int h_original;
    int new_w;
    int new_h;
    int pos_x;
    int vel_x;
    float vel_y;
    int vel;

}pg2;

int main() {

    al_init();
    al_install_keyboard();
    al_init_image_addon();

    bool running = true;

    pg2 pg2;


    // Display
    ALLEGRO_DISPLAY* display = al_create_display(1280, 716);
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_start_timer(timer);

    // Personagem
    ALLEGRO_BITMAP* pg = al_load_bitmap("./img/bruxa-right.png");
    int w_original = al_get_bitmap_width(pg);
    int h_original = al_get_bitmap_height(pg);
    int new_w = w_original / 6;
    int new_h = h_original / 6;
    int pos_x = 20, pos_y = 500;
    int vel_x = 0;
    float vel_y = 0;
    int vel = 10;

    // Inimigo
    ALLEGRO_BITMAP* cobra = al_load_bitmap("./img/cobra_esq.png");
    pg2.w_original = al_get_bitmap_width(cobra);
    pg2.h_original = al_get_bitmap_height(cobra);
    pg2.new_w = w_original / 6;
    pg2.new_h = h_original / 6;
    pg2.pos_x = 20, pos_y = 500;
    pg2.vel_x = 0;
    pg2.vel_y = 0;
    pg2.vel = 10;

    // Controle de pulo
    bool is_jumping = false;
    float jump_speed = -15;
    float gravity = 1;
    int ground_y = 500;

    // Ataque
    bool is_atacking = false;


    // Loop
    while (running) {

        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        if (running) {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                pos_x += vel_x;

                // Aplica gravidade se estiver no ar
                if (is_jumping) {
                    pos_y += vel_y;
                    vel_y += gravity;

                    // Limita a posição ao nível do chão
                    if (pos_y >= ground_y) {
                        pos_y = ground_y;
                        vel_y = 0;
                        is_jumping = false;
                    }
                }

                // Desenha o jogo
                al_clear_to_color(al_map_rgb(105, 111, 255));
                al_draw_scaled_bitmap(pg, 0, 0, w_original, h_original, pos_x, pos_y, new_w, new_h, 0);
                al_draw_scaled_bitmap(cobra, 0, 0, w_original, h_original, pos_x, pos_y, new_w, new_h, 0);
                al_flip_display();
            }

            // Controles de teclado para o movimento e o pulo
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                    vel_x = vel;
                    break;
                case ALLEGRO_KEY_LEFT:
                    vel_x = -vel;
                    break;
                case ALLEGRO_KEY_UP:
                    if (!is_jumping) {
                        is_jumping = true;
                        vel_y = jump_speed;
                    }
                    break;
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                case ALLEGRO_KEY_LEFT:
                    vel_x = 0;
                    break;
                }
            }

        }
    }

    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);

    return 0;
}