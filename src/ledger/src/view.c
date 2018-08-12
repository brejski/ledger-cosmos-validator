/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "view.h"
#include "view_templates.h"
#include "common.h"

#include "glyphs.h"
#include "bagl.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define TRUE  1
#define FALSE 0

enum UI_DISPLAY_MODE {
    VALUE_SCROLLING,
    KEY_SCROLLING_NO_VALUE,
    KEY_SCROLLING_SHORT_VALUE,
    PENDING
};

ux_state_t ux;
enum UI_STATE view_uiState;
enum UI_DISPLAY_MODE scrolling_mode;

volatile char view_data_height[MAX_SCREEN_LINE_WIDTH];
volatile char view_data_height2[MAX_SCREEN_LINE_WIDTH];
volatile char view_data_round[MAX_SCREEN_LINE_WIDTH];
volatile char view_data_publicKey[MAX_SCREEN_LINE_WIDTH];
int8_t data_round;
int64_t data_height;

//------ Event handlers
delegate_accept_reference_signature eh_accept = NULL;
delegate_reject_reference_signature eh_reject = NULL;
delegate_validation_reset           eh_validation_reset = NULL;

void view_set_validation_reset_eh(delegate_validation_reset delegate)
{
    eh_validation_reset = delegate;
}

void view_set_accept_eh(delegate_accept_reference_signature delegate)
{
    eh_accept = delegate;
}

void view_set_reject_eh(delegate_reject_reference_signature delegate)
{
    eh_reject = delegate;
}

//------ View elements
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

const ux_menu_entry_t menu_main[] = {
#ifdef TESTING_ENABLED
    {NULL, NULL, 0, NULL, "Cosmos TEST!", "Validator", 0, 0},
#else
    {NULL, NULL, 0, &C_icon_dashboard, "Cosmos", "Validator", 0, 0},
#endif
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END
};

static const bagl_element_t bagl_ui_initialize_transaction[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_CROSS),
    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000, "Init validation"),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_height),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_round),
};

static const bagl_element_t bagl_ui_validating_transaction[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000, "Validating"),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_publicKey),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_height2),
};


//static const bagl_element_t bagl_ui_transaction_info_valuescrolling[] = {
//    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
//    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_LEFT),
//    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_RIGHT),
//    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000, (const char *) pageInfo),
//    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000, (const char *) transactionDataKey),
//    UI_LabelLineScrolling(2, 0, 30, 128, 11, 0xFFFFFF, 0x000000, (const char *) transactionDataValue),
//};

//static const bagl_element_t bagl_ui_transaction_info_keyscrolling[] = {
//    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
//    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_LEFT),
//    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_RIGHT),
//    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000, (const char *) pageInfo),
//    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000, (const char *) transactionDataValue),
//    UI_LabelLineScrolling(2, 0, 19, 128, 11, 0xFFFFFF, 0x000000, (const char *) transactionDataKey),
//};
//------ View elements

// ------ Event handlers



//static unsigned int bagl_ui_sign_transaction_button(unsigned int button_mask,
//                                                    unsigned int button_mask_counter) {
//    switch (button_mask) {
//    default:view_display_transaction_menu(0);
//    }
//    return 0;
//}
//
//const bagl_element_t *ui_transaction_info_prepro(const bagl_element_t *element) {
//
//    switch (element->component.userid) {
//    case 0x01:UX_CALLBACK_SET_INTERVAL(2000);
//        break;
//    case 0x02:UX_CALLBACK_SET_INTERVAL(MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
//        break;
//    case 0x03:UX_CALLBACK_SET_INTERVAL(MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
//        break;
//    }
//    return element;
//}
//
//void submenu_left() {
//    transactionChunksPageIndex--;
//    scrolling_mode = PENDING;
//    display_transaction_page();
//}
//
//void submenu_right() {
//    transactionChunksPageIndex++;
//    scrolling_mode = PENDING;
//    display_transaction_page(TRUE);
//}
//
//void menu_left() {
//    scrolling_mode = PENDING;
//    transactionChunksPageIndex = 0;
//    transactionChunksPageCount = 1;
//    if (transactionDetailsCurrentPage > 0) {
//        transactionDetailsCurrentPage--;
//        display_transaction_page();
//    } else {
//        view_display_transaction_menu(0);
//    }
//}
//
//void menu_right() {
//    scrolling_mode = PENDING;
//    transactionChunksPageIndex = 0;
//    transactionChunksPageCount = 1;
//    if (transactionDetailsCurrentPage < transactionDetailsPageCount - 1) {
//        transactionDetailsCurrentPage++;
//        display_transaction_page();
//    } else {
//        view_display_transaction_menu(0);
//    }
//}
//

