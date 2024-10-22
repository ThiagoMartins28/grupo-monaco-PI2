#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

enum GameState {
    menu,
    game,
    next
};

int main() {

    al_init();
    al_install_mouse();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_image_addon();

    bool running = true;
    enum GameState state = menu;
    int btn_x = 550, btn_y = 200, btn_w = 200, btn_h = 80; // Start game

    // Display
    ALLEGRO_DISPLAY* display = al_create_display(1280, 716);
    ALLEGRO_BITMAP* icon = al_load_bitmap("./img/pocao.png");
    al_set_window_position(display, 200, 200);
    al_set_window_title(display, "Monaco");
    al_set_display_icon(display, icon);
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // Personagem
    ALLEGRO_BITMAP* pg = al_load_bitmap("./img/bruxa-right.png");
    int w_original = al_get_bitmap_width(pg);
    int h_original = al_get_bitmap_height(pg);
    int new_w = w_original / 6;
    int new_h = h_original / 6;
    // Posição e velocidade do personagem
    int pos_x = 20, pos_y = 500; 
    int vel_x = 0, vel_y = 0; 
    int vel = 10; 
    // Menu
    //ALLEGRO_BITMAP* bg = al_load_bitmap("./img/menu-pocoes.jpg");
    ALLEGRO_BITMAP* bg_2 = al_load_bitmap("./img/menu-2.jpg");

    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_start_timer(timer);

    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        // Estado atual: menu
        if (state == menu) {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(bg_2, 0, 0, 0);
            al_draw_filled_rectangle(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, al_map_rgb(162, 40, 206));
            al_draw_text(font, al_map_rgb(255, 255, 255), btn_x + btn_w / 2, btn_y + btn_h / 4, ALLEGRO_ALIGN_CENTRE, "Start Game");
            al_flip_display();

            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.mouse.x >= btn_x && event.mouse.x <= btn_x + btn_w &&
                    event.mouse.y >= btn_y && event.mouse.y <= btn_y + btn_h) {
                    state = game;  // Mudar para o estado do jogo
                }
            }
        }
        // Estado atual: jogo
        else if (state == game) {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                // Atualiza posição do personagem
                pos_x += vel_x;
                pos_y += vel_y;

                if (pos_x + new_w > 1280) {
                    state = next;
                }

                // Limpa tela e desenhar o personagem
                al_clear_to_color(al_map_rgb(105, 111, 255)); // Cor background
                al_draw_scaled_bitmap(pg, 0, 0, w_original, h_original, pos_x, pos_y, new_w, new_h, 0);
                al_flip_display();
            }

            // Eventos do teclado
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                    vel_x = vel;
                    break;
                case ALLEGRO_KEY_LEFT:
                    vel_x = -vel;
                    break;
                case ALLEGRO_KEY_UP:
                    vel_y = -vel;
                    break;
                case ALLEGRO_KEY_DOWN:
                    vel_y = vel;
                    break;
                }
            }
            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                case ALLEGRO_KEY_LEFT:
                    vel_x = 0;
                    break;
                case ALLEGRO_KEY_UP:
                case ALLEGRO_KEY_DOWN:
                    vel_y = 0;
                    break;
                }
            }
        }
        // Próxima fase
        else if (state == next) {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                // Atualiza posição do personagem
                pos_x += vel_x;
                pos_y += vel_y;

                if (pos_x + new_w > 1280) {
                    pos_x = 0; 
                }

                if (pos_x + new_w < 0) {
                    pos_x = 0;
                }

                // Limpa tela e desenha o personagem na nova fase
                al_clear_to_color(al_map_rgb(200, 100, 100));  // Cor background 
                al_draw_scaled_bitmap(pg, 0, 0, w_original, h_original, pos_x, pos_y, new_w, new_h, 0);
                al_flip_display();
            }

            // Eventos de teclado para a próxima fase
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                    vel_x = vel;
                    break;
                case ALLEGRO_KEY_LEFT:
                    vel_x = -vel;
                    break;
                case ALLEGRO_KEY_UP:
                    vel_y = -vel;
                    break;
                case ALLEGRO_KEY_DOWN:
                    vel_y = vel;
                    break;
                }
            }
            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                case ALLEGRO_KEY_LEFT:
                    vel_x = 0;
                    break;
                case ALLEGRO_KEY_UP:
                case ALLEGRO_KEY_DOWN:
                    vel_y = 0;
                    break;
                }
            }
        }
    }

    // Limpa a memória
    //al_destroy_bitmap(bg);
    al_destroy_bitmap(bg_2);
    al_destroy_bitmap(pg);
    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
