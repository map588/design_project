#ifndef DISPLAY_FUNCTIONS_H
#define DISPLAY_FUNCTIONS_H

#include "display_manager.h"



inline static void game_UI()
{
    Paint_SelectImage((UBYTE *)s_buffer);

    if (!fired)
    {
        clearflags();
        game_state = true;
        Paint_Clear(BLACK);
        write_prompt(action);
        populate_UI_elements(value, score_d);
        LCD_2IN_Display((UBYTE *)s_buffer);
        fired = true;
    }

    // drive_hex(9 - index);
    tone(&tone_gen, NOTE_C3, value / 50);
    loading_bar();
}

inline static void display_key()
{
    static bool calculator_mode = false;
    static int str_idx = 0;
    static char str_buffer[1024];
    static int calc_idx = 0;
    static int base = 10;

    // if(str_idx >= 1024){
    //   printf("Buffer full, hit delete or numlock->insert to clear\n");
    //   return;
    // }
    char character = (char)score_d;
    if (!enabled)
    {
        if (character == 'e')
            enabled = !enabled;

        return;
    }

    long long op1;
    char *endptr = str_buffer;
    char operand;
    long long op2;
    long long result;

    if (!key_state)
    {
        Paint_Clear(BLACK);
        key_state = true;
    }

    Paint_SelectImage((uint8_t *)s_buffer);

    switch (character)
    {
    case '\b':
        if (str_idx > 0)
        {
            str_buffer[str_idx - 1] = '\0';
            str_idx--;
        }
        break;
    case 0xAB:
        character = '<';
        goto put_char;
    case 0xBB:
        character = '>';
        goto put_char;
    case 'c':
        str_buffer[0] = '\0';
        str_idx = 0;
        break;
    case '<':
    case '>':
        break;
    case '\n':
        if (!calculator_mode)
        {
            character = '\n';
            goto put_char;
        }
        else
        {
            for (int i = 1; i < str_idx; i++)
            {
                if (str_buffer[str_idx - i] == '\n' || i == str_idx - 1)
                {
                    calc_idx = str_idx - i;
                    break;
                }
            }
            char calc_buffer[512];
            for (int i = 0; i < (str_idx - calc_idx); i++)
            {
                calc_buffer[i] = str_buffer[calc_idx + i];
            }
            op1 = strtoll(calc_buffer, &endptr, base);

            // Change of base
            if (*endptr == '\0' && calc_buffer[0] != '\0' && op1 > 0 && op1 <= 16)
            {
                base = op1;
                char base_str[32];
                sprintf(base_str, "Base changed to %d\n", base);
                Paint_DrawString_EN(5, 5, base_str, key_text.font_size, key_text.color, key_text.background);
            }
            else if (*endptr == '\0')
            {
                Paint_DrawString_EN(5, 5, "Invalid base.", key_text.font_size, key_text.color, key_text.background);
                // printf("Invalid base\n");
            }
            else
            {
                operand = *endptr;

                // accumulate partial results
                while (operand != '\0')
                {
                    op2 = strtoll(endptr + 1, &endptr, base);
                    switch (operand)
                    {
                    case '+':
                        op1 = op1 + op2;
                        break;
                    case '-':
                        op1 = op1 - op2;
                        break;
                    case '*':
                        op1 = op1 * op2;
                        break;
                    case '/':
                        op1 = (op2 == 0) ? 0 : op1 / op2;
                        break;
                    case '%':
                        op1 = (op2 == 0) ? 0 : op1 % op2;
                        break;
                    case '!':
                        op1 = !op2;
                        break;
                    case '&':
                        op1 = op1 & op2;
                        break;
                    case '|':
                        op1 = op1 | op2;
                        break;
                    case '^':
                        // op1 = pow(op1, op2);
                        break;
                    case '#':
                        op1 = op1 ^ op2;
                        break;
                    case '<':
                        op1 = op1 << op2;
                        break;
                    case '>':
                        op1 = op1 >> op2;
                        break;
                    default:
                        break;
                    }
                    operand = *endptr;
                }

                result = op1;
                str_buffer[str_idx] = ' ';
                str_buffer[str_idx + 1] = '=';
                str_buffer[str_idx + 2] = ' ';
                str_buffer[str_idx + 3] = '\0';
                str_idx += 3;
                char result_str[256];
                int result_idx = 0;

                sprintf(result_str, "%lld", result);

                while (result_str[result_idx] != '\0')
                {
                    str_buffer[str_idx] = result_str[result_idx];
                    str_idx++;
                    result_idx++;
                }
                str_buffer[str_idx] = '\n';
                str_buffer[str_idx + 1] = '\0';
                str_idx++;

                endptr = str_buffer + str_idx;
                calc_buffer[0] = '\0';
                break;
            }
        }
    default:
    put_char:
        str_buffer[str_idx] = (char)character;
        str_buffer[str_idx + 1] = '\0';
        str_idx++;
        break;
    }

    // printf("%c%c%c%c", 0x1B, 0x5B, 0x32, 0x4A); //This clears the serial terminal
    // printf("%s    %u %u", str_buffer, str_idx,calc_idx);
    tone(&tone_gen, NOTE_A3, 50);

    if (str_idx >= 5)
    {
        if (str_buffer[str_idx - 5] == '.' && str_buffer[str_idx - 4] == '.' &&
            str_buffer[str_idx - 3] == '.' && str_buffer[str_idx - 2] == '.' &&
            str_buffer[str_idx - 1] == '.')
        {
            calculator_mode = !calculator_mode;
            str_idx = 0;
            str_buffer[str_idx] = '\0';
            if (!calculator_mode)
            {
                // printf("Calculator mode disengaged.\n");
                Paint_DrawString_EN(5, 5, "Calculator mode disengaged.", key_text.font_size, key_text.color, key_text.background);

                melody(&tone_gen, CONFIRM, 1);
            }
            else
            {
                // printf("Calculator mode engaged.\n");
                Paint_DrawString_EN(5, 5, "Calculator mode engaged.", key_text.font_size, key_text.color, key_text.background);
                melody(&tone_gen, REJECT, 1);
            }
        }
    }

    Paint_ClearWindows(0, 0, 320, 18, BLACK);
    Paint_DrawString_EN(5, 5, str_buffer, key_text.font_size, key_text.color, key_text.background);
    LCD_2IN_Display((uint8_t *)s_buffer);
}

// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inlin

#endif // !DISPLAY_MANAGER_H