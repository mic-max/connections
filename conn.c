#include <ncurses/ncurses.h>
#include <ncurses/menu.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define NUM_GROUPS 4
#define GROUP_SIZE 4
#define MAX_MISTAKES 4
#define NUM_WORDS NUM_GROUPS * GROUP_SIZE

char *words[] = {
    "swing", "sandbox", "seesaw", "slide",
    "clout", "pull", "sway", "weight",
    "times", "courier", "papyrus", "impact",
    "nike", "zeus", "poseidon", "aphrodite"
};

char *descs[] = {
    "0", "1", "2", "3",
    "1", "1", "1", "1",
    "2", "2", "2", "2",
    "3", "3", "3", "3"
};

// char *categories[] = {
//     "Found in a playground",
//     "Word for influence",
//     "Font type",
//     "Egyptian hieroglyphs"
// };

// TODO: Make shuffle work directly on item**
void shuffle(int *array, size_t n) {
    if (n > 1)
    {
        for (size_t i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int temp = array[j];
          array[j] = array[i];
          array[i] = temp;
        }
    }
}

int num_items_selected(MENU *menu) {
    int count = 0;
    ITEM **items = menu_items(menu);
    for (int i = 0; i < item_count(menu); i++) {
        count += item_value(items[i]);
    }

    return count;
}

int main() {
    ITEM **items;
    int c;
    MENU *my_menu;
    WINDOW *my_menu_win;
    int n_words;
    int mistakes_left;
    int *display_map;

    /* Initialize game */
    srand(time(NULL));
    n_words = ARRAY_SIZE(words);
    mistakes_left = MAX_MISTAKES;

    /* Shuffle items */
    display_map = calloc(n_words, sizeof(int));
    for (int i = 0; i < n_words; i++) {
        display_map[i] = i;
    }
    shuffle(display_map, n_words);

    /* Initialize curses */	
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Check if player log exists
    // If today's is complete, show time remaining until tomorrow and exit

    // Fetch items
    // TODO make HTTP request to server

    /* Create items */
    items = (ITEM **)calloc(n_words, sizeof(ITEM *));
    for(int i = 0; i < n_words; i++) {
        // TODO: Use description to set which group an item belongs to
        items[i] = new_item(words[display_map[i]], descs[display_map[i]]);
    }

    /* Create menu */
    my_menu = new_menu((ITEM **)items);
    menu_opts_off(my_menu, O_ONEVALUE);

    /* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 70, 4, 4);
    keypad(my_menu_win, TRUE);
     
    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 68, 3, 1));
    set_menu_format(my_menu, 4, 4);
    set_menu_mark(my_menu, " * ");

    /* Print a border around the main window */
    box(my_menu_win, 0, 0);
    
    mvprintw(LINES - 5, 0, "Mistakes Left: %d", mistakes_left);
    mvprintw(LINES - 3, 0, "Use Arrow Keys to navigate and Space Bar to select items");
    mvprintw(LINES - 2, 0, "F1 to Exit - F2 to Clear Selection - F3 to Shuffle)");
    mvprintw(LINES - 1, 0, "Enter to Submit");
    refresh();

    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);
    refresh();

    while((c = wgetch(my_menu_win)) != KEY_F(1)) {
        switch(c) {
            case KEY_DOWN: {
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            }
            case KEY_UP: {
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            }
            case KEY_LEFT: {
                menu_driver(my_menu, REQ_LEFT_ITEM);
                break;
            }
            case KEY_RIGHT: {
                menu_driver(my_menu, REQ_RIGHT_ITEM);
                break;
            }
            case ' ': {
                // Toggle Current Word
                // TODO: No difference between a selected item you hover vs unselected
                if (num_items_selected(my_menu) < GROUP_SIZE || item_value(current_item(my_menu))) {
                    menu_driver(my_menu, REQ_TOGGLE_ITEM);
                }
                break;
            }
            case 10: {
                // Enter / Submit
                // ITEM **items;
                // int *counts = calloc(4, sizeof(int));
                // items = menu_items(my_menu);
                // for (int i = 0; i < item_count(my_menu); i++) {
                //     if(item_value(items[i]) == TRUE) {
                //         counts[atoi(item_description(items[i]))]++;
                //     }
                // }
                move(20, 0);
                clrtoeol();
                // If only one away
                if (FALSE) {
                    mvprintw(20, 0, "Only one away!");
                }
                refresh();
                break;
            }
            case KEY_F(2): {
                // Deselect all
                for (int i = 0; i < item_count(my_menu); i++) {
                    set_item_value(items[i], FALSE);
                }
                break;
            }
            case KEY_F(3): {
                // Shuffle: Maintain selection & cursor location
                unpost_menu(my_menu);

                // int cur_item_index = item_index(current_item(my_menu));
                // set_current_item(my_menu, items[cur_item_index]);

                bool item_vals[n_words];
                shuffle(display_map, n_words);
                for(int i = 0; i < n_words; i++) {
                    item_vals[display_map[i]] = item_value(items[i]);
                    free_item(items[i]); // Removing will cause memory leak?
                    items[i] = new_item(words[display_map[i]], descs[display_map[i]]);
                }

                set_menu_items(my_menu, items);
                post_menu(my_menu);

                // Must set_item_value AFTER post_menu
                for (int i = 0; i < n_words; i++) {
                    // works for first shuffle
                    set_item_value(items[display_map[i]], item_vals[i]);
                }
                break;
            }
        }
        wrefresh(my_menu_win);
    }

    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for(int i = 0; i < n_words; i++) {
        free_item(items[i]);
    }
    endwin();

    return 0;
}
