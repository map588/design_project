#ifndef DISPLAY_FUNCTIONS_H
#define DISPLAY_FUNCTIONS_H

#include "display_manager.h"

inline static void countdown_to_start()
{
    if (!fired)
    {
        clearflags();
        Paint_SelectImage((UBYTE *)s_buffer);
        Paint_Clear(BLACK);
        LCD_2IN_Display((UBYTE *)s_buffer);
        fired = true;
    }

    // draw the bomb here, then do the switch case
    // move the countdown to the side, bomb on the right

    switch (index)
    { 
    case 0:
      enclosure(-1,0);
      break;
    case 2: //This is intentional, I want the bomb to be drawn on the screen for 2 frames
      Paint_DrawString_EN(75, 83, "3", &Font20, GREEN, BLACK);
      drive_hex(3);
      tone(&tone_gen, NOTE_GS3, 100);
      break;
    case 3:
      Paint_DrawString_EN(75, 83, "2", &Font20, GREEN, BLACK);
      drive_hex(2);
      tone(&tone_gen, NOTE_GS3, 100);

      break;
    case 4:
      Paint_DrawString_EN(75, 83, "1", &Font20, GREEN, BLACK);
      drive_hex(1);
      tone(&tone_gen, NOTE_GS3, 100);
      break;
    case 5:
      Paint_DrawString_EN(3, 83, "DEFUSE IT!", &Font20, RED, BLACK);
      drive_hex(0);
      tone(&tone_gen, NOTE_FS4, 200);
      break;
    case 6:
      clear_hex();
      index = 10;
    default:
      break;
  }
  LCD_2IN_Display((UBYTE *)s_buffer);
  return;
}

static void __time_critical_func(loading_bar)()
{
  uint16_t x1;
  uint16_t x2;
  uint16_t y1;
  uint16_t y2;

  y1 = 225;
  y2 = 235;
  Paint_SelectImage((UBYTE *)s_buffer);

  if (!fired && !load_state){
        clearflags();
        load_state = true;
        Paint_Clear(BLACK);
        LCD_2IN_Display((UBYTE *)s_buffer);
  } 

  if (index == 0){
      Paint_ClearWindows(205, y1 - 2, 319, y2 + 2, BLACK);
      x1 = 220;
      x2 = 225;

    for(int i = 1; i <= 9; i++){
          x1 = 220 + 10 * (i);
          x2 = 220 + 10 * (i + 1) - 5;
          Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
    }

    LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
  }
  else {
      Paint_ClearWindows(205, 223, 215 + 10 * (index + 1) + 3 , y2 + 2, BLACK);
  }
      
    if(index >= 9){
        LCD_2IN_DisplayWindows(205, y1 - 2, 319, y2 + 2, s_buffer);
        Paint_ClearWindows(205, 223, 319, 239, BLACK);
    }
      
      blink_led();
      tone(&tone_gen, NOTE_C3, value / 20);

      LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
}

inline static void populate_UI_elements(){
 
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 29, BLACK);   // top bar
  Paint_ClearWindows(0, 220, 120, 239, BLACK); // approximately time window
  uint8_t round = score_d / 20;
  uint8_t n_round = 20 - (score_d % 20);

  char score_str[20]; 
  char round_str[20]; 
  char nextR_str[20]; 
  char  time_str[20];  

  sprintf(score_str, "SCORE: %u", score_d);
  sprintf(round_str, "ROUND: %u",  round);
  sprintf(nextR_str, "NEXT ROUND: %u", n_round);
  sprintf(time_str,  "TIME: %u ms", value);

  
  Paint_DrawString_EN(2,   2, score_str, &Font12,  GREEN, BLACK);
  Paint_DrawString_EN(2,  16, nextR_str, &Font12, YELLOW, BLACK);
  Paint_DrawString_EN(2, 226,  time_str, &Font12,   CYAN, BLACK);
  Paint_DrawString_EN(260, 2, round_str, &Font12,    RED, BLACK);

  
  LCD_2IN_DisplayWindows(0, 0, 30, 319, s_buffer);
  LCD_2IN_DisplayWindows(224, 0, 239, 100, s_buffer);
}


