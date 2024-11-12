#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>

#define MAX_ATTACKS 5 // Número máximo de ataques

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
    int direcao;
} Personagem;

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
    int direcao;
    bool active;
} Ataque;

typedef struct {
    int w_original;
    int h_original;
    int new_w;
    int new_h;
    int pos_x;
    int pos_y;
} Ground;

typedef struct {
    int w_original;
    int h_original;
    int new_w;
    int new_h;
    int pos_x;
    int pos_y;
    bool is_visible;
} Item;

// Carrega imagens do chão
ALLEGRO_BITMAP* load_ground(int caminho[], Ground* ground, int pos_x, int pos_y, float escala) {
    ALLEGRO_BITMAP* imagem = al_load_bitmap(caminho);

    ground->w_original = al_get_bitmap_width(imagem);
    ground->h_original = al_get_bitmap_height(imagem);

    ground->new_w = ground->w_original * escala;
    ground->new_h = ground->h_original * escala;

    // Posição
    ground->pos_x = pos_x;
    ground->pos_y = pos_y;

    return imagem;
}

// Desenha as imagens do chão no display
void draw_ground(ALLEGRO_BITMAP* bloco, Ground* ground, int qtd, int espaco) {

    for (int i = 0; i < qtd; i++) {
        int posicao_x = ground->pos_x + (ground->new_w + espaco) * i;

        al_draw_scaled_bitmap(bloco,
            0, 0, ground->w_original, ground->h_original,
            posicao_x, ground->pos_y, ground->new_w, ground->new_h,
            0);
    }
}

// Carregar e exibi a caixa de texto
void draw_text_box(ALLEGRO_FONT* font, const char* text) {
    al_draw_filled_rectangle(400, 300, 880, 400, al_map_rgb(255, 255, 255)); 
    al_draw_text(font, al_map_rgb(0, 0, 0), 640, 340, ALLEGRO_ALIGN_CENTER, text); 
}

