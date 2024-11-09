#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

typedef struct {
    int w_original;
    int h_original;
    int new_w;
    int new_h;
    int pos_x;
    int pos_y;
    int vel_x;
    float vel_y;
    int vel;
    int health;
}Personagem;

typedef struct {
    int w_original;
    int h_original;
    int new_w;
    int new_h;
    int pos_x;
    int pos_y;
    int vel_x;
    float vel_y;
    int vel;
    bool is_visible;
} Enemy;

typedef struct {
    int w;
    int h;
    float pos_x;
    float pos_y;
    float vel_x;
    float vel_y;
    bool active;
} Ataque;

int main() {

    al_init();
    al_install_keyboard();
    al_init_image_addon();
    al_init_primitives_addon();

    bool running = true;
    bool is_attacking = false;
    int attack_duration = 30; // duração do ataque em frames
    int attack_counter = 0;

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
    Personagem personagem;
    personagem.w_original = al_get_bitmap_width(pg);
    personagem.h_original = al_get_bitmap_height(pg);
    personagem.new_w = personagem.w_original / 6;
    personagem.new_h = personagem.h_original / 6;
    personagem.pos_x = 20, personagem.pos_y = 500;
    personagem.vel_x = 0;
    personagem.vel_y = 0;
    personagem.vel = 10;
    personagem.health = 2;

    // Inimigo
    ALLEGRO_BITMAP* cobra = al_load_bitmap("./img/cobra_esq.png");
    Enemy enemy;
    enemy.w_original = al_get_bitmap_width(cobra);
    enemy.h_original = al_get_bitmap_height(cobra);
    enemy.new_w = enemy.w_original / 6;
    enemy.new_h = enemy.h_original / 6;
    enemy.pos_x = 1000; // posição inicial do inimigo
    enemy.pos_y = 500;
    enemy.vel_x = -3; // velocidade do inimigo
    enemy.vel_y = 0;
    enemy.vel = 10;
    enemy.is_visible = true;

    ALLEGRO_BITMAP* attack_img = al_load_bitmap("./img/fogo.png");
    Ataque ataque;
    ataque.w = al_get_bitmap_width(attack_img) / 8;
    ataque.h = al_get_bitmap_height(attack_img) / 8;
    ataque.active = false; // Ataque começa inativo

    // Controle de pulo
    bool is_jumping = false;
    float jump_speed = -15;
    float gravity = 1;
    int ground_y = 500;

    // Loop
    while (running) {

        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        if (running) {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                personagem.pos_x += personagem.vel_x;

                // Desenha o jogo
                al_clear_to_color(al_map_rgb(105, 111, 255));

                // Aplica gravidade se estiver no ar
                if (is_jumping) {
                    personagem.pos_y += personagem.vel_y;
                    personagem.vel_y += gravity;
                    if (personagem.pos_y >= ground_y) {
                        personagem.pos_y = ground_y;
                        personagem.vel_y = 0;
                        is_jumping = false;
                    }
                }


                // Controle do ataque
                if (is_attacking) {
                    attack_counter++;
                    if (attack_counter >= attack_duration) {
                        is_attacking = false;
                        attack_counter = 0;
                    }
                }

                if (enemy.is_visible) {
                    enemy.pos_x += enemy.vel_x;
                    float distancia = sqrt(pow(enemy.pos_x - personagem.pos_x, 2) + pow(enemy.pos_y - personagem.pos_y, 2));

                    // Desativa o inimigo se o ataque colide
                    if (is_attacking && distancia < 50) {
                        enemy.is_visible = false;
                        is_attacking = false; // Desativa o ataque ao acertar
                    }
                }

                // Desenha o personagem com a imagem de ataque se ele estiver atacando
                if (is_attacking) {
                    al_draw_scaled_bitmap(attack_img, 0, 0, al_get_bitmap_width(attack_img), al_get_bitmap_height(attack_img), personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);
                }
                else {
                    al_draw_scaled_bitmap(pg, 0, 0, personagem.w_original, personagem.h_original, personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);
                }

                // Desenha o inimigo se ele estiver visível
                if (enemy.is_visible) {
                    al_draw_scaled_bitmap(cobra, 0, 0, enemy.w_original, enemy.h_original, enemy.pos_x, enemy.pos_y, enemy.new_w, enemy.new_h, 0);
                }

                // Calcular a distância entre o personagem e o inimigo
                float distancia = sqrt(pow(enemy.pos_x - personagem.pos_x, 2) + pow(enemy.pos_y - personagem.pos_y, 2));

                al_flip_display();
            }

            // Controles de teclado para o movimento, pulo e ataque
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                    personagem.vel_x = personagem.vel;
                    break;
                case ALLEGRO_KEY_LEFT:
                    personagem.vel_x = -personagem.vel;
                    break;
                case ALLEGRO_KEY_UP:
                    if (!is_jumping) {
                        is_jumping = true;
                        personagem.vel_y = jump_speed;
                    }
                    break;
                case ALLEGRO_KEY_SPACE: // Tecla de ataque
                    is_attacking = true;
                    break;
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_RIGHT:
                case ALLEGRO_KEY_LEFT:
                    personagem.vel_x = 0;
                    break;
                }
            }
        }
    }

    // Libera memória
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);
    al_destroy_bitmap(pg);
    al_destroy_bitmap(attack_img);
    al_destroy_bitmap(cobra);

    return 0;
}
