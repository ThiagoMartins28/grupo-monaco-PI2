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
    int potion_held;
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
} Objeto;

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

typedef struct {
    float button_x;
    float button_y;
    float button_width;
    float button_height;
    bool button_pressed;
} Book_button;

typedef struct {
    int x;
    int y;
    int potion_type;
}DroppedItem;

typedef struct {
    int x, y;      
    int w, h; 
    bool is_visible; 
    ALLEGRO_BITMAP* imagem_mistura;
} Mistura;

// Carrega imagens do chão
ALLEGRO_BITMAP* load_object(int caminho[], Objeto* objeto, int pos_x, int pos_y, float escala) {
    ALLEGRO_BITMAP* imagem = al_load_bitmap(caminho);

    objeto->w_original = al_get_bitmap_width(imagem);
    objeto->h_original = al_get_bitmap_height(imagem);

    objeto->new_w = objeto->w_original * escala;
    objeto->new_h = objeto->h_original * escala;

    // Posição
    objeto->pos_x = pos_x;
    objeto->pos_y = pos_y;

    return imagem;
}

// Desenha as imagens do chão no display
void draw_object(ALLEGRO_BITMAP* bloco, Objeto* objeto, int qtd, int espaco) {

    for (int i = 0; i < qtd; i++) {
        int posicao_x = objeto->pos_x + (objeto->new_w + espaco) * i;

        al_draw_scaled_bitmap(bloco,
            0, 0, objeto->w_original, objeto->h_original,
            posicao_x, objeto->pos_y, objeto->new_w, objeto->new_h,
            0);
    }
}

void verificar_colisao_e_movimento(Personagem* personagem, Enemy* enemy, double* ultimo_tempo_colisao, int* numero_de_vidas, bool* running, double colisoes) {
    if (enemy->is_visible) {

        bool colidiu = personagem->pos_x < enemy->pos_x + enemy->new_w &&
            personagem->pos_x + personagem->new_w > enemy->pos_x &&
            personagem->pos_y < enemy->pos_y + enemy->new_h &&
            personagem->pos_y + personagem->new_h > enemy->pos_y;

        if (colidiu) {
            double tempo_atual = al_get_time();

            if (tempo_atual - *ultimo_tempo_colisao >= colisoes) {
                (*numero_de_vidas)--;
                *ultimo_tempo_colisao = tempo_atual;

                if (*numero_de_vidas <= 0) {
                    *running = false;
                    printf("Game over!\n");
                }
            }
        }

        enemy->pos_x += enemy->vel_x;

        if (enemy->pos_x + enemy->new_w < 0) { 
            enemy->pos_x = 1280; 
        }
    }
}

// Misturas
bool combina_2(int a, int b, int x, int y) {
    return (a == x && b == y) || (a == y && b == x);
}