static unsigned int bagl_ui_initialize_transaction_button(
        unsigned int button_mask,
        unsigned int button_mask_counter) {

    switch (button_mask) {

        // Press left to progress to the previous element
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            if (eh_reject != NULL) {
                eh_reject();
            }
            break;
        }

        // Press right to progress to the next element
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            if (eh_accept != NULL) {
                eh_accept(data_round, data_height);
            }
            break;
        }
    }
    return 0;
}

static unsigned int bagl_ui_validating_transaction_button(
        unsigned int button_mask,
        unsigned int button_mask_counter) {

    switch (button_mask) {
        // Press both left and right to switch to value scrolling
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            if (eh_validation_reset != NULL) {
                eh_validation_reset();
            }
            view_display_main_menu();
            break;
        }
    }
    return 0;
}

//
//void crop_transaction_key()
//{
//    int offset = strlen((char *) transactionDataKey) - MAX_SCREEN_LINE_WIDTH;
//    if (offset > 0) {
//        char *start = (char *) transactionDataKey;
//        for (;;) {
//            *start = start[offset];
//            if (*start++ == '\0')
//                break;
//        }
//    }
//}
//
//static unsigned int bagl_ui_transaction_info_keyscrolling_button(unsigned int button_mask,
//                                                                 unsigned int button_mask_counter) {
//    switch (button_mask) {
//        // Press both left and right to switch to value scrolling
//        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
//            if (scrolling_mode == KEY_SCROLLING_NO_VALUE) {
//                display_transaction_page();
//            }
//            else {
//                view_display_transaction_menu(0);
//            }
//            break;
//        }
//
//        // Press left to progress to the previous element
//        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
//            if (transactionChunksPageIndex > 0) {
//                submenu_left();
//            } else {
//                menu_left();
//            }
//            break;
//        }
//
//        // Hold left to progress to the previous element quickly
//        // It also steps out from the value chunk page view mode
//        case BUTTON_EVT_FAST | BUTTON_LEFT: {
//            menu_left();
//            break;
//        }
//
//        // Press right to progress to the next element
//        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
//            if (transactionChunksPageIndex < transactionChunksPageCount - 1) {
//                submenu_right();
//            } else {
//                menu_right();
//            }
//            break;
//        }
//
//        // Hold right to progress to the next element quickly
//        // It also steps out from the value chunk page view mode
//        case BUTTON_EVT_FAST | BUTTON_RIGHT: {
//            menu_right();
//            break;
//        }
//    }
//    return 0;
//}

