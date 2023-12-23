/* Copyright 2022 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quantum.h"
#include "rgb_matrix.h"

// clang-format off

const matrix_row_t matrix_mask[] = {
    0b1111111111111111,
    0b1111111111111111,
    0b1111111111111111,
    0b1111111111111111,
    0b1111111111111111,
    0b1111111111101111,
};

//
bool game_mod = false;

// Map object
#define i_empty  0
#define i_light  1
#define i_player 2

//
enum gameIndex {
    NONE,
    COLLECT_APPLE
};
enum gameIndex current_game = NONE;

//
static short RNDSEED = 2;
short randomval(short max) {
    RNDSEED += 1;
    return ((unsigned short)(RNDSEED * 1664525 + 1013904223) % max);
}

//
short clamp(short value, short min, short max) {
    if (value <= min) {
        return min;
    } else if (value >= max) {
        return max;
    }

    return value;
}

//
short player_y = 0;
short player_x = 0;

#define LVL_X 17
#define LVL_Y 6
short led_matrix[LVL_Y][LVL_X] = {
    //                                       knob
    //                                       \/
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 }
};

// Item struct
struct s_item {
    bool destroy; 
    short x, y;
};

// Apple obj
#define i_apple 3
struct s_item o_apple = {};

//
void apple_init(void);
void apple_init() {
    short x = randomval(10);
    short y = randomval(4);
    led_matrix[y][x] = i_apple;

    o_apple.x = x;
    o_apple.y = y;
    o_apple.destroy = false;
}

//
void clear_level(void);
void clear_level() {
    for (short y = 0; y < LVL_Y; y++) {
        for (short x = 0; x < LVL_X; x++) {
            if (led_matrix[y][x] == i_apple) {
                led_matrix[y][x] = i_empty;
            }  
        }
    }
}


// clang-format on

#ifdef DIP_SWITCH_ENABLE

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0) {
        default_layer_set(1UL << (active ? 2 : 0));
    }
    return true;
}

#endif

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_LED_INDEX)

//
bool check_collision(void);
bool check_collision() {
    if (led_matrix[player_y][player_x] == i_apple) {
        apple_init();
        return true;
    }

    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static bool rshift_pressed = false;
    static bool rctrl_pressed = false;
    
    switch (keycode) {
        //
        case KC_RCTL:
            rctrl_pressed = record->event.pressed;
        break;
        case KC_RSFT:
            rshift_pressed = record->event.pressed;
        break;

        // Game 1
        case KC_1:
            if (rctrl_pressed && rshift_pressed) {
                if (record->event.pressed) {
                    game_mod = !game_mod;

                    rgb_matrix_set_flags(LED_FLAG_NONE);
                    rgb_matrix_set_color_all(0, 0, 0);
                    
                    if (game_mod) {
                        // Enable / Disable game mode
                        current_game = COLLECT_APPLE;
                        apple_init();
                    } else {
                        current_game = NONE;
                        clear_level();
                    }
                }
            }
        break;
        //

        case KC_RGHT:
            if (record->event.pressed) {
                led_matrix[player_y][player_x] = i_empty;
                player_x += 1;
                check_collision();
            }
        break;

        case KC_LEFT:
            if (record->event.pressed) {
                led_matrix[player_y][player_x] = i_empty;
                player_x -= 1;
                check_collision();
            }
        break;

        case KC_UP:
            if (record->event.pressed) {
                led_matrix[player_y][player_x] = i_empty;
                player_y -= 1;
                check_collision();
            }
        break;

        case KC_DOWN:
            if (record->event.pressed) {
                led_matrix[player_y][player_x] = i_empty;
                player_y += 1;
                check_collision();
            }
        break;
    }

    return true;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) { return false; }
    switch (keycode) {
#ifdef RGB_MATRIX_ENABLE
        case RGB_TOG:
            if (record->event.pressed) {
                switch (rgb_matrix_get_flags()) {
                    case LED_FLAG_ALL: {
                        rgb_matrix_set_flags(LED_FLAG_NONE);
                        rgb_matrix_set_color_all(0, 0, 0);
                    } break;
                    default: {
                        rgb_matrix_set_flags(LED_FLAG_ALL);
                    } break;
                }
            }
            if (!rgb_matrix_is_enabled()) {
                rgb_matrix_set_flags(LED_FLAG_ALL);
                rgb_matrix_enable();
            }
            return false;
#endif
    }
    return true;
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }

    if (current_game == COLLECT_APPLE) {
        player_y = clamp(player_y, 0, 4);

        if (player_y == 0) {
            player_x = clamp(player_x, 0, 9);
        } else if (player_y == 1) {
            player_x = clamp(player_x, 0, 11);
        } else if (player_y == 2) {
            player_x = clamp(player_x, 0, 11);
        } else if (player_y == 3) {
            player_x = clamp(player_x, 0, 10);
        } else if (player_y == 4) {
            player_x = clamp(player_x, 0, 9);
        }

        led_matrix[player_y][player_x] = i_player;

        for (short y = 0; y < LVL_Y; y++) {
            for (short x = 0; x < LVL_X; x++) {
                short count = 0;
                switch (y) {
                    case 0: count = 1; break;
                    case 1: count = 17; break;
                    case 2: count = 34; break;
                    case 3: count = 51; break;
                    case 4: count = 64; break;
                }

                if (led_matrix[y][x] == i_player) {
                    RGB_MATRIX_INDICATOR_SET_COLOR(count+x, 255, 0, 0);
                } else if (led_matrix[y][x] == i_apple) {
                    RGB_MATRIX_INDICATOR_SET_COLOR(count+x, 0, 255, 0);
                } else if (led_matrix[y][x] == i_empty) {
                    RGB_MATRIX_INDICATOR_SET_COLOR(count+x, 0, 0, 0);
                }
            }
        }
    }
    
    if (host_keyboard_led_state().caps_lock) {
        RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED_INDEX, 255, 255, 255);
    } else {
        if (!rgb_matrix_get_flags()) {
           RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED_INDEX, 0, 0, 0);
        }
    }
    return true;
}

#endif // CAPS_LOCK_LED_INDEX
