#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>


enum GameState {
    menu,
    game
};

int main() {

    al_init();
    al_install_mouse();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_image_addon();

    bool running = true;
    enum GameState state = menu;
    int btn_x = 550, btn_y = 200, btn_w = 200, btn_h = 80; // Start game

    // Display
    ALLEGRO_DISPLAY* display = al_create_display(1280, 716);
    ALLEGRO_BITMAP* icon = al_load_bitmap("./img/pocao.png"); // Icon do display
    al_set_window_position(display, 200, 200);
    al_set_window_title(display, "Monaco");
    al_set_display_icon(display, icon);
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // Personagem
    ALLEGRO_BITMAP* right;
    ALLEGRO_BITMAP* left;
    ALLEGRO_BITMAP* atual;
    int pos_x, pos_y, vel;
    bool facing_left;

    ALLEGRO_BITMAP* pg = al_load_bitmap("./img/bruxa-right.png");
    int w_original = al_get_bitmap_width(pg);
    int h_original = al_get_bitmap_height(pg);
    int new_w = w_original / 6;
    int new_h = h_original / 6;

    // Imagens
    ALLEGRO_BITMAP* bg = al_load_bitmap("./img/menu-pocoes.jpg");
    ALLEGRO_BITMAP* bg_2 = al_load_bitmap("./img/menu-2.jpg");

    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue(); // Fila de evento
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_start_timer(timer);

    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        // Estado do menu
        if (state == menu) {
            // Limpando a tela
            al_clear_to_color(al_map_rgb(0, 0, 0));

            al_draw_bitmap(bg_2, 0, 0, 0);
            // Desenhando botão
            al_draw_filled_rectangle(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, al_map_rgb(162, 40, 206));
            al_draw_text(font, al_map_rgb(255, 255, 255), btn_x + btn_w / 2, btn_y + btn_h / 4, ALLEGRO_ALIGN_CENTRE, "Start Game");

            // Atualizando a tela
            al_flip_display();

            // Detectando clique no botão
            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.mouse.x >= btn_x && event.mouse.x <= btn_x + btn_w &&
                    event.mouse.y >= btn_y && event.mouse.y <= btn_y + btn_h) {
                    state = game;  // Mudar para o estado do jogo
                }
            }
        }
        // Estado do jogo após clicar no botão
        else if (state == game) {
            // Limpar a tela para o próximo estado
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // Atualizar a tela
            al_clear_to_color(al_map_rgb(105, 111, 255));
            
            //al_draw_bitmap(bg, 0, 0, 0);
            al_draw_scaled_bitmap(pg, 0, 0, w_original, h_original, 20, 500, new_w, new_h, 0);
            
            al_flip_display();
        }

    }

    al_destroy_bitmap(bg);
    al_destroy_bitmap(bg_2);
    al_destroy_bitmap(pg);
    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);


    return 0;
}