//void display_transaction_page() {
//    update_transaction_page_info();
//    if (scrolling_mode == VALUE_SCROLLING) {
//        UX_DISPLAY(bagl_ui_transaction_info_valuescrolling, ui_transaction_info_prepro);
//    } else {
//        UX_DISPLAY(bagl_ui_transaction_info_keyscrolling, ui_transaction_info_prepro);
//    }
//}
//
//void start_transaction_info_display(unsigned int unused) {
//    scrolling_mode = PENDING;
//    transactionDetailsCurrentPage = 0;
//    transactionChunksPageIndex = 0;
//    transactionChunksPageCount = 1;
//    display_transaction_page();
//}
//
//void update_transaction_page_info() {
//
//    if (event_handler_update_transaction_info != NULL) {
//        int index = transactionChunksPageIndex;
//
//        // Read key and value strings from json
//        event_handler_update_transaction_info(
//            (char *) transactionDataKey,
//            sizeof(transactionDataKey),
//            (char *) transactionDataValue,
//            sizeof(transactionDataValue),
//            transactionDetailsCurrentPage,
//            &index);
//        transactionChunksPageCount = index;
//
//        // If value is very long we split it into chunks
//        // and add chunk index/count information at the of the key
//        if (transactionChunksPageCount > 1) {
//            int position = strlen((char *) transactionDataKey);
//            snprintf(
//                (char *) transactionDataKey + position,
//                sizeof(transactionDataKey) - position,
//                " %02d/%02d",
//                transactionChunksPageIndex + 1,
//                transactionChunksPageCount);
//        }
//
//        if (scrolling_mode == KEY_SCROLLING_NO_VALUE) {
//            crop_transaction_key();
//            scrolling_mode = VALUE_SCROLLING;
//        }
//        else if (scrolling_mode == PENDING) {
//            if (strlen((char *) transactionDataKey) > MAX_SCREEN_LINE_WIDTH) {
//                int value_length = strlen((char *) transactionDataValue);
//                if (value_length > MAX_SCREEN_LINE_WIDTH) {
//                    strcpy((char *) transactionDataValue, "DBL-CLICK FOR VALUE");
//                    scrolling_mode = KEY_SCROLLING_NO_VALUE;
//                } else {
//                    scrolling_mode = KEY_SCROLLING_SHORT_VALUE;
//                }
//            } else {
//                scrolling_mode = VALUE_SCROLLING;
//            }
//        }
//    }
//
//    switch (current_sigtype) {
//        case ED25519:
//            snprintf(
//                    (char *) pageInfo,
//                    sizeof(pageInfo),
//                    "ED25519 - %02d/%02d",
//                    transactionDetailsCurrentPage + 1,
//                    transactionDetailsPageCount);
//            break;
//    }
//}

//void view_sign_transaction(unsigned int unused) {
//    UNUSED(unused);
//
//    if (event_handler_sign_transaction != NULL) {
//        event_handler_sign_transaction();
//    } else {
//        UX_DISPLAY(bagl_ui_sign_transaction, NULL);
//    }
//}
//
//void reject(unsigned int unused) {
//    if (event_handler_reject_transaction != NULL) {
//        event_handler_reject_transaction();
//    }
//}

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

void view_init() {
    UX_INIT();
    view_uiState = UI_MAIN_MENU;
}

void view_display_main_menu() {
    view_uiState = UI_MAIN_MENU;
    UX_MENU_DISPLAY(0, menu_main, NULL);
}

void view_display_validation_init() {

    view_uiState = UI_VALIDAITON_INIT;
    UX_DISPLAY(bagl_ui_initialize_transaction, NULL);
}

void view_display_validation_processing() {

    view_uiState = UI_VALIDATION_PROCESSING;
    UX_DISPLAY(bagl_ui_validating_transaction, NULL);
}

void view_set_height(int64_t height)
{
    data_height = height;
    snprintf((char*)view_data_height, MAX_SCREEN_LINE_WIDTH, "Height: %d\n", (int)height);
    snprintf((char*)view_data_height2, MAX_SCREEN_LINE_WIDTH, "Height: %d\n", (int)height);
}

void view_set_round(int8_t round)
{
    data_round = round;
    snprintf((char*)view_data_round, MAX_SCREEN_LINE_WIDTH, "Round: %d\n", round);
}

void view_set_pubic_key(const char* publicKey)
{
    snprintf((char*)view_data_publicKey, MAX_SCREEN_LINE_WIDTH, "PK: %s\n", publicKey);
}