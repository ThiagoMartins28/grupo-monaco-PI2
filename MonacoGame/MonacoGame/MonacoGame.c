#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

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
    int btn_x = 550, btn_y = 200, btn_w = 200, btn_h = 80; // Botão Start Game

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
    int pos_x = 20, pos_y = 500;
    int vel_x = 0;
    float vel_y = 0;
    int vel = 10;

    // Controle de pulo
    bool is_jumping = false;
    float jump_speed = -15;
    float gravity = 1;
    int ground_y = 500;

    // Menu
    ALLEGRO_BITMAP* bg_2 = al_load_bitmap("./img/menu-2.jpg");

    // Poções e cesto para o minigame  
    int potion_x = rand() % 1280;
    int potion_y = 0;
    int potion_speed = 5;
    int basket_w = 100, basket_h = 30;
    int basket_x, basket_y;
    int score = 0;

    ALLEGRO_BITMAP* potion = al_create_bitmap(50, 50);
    al_set_target_bitmap(potion);
    al_clear_to_color(al_map_rgb(0, 0, 255));
    al_set_target_bitmap(al_get_backbuffer(display));

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
                    state = game;
                }
            }
        }
        // Estado atual: primeira fase do jogo
        else if (state == game) {
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

                // Verifica se a bruxa chegou ao fim da tela
                if (pos_x + new_w > 1280) {
                    state = next;
                }

                // Desenha o jogo
                al_clear_to_color(al_map_rgb(105, 111, 255));
                al_draw_scaled_bitmap(pg, 0, 0, w_original, h_original, pos_x, pos_y, new_w, new_h, 0);
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
        // Estado atual: minigame da fase next
        else if (state == next) {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                pos_x += vel_x;

                // Atualiza a posição do cesto com a bruxa
                basket_x = pos_x + (new_w / 2) - (basket_w / 2); // Centralizado horizontalmente em relação à bruxa
                basket_y = pos_y - basket_h; // Colocado em cima da cabeça da bruxa

                // Aplica gravidade e atualiza o pulo
                if (is_jumping) {
                    pos_y += vel_y;
                    vel_y += gravity;
                    if (pos_y >= ground_y) {
                        pos_y = ground_y;
                        vel_y = 0;
                        is_jumping = false;
                    }
                }

                // Movimento da poção
                potion_y += potion_speed;
                if (potion_y > 720) {
                    potion_x = rand() % 1280;
                    potion_y = 0;
                }

                // Verifica colisão entre cesto e poção
                if (potion_x + 50 > basket_x && potion_x < basket_x + basket_w &&
                    potion_y + 50 > basket_y && potion_y < basket_y + basket_h) {
                    score++;
                    potion_x = rand() % 1280;
                    potion_y = 0;
                }

                // Desenha o cenário do minigame
                al_clear_to_color(al_map_rgb(200, 100, 100));
                al_draw_scaled_bitmap(pg, 0, 0, w_original, h_original, pos_x, pos_y, new_w, new_h, 0);
                al_draw_filled_rectangle(basket_x, basket_y, basket_x + basket_w, basket_y + basket_h, al_map_rgb(0, 255, 0));
                al_draw_bitmap(potion, potion_x, potion_y, 0);
                al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, 0, "Score: %d", score);
                al_flip_display();
            }

            // Controle de movimento e pulo no minigame
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    vel_x = vel;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    vel_x = -vel;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_UP && !is_jumping) {
                    is_jumping = true;
                    vel_y = jump_speed;
                }
            }
            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    vel_x = 0;
                }
            }
        }
    }

    al_destroy_bitmap(bg_2);
    al_destroy_bitmap(pg);
    al_destroy_bitmap(potion);
    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