inline static void write_prompt(){     //BEING CHANGED
  const char *prompt_str[3] = {"TURN IT", "YANK IT", "WIRE IT"};
  const uint16_t colors[3] = {RED, BLUE, GREEN};
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(6, 59, 145, 111, BLACK); // rough prompt window

  //draw the bomb graphic on screen
  enclosure(action, 0);
 
  //writes in the action to be done and the rectangle encompassing the string
  
  //dotted lines from corners of graphic display to corners of prompt window
  Paint_DrawLine(160, 62, 7, 42, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(266, 62, 111, 42, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(160, 108, 7, 88, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(266, 108, 111, 88, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);

 
  Paint_DrawRectangle(7, 42, 111, 88, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(7, 42, 111, 88, colors[action], DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  Paint_DrawString_EN(10, 55, prompt_str[action], &Font20, colors[action], BLACK);

  //partial update which updates the prompt specifically
  //need to move this and the prompt itself off to the side
  //TODO: changed the dimensions of this partial update
  LCD_2IN_DisplayWindows(59, 6, 111, 145, s_buffer);
}


inline static void correct_disp(){
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_Clear(BLACK);
  //need to add in the bomb graphic
  enclosure(action, 1);
  
  //TODO: I moved both switch statements into enclosure, and the boolean determines whether it should call the green or red setting


  //need to shift this string to the left side
  Paint_DrawString_EN(12, 77, "CORRECT", &Font20, GREEN, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
}

inline static void incorrect_disp(){  //BEING CHANGED
  if(!fired || !incorrect_state){
    clearflags();
    incorrect_state = true;
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_Clear(BLACK);
    Paint_DrawString_EN (110, 100, "BOOM", &Font24, RED, BLACK);
    LCD_2IN_Display((UBYTE *)s_buffer);
  }

//add multiple explosion graphics in the form of concentric circles
//use a switch case depending on index
//start at case index=9, work down to 0
//no break statements

//ZONE OF EXCLUSION FOR EXPLOSIONS
//X COORDS: 110 TO 197
//Y COORDS: 100 TO 120
//Assuming the radfactor is no greater than 1:
//x_cen can not be between 62 and 245
//x_cen can not be less than 49 or greater than 272
//y_cen can not be between 52 and 168
//y_cen can not be less than 48 or greater than 192
//center coordinates in that range will draw over the text
switch(index){
  case 9:
    //in each case, add an explosion_draw
    //need to figure out coordinates for each explosion, as well as radius factor
    //gonna just guess the coords and not change radius factor for now
    multicore_lockout_end_blocking();
    break;
  case 8:
    explosion_draw(262, 57, 1);
    explosion_draw(134, 40, 1);
    break;
  case 7:
    explosion_draw(132, 186, 1);
    break;
  case 6:
    explosion_draw(265, 183, 1);
    break;
  case 5:
    explosion_draw(56, 113, 1);
    break;
  case 4:
    explosion_draw(68, 51, 1);
    break;
  case 3:
    explosion_draw(218, 169, 1);
    break;
  case 2:
    explosion_draw(205, 49, 1);
    break;
  case 1:
    explosion_draw(77, 174, 1);
    multicore_lockout_start_blocking();
    break;
  case 0:
    explosion_draw(268, 108, 1);
    break;
  default:
  break;
}

//LCD_2IN_Display((UBYTE *)s_buffer);
}

// inline static void play_again(){
// if (!fired)
// {
//   clearflags();
//   restart_state = true;
//   Paint_SelectImage((UBYTE *)s_buffer);
//   Paint_DrawString_EN(110, 100, "AGAIN?", &Font20, GREEN, BLACK);
//   LCD_2IN_Display((UBYTE *)s_buffer);
//   fired = true;
// }
//   loading_bar();
// }


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

    drive_hex(9 - index);
    //tone(&tone_gen, NOTE_C3, value / 50);
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
    if (!enabled){
     if (character == 'e'){
            enabled = !enabled;
            *key_lock = true;
            return;
     }else
       *key_lock = false;
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
    case 'e':
        enabled = !enabled;
        *key_lock = false;
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
            else{
                // printf("Calculator mode engaged.\n");
                Paint_DrawString_EN(5, 5, "Calculator mode engaged.", key_text.font_size, key_text.color, key_text.background);
                melody(&tone_gen, REJECT, 1);
            }
        }
    }

    Paint_ClearWindows(0, 0, 320, 18, BLACK);
    Paint_DrawString_EN(5, 5, str_buffer, &Font16, WHITE, BLACK);
    LCD_2IN_Display((uint8_t *)s_buffer);
}

inline static void random_key(){
    if (!fired){
        clearflags();
        Paint_SelectImage((UBYTE *)s_buffer);
        Paint_Clear(BLACK);
        LCD_2IN_Display((UBYTE *)s_buffer);
        fired = true;
    }
    char random_prompt[20];
    sprintf(random_prompt, "%u", (char)value);
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(0, 0, 320, 18, BLACK);
    Paint_DrawString_EN(68, 108, random_prompt, &Font24, WHITE, BLACK);
    LCD_2IN_Display((uint8_t *)s_buffer);
}

#endif // !DISPLAY_MANAGER_H