bool combina_3(int a, int b, int c, int x, int y, int z) {
    return (a == x && b == y && c == z) ||
        (a == x && b == z && c == y) ||
        (a == y && b == x && c == z) ||
        (a == y && b == z && c == x) ||
        (a == z && b == x && c == y) ||
        (a == z && b == y && c == x);
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
    int btn_x = 550, btn_y = 250, btn_w = 200, btn_h = 60; // Botão Start Game

    // Poções e cesto para o minigame I  
    Potion potion;
    potion.potion_x = rand() % 1280;
    potion.potion_y = 0;
    potion.potion_speed = 5;
    potion.basket_w = 100, potion.basket_h = 30;
    //basket_x, basket_y;
    potion.score = 0;
    bool potion_is_dropped = false;

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

    // Misturas
    /*ALLEGRO_BITMAP* mistura_amarela = NULL;
    ALLEGRO_BITMAP* mistura_roxo = NULL;
    ALLEGRO_BITMAP* mistura_verde = NULL;*/
    #define MAX_MISTURA 3
    Mistura mistura[MAX_MISTURA];
    int total_mistura = 0;
    for (int i = 0; i < MAX_MISTURA; i++) {
        mistura[i].x = 600;
        mistura[i].y = 400;
        mistura[i].w = 60;
        mistura[i].h = 90;
        mistura[i].is_visible = false;
        mistura[i].imagem_mistura = NULL;
    }
 
    // Coleta e drop de poções
    #define MAX_ITEMS 10
    DroppedItem dropped_items[MAX_ITEMS]; // Array para armazenar os itens
    int dropped_item_count = 0; // Contador de itens droppados

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
    personagem.potion_held = -1;

    // Vidas
    int numero_de_vidas = 3;
    Objeto vidas; 
    ALLEGRO_BITMAP* coracao = load_object("./img/coracao.png", &vidas, 5, 20, 0.20);

    // Book
    Objeto inventario; 
    ALLEGRO_BITMAP* book = load_object("./img/book.png", &inventario, 1200, 20, 0.20);
    Book_button book_button;
    book_button.button_x = 1200;  
    book_button.button_y = 20;  
    book_button.button_width = 60; 
    book_button.button_height = 60;
    book_button.button_pressed = false;

    // Caldeirão
    Objeto caldeirao;
    ALLEGRO_BITMAP* caldao = load_object("./img/caldeirao.png", &caldeirao, 600, 510, 0.25);
    int drop_zone = 600;
    int drop_zone_w = 200;

    // Inimigo
    ALLEGRO_BITMAP* inimigo_ghost = al_load_bitmap("./img/ghost.png");
    ALLEGRO_BITMAP* inimigo_gelo = al_load_bitmap("./img/gelo.png");
    Enemy enemy;
    enemy.w_original = al_get_bitmap_width(inimigo_ghost);
    enemy.h_original = al_get_bitmap_height(inimigo_ghost);
    enemy.new_w = al_get_bitmap_width(inimigo_ghost) / 4.5;
    enemy.new_h = al_get_bitmap_height(inimigo_ghost) / 4.5;
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
    ALLEGRO_FONT* font = al_load_font("./Arial.ttf", 20, 0);

    // Chão
    Objeto ground;
    ALLEGRO_BITMAP* bloco = load_object("./img/bloco.png", &ground, 0, 592, 0.33);

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
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Verifica se o clique está dentro da área do botão invisível.
            if (event.mouse.x >= book_button.button_x && event.mouse.x <= book_button.button_x + book_button.button_width &&
                event.mouse.y >= book_button.button_y && event.mouse.y <= book_button.button_y + book_button.button_height) {
                book_button.button_pressed = true;
            }
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

        // Estado atual: segunda fase do jogo
        else if (state == game) {
            if (event.type == ALLEGRO_EVENT_TIMER) {

                // Colisão com o inimigo
                verificar_colisao_e_movimento(&personagem, &enemy, &ultimo_tempo_colisao, &numero_de_vidas, &running, colisoes);

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

                // Verifica se a bruxa chegou ao fim da tela
                if (personagem.pos_x + personagem.new_w > 1280) {
                    state = next;
                    personagem.pos_x = 0;
                    personagem.pos_y = 500;
                }

                // Renderização
                al_clear_to_color(al_map_rgb(105, 111, 255));
            
                // Book Botão 
                // desenhar o contorno do botão
                al_draw_rectangle(book_button.button_x, book_button.button_y,
                    book_button.button_x + book_button.button_width, book_button.button_y + book_button.button_height,
                    al_map_rgb(255, 0, 0), 2);

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
                    al_draw_scaled_bitmap(inimigo_gelo, 0, 0, enemy.w_original, enemy.h_original, enemy.pos_x, enemy.pos_y, enemy.new_w, enemy.new_h, 0);
                }

                // Desenhando os blocos
                if (bloco) {
                    int qtd_bloco = 11;
                    int espaco = 0;

                    draw_object(bloco, &ground, qtd_bloco, espaco);
                }

                if (coracao) {
                    int espaco = 2;

                    draw_object(coracao, &vidas, numero_de_vidas, espaco);
                }

                if (book) {
                    int qtd_book = 1;
                    int espaco = 2;

                    draw_object(book, &inventario, qtd_book, espaco);
                }

                if (!item.is_visible) {
                    bool coletou_item = personagem.pos_x < item.pos_x + item.new_w &&
                        personagem.pos_x + personagem.new_w > item.pos_x &&
                        personagem.pos_y < item.pos_y + item.new_h &&
                        personagem.pos_y + personagem.new_h > item.pos_y;

                    if (coletou_item) {
                        potion.score += 1; // Incrementa o placar
                        item.is_visible = true; // Oculta o item após a coleta
                        printf("Item coletado! Score: %d\n", potion.score);
                    }
                }

                // Desenha item
                if (!item.is_visible) {
                    al_draw_scaled_bitmap(pocao, 0, 0, item.w_original, item.h_original, item.pos_x, item.pos_y, item.new_w, item.new_h, 0);
                }

                al_draw_textf(font, al_map_rgb(255, 255, 255), 640, 20, 0, "Potions: %d", potion.score);
                al_flip_display();
            }
        }

        // Estado atual: primeira fase do jogo
        else if (state == next) {
            if (event.type == ALLEGRO_EVENT_TIMER) {

                // Reinicializa o inimigo no início da próxima fase
                if (!enemy.is_visible) {
                    enemy.is_visible = true;
                    enemy.pos_x = 1280;
                    enemy.pos_y = 540;
                }

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
                    personagem.potion_held = potion.potion_atual; // Poção coletada
                    potion.potion_x = rand() % 1280 - potion.potion_w;
                    potion.potion_y = 0;
                    potion.potion_atual = rand() % NUM_POTIONS;
                }

                // Colisão com inimigo
                verificar_colisao_e_movimento(&personagem, &enemy, &ultimo_tempo_colisao, &numero_de_vidas, &running, colisoes);

                // Atualiza posição do inimigo
                if (enemy.is_visible) {
                    enemy.pos_x += enemy.vel_x;
                    if (enemy.pos_x < 0) {
                        enemy.pos_x = 1280;
                    }
                }

                // Desenha o cenário do minigame I
                al_clear_to_color(al_map_rgb(200, 100, 100));

                // Desenhando os blocos
                if (bloco) {
                    int qtd_blocos = 11;
                    int espaco = 0;

                    draw_object(bloco, &ground, qtd_blocos, espaco);
                }

                // Desenha vidas
                if (coracao) {
                    int espaco = 2;

                    draw_object(coracao, &vidas, numero_de_vidas, espaco);
                }

                if (book) {
                    int qtd_book = 1;
                    int espaco = 2;

                    draw_object(book, &inventario, qtd_book, espaco);
                }

                // Desenha caldeirão
                if (caldao) {
                    int qtd_caldao = 1;
                    int espaco = 2;

                    draw_object(caldao, &caldeirao, qtd_caldao, espaco);
                }

                // Verifica se a bruxa está na zona de drop
                bool is_in_drop_zone = (personagem.pos_x >= drop_zone && personagem.pos_x <= drop_zone + drop_zone_w);

                // Desenha poção coletada na tela
                if (personagem.potion_held != -1) {
                    int width = 80;  
                    int height = 80; 

                    al_draw_scaled_bitmap(
                        potion_images[personagem.potion_held], 
                        0, 0,                                 
                        al_get_bitmap_width(potion_images[personagem.potion_held]), 
                        al_get_bitmap_height(potion_images[personagem.potion_held]), 
                        640,  // x
                        50,  // y
                        width,                         
                        height,                        
                        0                                     
                    );
                }

                // Desenha poção dropada
                for (int i = 0; i < dropped_item_count; i++) {
                    int potion_type = dropped_items[i].potion_type;
                    int width = 80;
                    int height = 80;

                    al_draw_scaled_bitmap(
                        potion_images[potion_type],
                        0, 0,
                        al_get_bitmap_width(potion_images[potion_type]),
                        al_get_bitmap_height(potion_images[potion_type]),
                        dropped_items[i].x,
                        dropped_items[i].y - 150,
                        width,
                        height,
                        0
                    );
                }

                // Verifica se as poções estão no caldeirão e desenha a mistura
                if (is_in_drop_zone && potion_is_dropped) {
                    if (dropped_item_count >= 3) {
                        int index1 = dropped_items[dropped_item_count - 3].potion_type;
                        int index2 = dropped_items[dropped_item_count - 2].potion_type;
                        int index3 = dropped_items[dropped_item_count - 1].potion_type;
                       
                        if (total_mistura < MAX_MISTURA) {
                            Mistura nova_mistura;
                            nova_mistura.is_visible = true;
                            nova_mistura.x = 600;
                            nova_mistura.y = 400;
                            nova_mistura.w = 80;
                            nova_mistura.h = 80;

                            // Misturando
                            if (combina_2(index1, index2, 5, 6)) {
                                nova_mistura.imagem_mistura = al_load_bitmap("./img/PNG/amarelo.png");
                                nova_mistura.is_visible = true;
                            }
                            else if (combina_2(index1, index2, 0, 2)) {
                                nova_mistura.imagem_mistura = al_load_bitmap("./img/PNG/roxo.png");
                                nova_mistura.is_visible = true;
                            }
                            else if (combina_3(index1, index2, index3, 1, 3, 5)) {
                                nova_mistura.imagem_mistura = al_load_bitmap("./img/PNG/verde.png");
                                nova_mistura.is_visible = true;
                            }

                            mistura[total_mistura] = nova_mistura;  // Adiciona a nova mistura
                            total_mistura++;

                            dropped_item_count = 0;
                            potion.score--;
                            potion_is_dropped = false;
                        }
                    }
                }
                
                // Score após drop de poção
                if (potion_is_dropped == true) {
                    potion.score--;
                    potion_is_dropped = false;
                }

                // Desenha mistura feita com as poções
                for (int i = 0; i < total_mistura; i++) {
                    if (mistura[i].is_visible) {
                        al_draw_scaled_bitmap(
                            mistura[i].imagem_mistura,
                            0, 0,
                            al_get_bitmap_width(mistura[i].imagem_mistura),
                            al_get_bitmap_height(mistura[i].imagem_mistura),
                            mistura[i].x,
                            mistura[i].y,
                            mistura[i].w,
                            mistura[i].h,
                            0
                        );
                    }
                }

                // Verifica se o personagem coletou a mistura
                for (int i = 0; i < total_mistura; i++) {
                    if (mistura[i].is_visible) {
                        bool coletou_mistura =
                            personagem.pos_x < mistura[i].x + mistura[i].w &&
                            personagem.pos_x + personagem.new_w > mistura[i].x &&
                            personagem.pos_y < mistura[i].y + mistura[i].h &&
                            personagem.pos_y + personagem.new_h > mistura[i].y;

                        if (coletou_mistura) {
                            potion.score += 1;
                            mistura[i].is_visible = false;
                            printf("Mistura coletada! Score: %d\n", potion.score);
                        }
                    }
                }

                // Desenha mistura maior na tela acima
                for (int i = 0; i < total_mistura; i++) {
                    if (mistura[i].is_visible) {
                        int width = 100;
                        int height = 130;
                        al_draw_scaled_bitmap(
                            mistura[i].imagem_mistura,
                            0, 0,
                            al_get_bitmap_width(mistura[i].imagem_mistura),
                            al_get_bitmap_height(mistura[i].imagem_mistura),
                            640,  // x
                            50,  // y
                            width,
                            height,
                            0
                        );
                    }
                }

                // Desenha inimigo
                if (enemy.is_visible) {
                    al_draw_scaled_bitmap(inimigo_ghost, 0, 0, enemy.w_original, enemy.h_original, enemy.pos_x, enemy.pos_y, enemy.new_w, enemy.new_h, 0);
                }

                al_draw_scaled_bitmap(personagem_img, 0, 0, personagem.w_original, personagem.h_original, personagem.pos_x, personagem.pos_y, personagem.new_w, personagem.new_h, 0);
                al_draw_filled_rectangle(potion.basket_x, potion.basket_y, potion.basket_x + potion.basket_w, potion.basket_y + potion.basket_h, al_map_rgb(200, 100, 100)); // Cesto para pegar as poções
                al_draw_scaled_bitmap(potion_images[potion.potion_atual], 
                                      0, 0, 
                                      al_get_bitmap_width(potion_images[potion.potion_atual]), 
                                      al_get_bitmap_height(potion_images[potion.potion_atual]),
                                      potion.potion_x, 
                                      potion.potion_y, 
                                      potion.potion_w, 
                                      potion.potion_h, 0);
                al_draw_textf(font, al_map_rgb(255, 255, 255), 640, 20, 0, "Potions: %d", potion.score);
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

                        ataques[i].vel_x = 10 * personagem.direcao;
                        break;
                    }
                }
                break;
            case ALLEGRO_KEY_ENTER: // Drop de item
                if (personagem.potion_held != -1 && dropped_item_count < MAX_ITEMS) {
                    potion_is_dropped = true;
                    // Adiciona a poção carregada no vetor de itens dropados
                    dropped_items[dropped_item_count].x = personagem.pos_x + (personagem.new_w / 2);
                    dropped_items[dropped_item_count].y = personagem.pos_y + personagem.new_h;
                    dropped_items[dropped_item_count].potion_type = personagem.potion_held;

                    // Itens dropados
                    dropped_item_count++;

                    // Tira a poção dropada da tela
                    personagem.potion_held = -1;
                }
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
    al_destroy_bitmap(inimigo_gelo);
    al_destroy_bitmap(inimigo_ghost);
    al_destroy_bitmap(bloco);
    al_destroy_bitmap(coracao);
    al_destroy_bitmap(book);
    al_destroy_font(font);
    al_destroy_bitmap(pocao);
    for (int i = 0; i < NUM_POTIONS; i++) {
        if (potion_images[i]) {
            al_destroy_bitmap(potion_images[i]);
        }
    }

    return 0;
}
