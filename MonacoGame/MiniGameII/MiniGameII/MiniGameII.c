#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>

#define MAX_ATTACKS 5 // Número máximo de ataques
double ultimo_tempo_colisao = 0;
double colisoes = 0.5;
#define NUM_POTIONS 7

enum GameState {
    menu,
    game,
    next
};

typedef struct {
    int w_original;
    int h_original;
    int new_w;
    int new_h;
    int w_original_left;
    int h_original_left;
    int new_w_left;
    int new_h_left;
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
    ALLEGRO_BITMAP* img;
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

typedef struct {
    int potion_x;
    int potion_y;
    int potion_speed;
    int basket_w, basket_h;
    int basket_x, basket_y;
    int score;
    int potion_atual;
    float potion_w;
    float potion_h;
    float escala;
} Potion;

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

void draw_life(ALLEGRO_BITMAP* bloco, Ground* vida, int vidas_restantes, int espaco) {
    for (int i = 0; i < vidas_restantes; i++) { // Desenhar apenas até o número de vidas restantes
        int posicao_x = vida->pos_x + (vida->new_w + espaco) * i;

        al_draw_scaled_bitmap(bloco,
            0, 0, vida->w_original, vida->h_original,
            posicao_x, vida->pos_y, vida->new_w, vida->new_h,
            0);
    }
}

int main() {
    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    enum GameState state = menu;
    int btn_x = 550, btn_y = 250, btn_w = 200, btn_h = 60; // Botão Start Game

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
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_start_timer(timer);

    // Menu
    ALLEGRO_BITMAP* bg_2 = al_load_bitmap("./img/menu-2.jpg");

    // Poções e cesto para o minigame I  
    Potion potion;
    potion.potion_x = rand() % 1280;
    potion.potion_y = 0;
    potion.potion_speed = 5;
    potion.basket_w = 100, potion.basket_h = 30;
    //basket_x, basket_y;
    potion.score = 0;

    ALLEGRO_BITMAP* potion_images[NUM_POTIONS];
    potion.potion_atual = 0;
    potion.potion_w = 0;
    potion.potion_h = 0;
    potion.escala = 0.5;
    const char* potion_files[NUM_POTIONS] = { "./img/pocoes/pocao1.png", "./img/pocoes/pocao2.png", "./img/pocoes/pocao3.png",
                                              "./img/pocoes/pocao4.png", "./img/pocoes/pocao5.png", "./img/pocoes/pocao6.png",
                                              "./img/pocoes/pocao7.png"};
    for (int i = 0; i < NUM_POTIONS; i++) {
        potion_images[i] = al_load_bitmap(potion_files[i]);

        float original_w = al_get_bitmap_width(potion_images[i]);
        float original_h = al_get_bitmap_height(potion_images[i]);

        potion.potion_w = original_w * potion.escala;
        potion.potion_h = original_h * potion.escala;
    }

    /*ALLEGRO_BITMAP* potion = al_create_bitmap(50, 50);
    al_set_target_bitmap(potion);
    al_clear_to_color(al_map_rgb(0, 0, 255));
    al_set_target_bitmap(al_get_backbuffer(display));*/

    // Personagem
    ALLEGRO_BITMAP* bruxa_right = al_load_bitmap("./img/bruxa-right.png");
    ALLEGRO_BITMAP* bruxa_left = al_load_bitmap("./img/bruxa-left.png");
    Personagem personagem;
    personagem.w_original = al_get_bitmap_width(bruxa_right);
    personagem.h_original = al_get_bitmap_height(bruxa_right);
    personagem.new_w = al_get_bitmap_width(bruxa_right) * 0.26;
    personagem.new_h = al_get_bitmap_height(bruxa_right) * 0.26;
    personagem.pos_x = 20;
    personagem.pos_y = 500;
    personagem.vel_x = 0;
    personagem.vel_y = 0;
    personagem.vel = 10;
    personagem.direcao = 1;

    // Vidas
    int numero_de_vidas = 3;
    Ground vidas; // !ALERTA DE GAMBIARRA!: usando a struct de Ground pra desenhar as imagens de vida
    ALLEGRO_BITMAP* coracao = load_ground("./img/coracao.png", &vidas, 5, 20, 0.20);

    // Inimigo
    ALLEGRO_BITMAP* cobra = al_load_bitmap("./img/gelo.png");
    Enemy enemy;
    enemy.w_original = al_get_bitmap_width(cobra);
    enemy.h_original = al_get_bitmap_height(cobra);
    enemy.new_w = al_get_bitmap_width(cobra) / 4.5;
    enemy.new_h = al_get_bitmap_height(cobra) / 4.5;
    enemy.pos_x = 1000;
    enemy.pos_y = 540;
    enemy.vel_x = -2;
    enemy.is_visible = true;

    // Item drop
    ALLEGRO_BITMAP* pocao = al_load_bitmap("./img/PNG/azul.png");
    Item item;
    item.w_original = al_get_bitmap_width(pocao);
    item.h_original = al_get_bitmap_height(pocao);
    item.new_w = al_get_bitmap_width(pocao) / 6;
    item.new_h = al_get_bitmap_height(pocao) / 6;
    item.pos_x = enemy.pos_x;
    item.pos_y = enemy.pos_y;
    item.is_visible = true;

    // Fontes de texto
    //ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_FONT* font = al_load_font("./Arial.ttf", 20, 0);

    // Chão
    Ground ground;
    ALLEGRO_BITMAP* bloco = load_ground("./img/bloco.png", &ground, 0, 592, 0.33);

    // Ataques
    ALLEGRO_BITMAP* attack_img = al_load_bitmap("./img/fogo.png");
    ALLEGRO_BITMAP* attack_img_left = al_load_bitmap("./img/fogo_left.png");
    Ataque ataques[MAX_ATTACKS] = { 0 }; // Array para os ataques


    // Loop principal
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        // Movimento de pulo do personagem
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

        // Escolha da imagem a partir da direção
        ALLEGRO_BITMAP* personagem_img;

        if (personagem.direcao == 1) {
            personagem_img = bruxa_right;
        }
        else {
            personagem_img = bruxa_left;
        }

        // Estado atual: menu
        if (state == menu) {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(bg_2, 0, 0, 0);
            al_draw_filled_rectangle(btn_x + 2, btn_y + 2, btn_x + btn_w + 2, btn_y + btn_h + 2, al_map_rgb(50, 50, 50));
            al_draw_filled_rectangle(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, al_map_rgb(162, 40, 206));
            al_draw_text(font, al_map_rgb(255, 255, 255), btn_x + btn_w / 2, btn_y + btn_h / 4, ALLEGRO_ALIGN_CENTRE, "Start Game");
            al_draw_text(font, al_map_rgb(0, 0, 0), btn_x + btn_w / 2 + 2, btn_y + btn_h / 4 + 2, ALLEGRO_ALIGN_CENTRE, "Start Game");
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
                personagem.pos_x += personagem.vel_x;

                // Atualiza a posição do cesto com a bruxa
                potion.basket_x = personagem.pos_x + (personagem.new_w / 2) - (potion.basket_w / 2); // Centralizado horizontalmente em relação à bruxa
                potion.basket_y = personagem.pos_y - potion.basket_h; // Colocado em cima da cabeça da bruxa

                // Movimento da poção
                potion.potion_y += potion.potion_speed;
                if (potion.potion_y > 720) {
                    potion.potion_x = rand() % 1280 - potion.potion_w;
                    potion.potion_y = 0;
                    potion.potion_atual = rand() % NUM_POTIONS;
                }

                // Verifica colisão entre cesto e poção
                if (potion.potion_x + 50 > potion.basket_x && potion.potion_x < potion.basket_x + potion.basket_w &&
                    potion.potion_y + 50 > potion.basket_y && potion.potion_y < potion.basket_y + potion.basket_h) {
                    potion.score++;
                    potion.potion_x = rand() % 1280 - potion.potion_w;
                    potion.potion_y = 0;
                    potion.potion_atual = rand() % NUM_POTIONS;
                }

                // Verifica se a bruxa chegou ao fim da tela
                if (personagem.pos_x + personagem.new_w > 1280) {
                    state = next;
                }

                // Desenha o cenário do minigame I
                al_clear_to_color(al_map_rgb(200, 100, 100));

                // Desenhando os blocos
                if (bloco) {
                    int qtd_blocos = 11;
                    int espaco = 0;

                    draw_ground(bloco, &ground, qtd_blocos, espaco);
                }

                al_draw_scaled_bitmap(personagem_img, 0, 0, personagem.w_original, personagem.h_original, personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);
                al_draw_filled_rectangle(potion.basket_x, potion.basket_y, potion.basket_x + potion.basket_w, potion.basket_y + potion.basket_h, al_map_rgb(0, 255, 0));
                al_draw_scaled_bitmap(potion_images[potion.potion_atual], 
                                      0, 0, al_get_bitmap_width(potion_images[potion.potion_atual]), 
                                      al_get_bitmap_height(potion_images[potion.potion_atual]),
                                      potion.potion_x, potion.potion_y, 
                                      potion.potion_w, potion.potion_h, 0);
                al_draw_textf(font, al_map_rgb(255, 255, 255), 20, 20, 0, "Score: %d", potion.score);
                al_flip_display();
            }
        }

        // Estado atual: segunda fase do jogo
        else if (state == next) {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                
                // Colisão
                if (enemy.is_visible) {

                    bool colidiu = personagem.pos_x < enemy.pos_x + enemy.new_w &&
                        personagem.pos_x + personagem.new_w > enemy.pos_x &&
                        personagem.pos_y < enemy.pos_y + enemy.new_h &&
                        personagem.pos_y + personagem.new_h > enemy.pos_y;

                    if (colidiu) {

                        double tempo_atual = al_get_time();

                        if (tempo_atual - ultimo_tempo_colisao >= colisoes) {
                            numero_de_vidas--;
                            ultimo_tempo_colisao = tempo_atual;

                            // Se as vidas acabarem, acaba o jogo
                            if (numero_de_vidas <= 0) {
                                running = false;
                                printf("Fim de jogo!");
                            }
                        }
                    }

                    // Movimento do inimigo
                    enemy.pos_x += enemy.vel_x;

                    // Fazer o inimigo reaparecer do outro lado da tela, se sair
                    if (enemy.pos_x + enemy.new_w < 0) { // Saiu pela esquerda
                        enemy.pos_x = 1280; // Reaparece no lado direito
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
                            item.is_visible = false;
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

                // Desenhando personagem
                al_draw_scaled_bitmap(personagem_img, 0, 0, personagem.w_original, personagem.h_original, personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);

                // Desenhando ataque
                for (int i = 0; i < MAX_ATTACKS; i++) {
                    if (ataques[i].active) {
                        al_draw_scaled_bitmap(ataques[i].img, 0, 0, al_get_bitmap_width(ataques[i].img), al_get_bitmap_height(ataques[i].img), ataques[i].pos_x, ataques[i].pos_y, ataques[i].w, ataques[i].h, 0);
                    }
                }

                // Desenhando inimigo
                if (enemy.is_visible) {
                    al_draw_scaled_bitmap(cobra, 0, 0, enemy.w_original, enemy.h_original, enemy.pos_x, enemy.pos_y, enemy.new_w, enemy.new_h, 0);
                }

                // Desenhando os blocos
                if (bloco) {
                    int qtd_blocos = 11;
                    int espaco = 0;

                    draw_ground(bloco, &ground, qtd_blocos, espaco);
                }

                if (coracao) {
                    int qtd_coracao = 3;
                    int espaco = 2;

                    draw_life(coracao, &vidas, numero_de_vidas, espaco);
                }

                // Desenha item
                if (!item.is_visible) {
                    al_draw_scaled_bitmap(pocao, 0, 0, item.w_original, item.h_original, item.pos_x, item.pos_y, item.new_w, item.new_h, 0);
                }

                al_flip_display();
            }
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

                        if (personagem.direcao == 1) {
                            ataques[i].img = attack_img;
                            ataques[i].pos_x = personagem.pos_x + personagem.new_w;
                        }
                        else if (personagem.direcao == -1){
                            ataques[i].img = attack_img_left;
                            ataques[i].pos_x = personagem.pos_x - (al_get_bitmap_width(attack_img_left) / 6);
                        }

                        ataques[i].pos_y = personagem.pos_y + personagem.new_h / 2 - (al_get_bitmap_height(ataques[i].img) / 6) / 2;

                        ataques[i].w = al_get_bitmap_width(ataques[i].img) / 6;
                        ataques[i].h = al_get_bitmap_height(ataques[i].img) / 6;

                        ataques[i].vel_x = 5 * personagem.direcao;
                        break;
                    }
                }
                break;
            }
        }

        // Controlo de movimento
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
    al_destroy_bitmap(bruxa_right);
    al_destroy_bitmap(bruxa_left);
    al_destroy_bitmap(attack_img);
    al_destroy_bitmap(cobra);
    al_destroy_bitmap(bloco);
    al_destroy_bitmap(coracao);
    al_destroy_font(font);
    al_destroy_bitmap(pocao);
    for (int i = 0; i < NUM_POTIONS; i++) {
        if (potion_images[i]) {
            al_destroy_bitmap(potion_images[i]);
        }
    }

    return 0;
}