int main() {
    al_init();
    al_install_keyboard();
    al_init_image_addon();
    al_init_primitives_addon();

    bool running = true;
    bool is_jumping = false;
    float jump_speed = -15;
    float gravity = 1;
    int ground_y = 500;

    ALLEGRO_DISPLAY* display = al_create_display(1280, 716);
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_start_timer(timer);

    // Personagem
    ALLEGRO_BITMAP* bruxa = al_load_bitmap("./img/bruxa-right.png");
    ALLEGRO_BITMAP* bruxa_left = al_load_bitmap("./img/bruxa-left.png");
    Personagem personagem;
    personagem.w_original = al_get_bitmap_width(bruxa);
    personagem.h_original = al_get_bitmap_height(bruxa);
    personagem.new_w = al_get_bitmap_width(bruxa) / 6;
    personagem.new_h = al_get_bitmap_height(bruxa) / 6;
    personagem.pos_x = 20;
    personagem.pos_y = 500;
    personagem.vel_x = 0;
    personagem.vel_y = 0;
    personagem.vel = 10;
    personagem.direcao = 1;

    // Inimigo
    ALLEGRO_BITMAP* cobra = al_load_bitmap("./img/cobra_esq.png");
    Enemy enemy;
    enemy.w_original = al_get_bitmap_width(cobra);
    enemy.h_original = al_get_bitmap_height(cobra);
    enemy.new_w = al_get_bitmap_width(cobra) / 6;
    enemy.new_h = al_get_bitmap_height(cobra) / 6;
    enemy.pos_x = 1000;
    enemy.pos_y = 540;
    enemy.vel_x = -3;
    enemy.is_visible = true;

    // Item drop
    ALLEGRO_BITMAP* pocao = al_load_bitmap("./img/pocao.png");
    Item item;
    item.w_original = al_get_bitmap_width(pocao);
    item.h_original = al_get_bitmap_height(pocao);
    item.new_w = al_get_bitmap_width(pocao) / 6;
    item.new_h = al_get_bitmap_height(pocao) / 6;
    item.pos_x = enemy.pos_x;
    item.pos_y = enemy.pos_y;
    item.is_visible = true;

    // Fonte de texto
    ALLEGRO_FONT* font = al_create_builtin_font();

    // Chão
    Ground ground;
    ALLEGRO_BITMAP* bloco = load_ground("./img/bloco.png", &ground, 0, 592, 0.33);

    // Ataques
    ALLEGRO_BITMAP* attack_img = al_load_bitmap("./img/fogo.png");
    Ataque ataques[MAX_ATTACKS] = { 0 }; // Array para os ataques


    // Loop principal
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        if (event.type == ALLEGRO_EVENT_TIMER) {
            // Movimento do personagem
            personagem.pos_x += personagem.vel_x;
            if (is_jumping) {
                personagem.pos_y += personagem.vel_y;
                personagem.vel_y += gravity;
                if (personagem.pos_y >= ground_y) {
                    personagem.pos_y = ground_y;
                    personagem.vel_y = 0;
                    is_jumping = false;
                }
            }

            // Controle dos ataques
            for (int i = 0; i < MAX_ATTACKS; i++) {
                if (ataques[i].active) {
                    ataques[i].pos_x += ataques[i].vel_x;

                    // Colisão com o inimigo
                    float distancia = sqrt(pow(enemy.pos_x - ataques[i].pos_x, 2) + pow(enemy.pos_y - ataques[i].pos_y, 2));
                    if (distancia < 50 && enemy.is_visible) {
                        enemy.is_visible = false;
                        ataques[i].active = false;

                        // Inicializar item na posição do inimigo derrotado
                        item.pos_x = enemy.pos_x;
                        item.pos_y = enemy.pos_y;
                        item.w_original = al_get_bitmap_width(pocao);
                        item.h_original = al_get_bitmap_height(pocao);
                        item.new_w = al_get_bitmap_width(pocao) / 6;
                        item.new_h = al_get_bitmap_height(pocao) / 6;
                        item.is_visible = true;
                    }

                    // Desativa ataque se sair da tela
                    if (ataques[i].pos_x > 1280) {
                        ataques[i].active = false;
                    }
                }
            }

            // Atualiza posição do inimigo
            if (enemy.is_visible) {
                enemy.pos_x += enemy.vel_x;
                if (enemy.pos_x < 0) {
                    enemy.pos_x = 1280;
                }
            }

            // Renderização
            al_clear_to_color(al_map_rgb(105, 111, 255));
            //al_draw_scaled_bitmap(bruxa, 0, 0, personagem.w_original, personagem.h_original, personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);
            
            // Escolha da imagem a partir da direção
            ALLEGRO_BITMAP* personagem_img;

            if (personagem.direcao == 1) {
                personagem_img = bruxa;
            }
            else {
                personagem_img = bruxa_left;
            }

            al_draw_scaled_bitmap(personagem_img, 0, 0, personagem.w_original, personagem.h_original, personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);

            for (int i = 0; i < MAX_ATTACKS; i++) {
                if (ataques[i].active) {
                    al_draw_scaled_bitmap(attack_img, 0, 0, al_get_bitmap_width(attack_img), al_get_bitmap_height(attack_img), ataques[i].pos_x, ataques[i].pos_y, ataques[i].w, ataques[i].h, 0);
                }
            }

            if (enemy.is_visible) {
                al_draw_scaled_bitmap(cobra, 0, 0, enemy.w_original, enemy.h_original, enemy.pos_x, enemy.pos_y, enemy.new_w, enemy.new_h, 0);
            }

            // Desenhando os blocos
            if (bloco) {
                int qtd_blocos = 11;
                int espaco = 0;

                draw_ground(bloco, &ground, qtd_blocos, espaco);
            }

            // Desenha item
            if (item.is_visible) {
                al_draw_scaled_bitmap(pocao, 0, 0, item.w_original, item.h_original, item.pos_x, item.pos_y, item.new_w, item.new_h, 0);
            }

            // Verificar se o personagem pegou o item
            float distancia_item = sqrt(pow(item.pos_x - personagem.pos_x, 2) + pow(item.pos_y - personagem.pos_y, 2));
            if (distancia_item < 30 && item.is_visible) {
                item.is_visible = false; // Ocultar item após coleta
                draw_text_box(font, "Item coletado!"); // Exibir a caixa de texto
            }

            al_flip_display();
        }

        // Controles de teclado
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (event.keyboard.keycode) {
            case ALLEGRO_KEY_RIGHT:
                personagem.vel_x = personagem.vel;
                personagem.direcao = 1;
                break;
            case ALLEGRO_KEY_LEFT:
                personagem.vel_x = -personagem.vel;
                personagem.direcao = -1;
                break;
            case ALLEGRO_KEY_UP:
                if (!is_jumping) {
                    is_jumping = true;
                    personagem.vel_y = jump_speed;
                }
                break;
            case ALLEGRO_KEY_SPACE: // Lançar um ataque
                for (int i = 0; i < MAX_ATTACKS; i++) {
                    if (!ataques[i].active) {
                        ataques[i].active = true;
                        ataques[i].pos_x = personagem.pos_x + personagem.new_w;
                        ataques[i].pos_y = personagem.pos_y + personagem.new_h / 2;
                        ataques[i].w = al_get_bitmap_width(attack_img) / 6;
                        ataques[i].h = al_get_bitmap_height(attack_img) / 6;
                        ataques[i].vel_x = 5 * personagem.direcao;
                        break;
                    }
                }
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

    // Libera memória
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);
    al_destroy_bitmap(bruxa);
    al_destroy_bitmap(bruxa_left);
    al_destroy_bitmap(attack_img);
    al_destroy_bitmap(cobra);
    al_destroy_bitmap(bloco);
    al_destroy_font(font);
    al_destroy_bitmap(pocao);

    return 0;
}
