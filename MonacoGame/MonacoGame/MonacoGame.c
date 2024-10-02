#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <assert.h>


typedef struct {
    int pos_x, pos_y;
    int vel;
    ALLEGRO_BITMAP* direita;
    ALLEGRO_BITMAP* esquerda;
    ALLEGRO_BITMAP* atual;
    bool facing_left;
}Personagem;

typedef struct {
    ALLEGRO_DISPLAY* display;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_TIMER* timer;
    bool running;
}Jogo;

ALLEGRO_BITMAP* carregar_imagem_redimensionada(char* img, int tamanho) {
    ALLEGRO_BITMAP* bitmap = al_load_bitmap(img);
    assert(bitmap);

    int original_width = al_get_bitmap_width(bitmap);
    int original_height = al_get_bitmap_height(bitmap);

    int new_width = original_width / tamanho;
    int new_height = original_height / tamanho;

    // mexendo no tamanho da imagem
    ALLEGRO_BITMAP* new_bitmap = al_create_bitmap(new_width, new_height);
    al_set_target_bitmap(new_bitmap); // local dos príximos desenhos
    al_draw_scaled_bitmap(bitmap, 0, 0, original_width, original_height, 0, 0, new_width, new_height, 0); // desenha o bitmap no new_bitmap e redimensiona
    al_set_target_bitmap(al_get_backbuffer(al_get_current_display())); // desenha na janela exibida ao usuário
    al_destroy_bitmap(bitmap);

    return new_bitmap;
}

void inicializa_personagem(Personagem* p, char* img_direita, char* img_esquerda, int pos_x, int pos_y, int vel, int tamanho) {
    p->direita = carregar_imagem_redimensionada(img_direita, tamanho);
    p->esquerda = carregar_imagem_redimensionada(img_esquerda, tamanho);
    p->pos_x = pos_x;
    p->pos_y = pos_y;
    p->vel = vel;
    p->atual = p->direita;
    p->facing_left = false;
}

void atualiza_personagem(Personagem* p, bool key[]) {
    if (key[0]) p->pos_y -= p->vel;  // UP
    if (key[1]) p->pos_y += p->vel;  // DOWN
    if (key[2]) {                    // RIGHT
        p->pos_x += p->vel;
        p->atual = p->direita;
        p->facing_left = false;
    }
    if (key[3]) {                    // LEFT
        p->pos_x -= p->vel;
        p->atual = p->esquerda;
        p->facing_left = true;
    }
}

void desenha_personagem(Personagem* p) {
    al_draw_bitmap(p->atual, p->pos_x, p->pos_y, 0);
}

void processa_eventos(ALLEGRO_EVENT event, bool key[], bool* running) {
    if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event.keyboard.keycode) {
        case ALLEGRO_KEY_ESCAPE:
            *running = false;
            break;
        case ALLEGRO_KEY_UP:
            key[0] = true;
            break;
        case ALLEGRO_KEY_DOWN:
            key[1] = true;
            break;
        case ALLEGRO_KEY_RIGHT:
            key[2] = true;
            break;
        case ALLEGRO_KEY_LEFT:
            key[3] = true;
            break;
        }
    }
    else if (event.type == ALLEGRO_EVENT_KEY_UP) {
        switch (event.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
            key[0] = false;
            break;
        case ALLEGRO_KEY_DOWN:
            key[1] = false;
            break;
        case ALLEGRO_KEY_RIGHT:
            key[2] = false;
            break;
        case ALLEGRO_KEY_LEFT:
            key[3] = false;
            break;
        }
    }
}

void inicializa_allegro(Jogo* jogo) {
    al_init();
    al_init_image_addon();
    al_install_keyboard();

    jogo->display = al_create_display(1000, 600);
    jogo->timer = al_create_timer(1.0 / 60);
    jogo->queue = al_create_event_queue();

    al_register_event_source(jogo->queue, al_get_keyboard_event_source());
    al_register_event_source(jogo->queue, al_get_display_event_source(jogo->display));
    al_register_event_source(jogo->queue, al_get_timer_event_source(jogo->timer));

    jogo->running = true;

    al_start_timer(jogo->timer);
}

void finaliza_jogo(Jogo* jogo, Personagem* personagem) {
    al_destroy_display(jogo->display);
    al_destroy_event_queue(jogo->queue);
    al_destroy_timer(jogo->timer);
    al_destroy_bitmap(personagem->direita);
    al_destroy_bitmap(personagem->esquerda);
}


int main() {
    Jogo jogo;
    Personagem personagem;
    bool key[4] = { false, false, false, false };  // UP, DOWN, RIGHT, LEFT

    inicializa_allegro(&jogo);
    inicializa_personagem(&personagem, "./img/bruxa-right.png", "./img/bruxa-left.png", 200, 200, 5, 10);

    // loop principal do jogo
    while (jogo.running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(jogo.queue, &event);

        // processa eventos de teclado
        processa_eventos(event, key, &jogo.running);

        // evento de fechar janela
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            jogo.running = false;
        }

        // atualiza e desenha o personagem se o evento for do tipo TIMER
        if (event.type == ALLEGRO_EVENT_TIMER) {
            atualiza_personagem(&personagem, key);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            desenha_personagem(&personagem);
            al_flip_display();
        }
    }


    finaliza_jogo(&jogo, &personagem);

    return 0;
}