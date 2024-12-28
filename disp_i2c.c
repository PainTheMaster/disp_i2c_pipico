#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define SSD1306ADDR 0x3C

void ssd1306_init(i2c_inst_t* i2c, uint8_t i2cAddr);
void ssd1306_dot(i2c_inst_t* i2c, uint8_t i2cAddr, uint8_t i, uint8_t j);
void ssd1306_clear();
void ssd1306_output(i2c_inst_t* i2c, uint8_t i2cAddr);
void cursor(const int row, const int col);
void decursor(const int row, const int col);
void charbitmap(const uint8_t code, const int row, const int col);
int print_str(int *row_start, int *col_start, const uint8_t *str, i2c_inst_t* i2c, uint8_t i2cAddr);


uint8_t cambus[64][128];

int main()
{
    stdio_init_all();


    sleep_ms(5000);
    printf("board init!\n");

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

        printf("Hello, world!\n");
        sleep_ms(1000);

    ssd1306_init(I2C_PORT, SSD1306ADDR);

        printf("init done!\n");
        sleep_ms(1000);

    int i, j;
    for(i=0; i<=63; i++){
        for(j=0; j<=63; j++){
            cambus[i][j] = 1;
        }
        for(j=64; j<=127; j++){
            cambus[i][j] = 0;
        }
    }

    printf("cambus done\n");
    sleep_ms(1000);

    ssd1306_output(I2C_PORT, SSD1306ADDR);
    printf("output done\n");
    sleep_ms(1000);

    ssd1306_clear();
//    circle(20, 32, 63);
    ssd1306_output(I2C_PORT, SSD1306ADDR);
    sleep_ms(2000);

    while(1) {
        ssd1306_clear();
        for(i=0; i<=3; i++){
            decursor(0, 0);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(200);
            cursor(0, 0);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(200);
        }
        

        int row_char, col_char, row_curs, col_curs;
        for (i=0x20; i<=0x7e; i++){
            row_curs = ((i+1-0x20)/16)*8;
            col_curs = ((i+1-0x20)%16)*8;        
            row_char = ((i-0x20)/16)*8;
            col_char = ((i-0x20)%16)*8;
            cursor(row_curs, col_curs);
            charbitmap(i, row_char, col_char);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(5);
            decursor(row_curs, col_curs);
        }


        for(i=0; i<= 9; i++){
            cursor(row_curs, col_curs);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(200);
            decursor(row_curs, col_curs);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(200);
        }

        ssd1306_clear();
        ssd1306_output(I2C_PORT, SSD1306ADDR);
        sleep_ms(1000);
        //ここまで移植した

        row_curs = 0;
        col_curs = 0;
        uint8_t msg[] = "Powered by\nKen Yamashita\nsample@email.com";
        print_str(&row_curs, &col_curs, msg, I2C_PORT, SSD1306ADDR);

        for(i=0; i<= 9; i++){
            cursor(row_curs, col_curs);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(200);
            decursor(row_curs, col_curs);
            ssd1306_output(I2C_PORT, SSD1306ADDR);
            sleep_ms(200);
        }
    }

/*
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
*/
}

void ssd1306_init(i2c_inst_t* i2c, uint8_t i2cAddr){
    const uint8_t init_ctrl_cmd = 0x00;
    const uint8_t init_ctrl_data = 0x40;

    uint8_t buf_cmd[16];

    buf_cmd[0] = init_ctrl_cmd;
    buf_cmd[1] = 0xae;  //off
    buf_cmd[2] = 0xa6;  //normal display
    buf_cmd[3] = 0x2e;  //deactivate scroll
    buf_cmd[4] = 0x20;  //set memory addressing mode
    buf_cmd[5] = 0x02;  //memory addressing mode = page addressing mode
    buf_cmd[6] = 0x00;  buf_cmd[7] = 0x10;  //column start address 0
    buf_cmd[8] = 0x40;  //display RAM stat line 0
    buf_cmd[9] = 0xa0;  //column address 0 is mapped to SEG0
    buf_cmd[10] = 0xa8; buf_cmd[11] = 0x3f; //multiplex ratio 63+1=64
    buf_cmd[12] = 0xa4; //GDDRAM
    buf_cmd[13] = 0x8d; buf_cmd[14] = 0x14; //charge pump enable 
    buf_cmd[15] = 0xaf;  //on
    i2c_write_blocking(i2c, i2cAddr, buf_cmd, 16, false);
    sleep_ms(100);
}

void ssd1306_dot(i2c_inst_t* i2c, uint8_t i2cAddr, uint8_t i, uint8_t j){
    const uint8_t dot_ctrl_cmd = 0x00;
    const uint8_t dot_ctrl_data = 0x40;
    const uint8_t dot_cmd_page = 0xb0;

    uint8_t page = i/8;
    uint8_t common = i%8;

    uint8_t dot_buf_cmd[4];
    dot_buf_cmd[0]=dot_ctrl_cmd;
    dot_buf_cmd[1]=0xb0+page;
    dot_buf_cmd[2]=0x00 + (j & 0x0f); 
    dot_buf_cmd[3]=0x10 + (j >> 4); 
    i2c_write_blocking(i2c, i2cAddr, dot_buf_cmd, 4, false);

    uint8_t dot_buf_disp[2];
    dot_buf_disp[0] = dot_ctrl_data;
    dot_buf_disp[1] = (uint8_t)(1)<<(common);
    i2c_write_blocking(i2c, i2cAddr, dot_buf_disp, 2, false); 
}

void ssd1306_clear(){
    int i, j;
    for (i=0; i<=63; i++){
        for(j=0; j <=127; j++){
            cambus[i][j] = 0;
        }
    }
}

void ssd1306_output(i2c_inst_t* i2c, uint8_t i2cAddr){
    uint8_t buf_out[8][129];
    uint8_t buf_cmd[3], ret_cmd[3];
    int i_cam, j_cam;
    int page, i_page, seg;

    for (page=0; page <= 7; page++){
        buf_out[page][0]=0x40; //graphic data tag
        for(seg=1; seg<=128; seg++){
            buf_out[page][seg]=0;
        }
    }
    for (i_cam=0; i_cam <= 63; i_cam++){
        page = i_cam/8;
        i_page = i_cam % 8;
        for (j_cam=0; j_cam <= 127; j_cam++){
            seg = j_cam+1;
            buf_out[page][seg] |= (cambus[i_cam][j_cam]) << i_page; 
        }
    }
    

    ret_cmd[0] = 0x00;//cmd tag
    ret_cmd[1] = 0x00;//start segment nibble
    ret_cmd[2] = 0x10;//start segment nibble

    buf_cmd[0] = 0x00; //cmd tag

    
    for (page=0; page <=7; page++){
        i2c_write_blocking(i2c, i2cAddr, ret_cmd, 3, false);         
        buf_cmd[1] = 0xb0 | ((uint8_t)(page));
        i2c_write_blocking(i2c, i2cAddr, buf_cmd, 2, false); 
        i2c_write_blocking(i2c, i2cAddr, buf_out[page], 129, false); 
    }
}

void cursor(const int row, const int col){
    int i, j;
    for(j=0; j <=6 && col+j <= 127; j++){
        for(i=0; i<=6 && row+i <=63; i++){
            cambus[row+i][col+j]=1;
        }
        cambus[row+7][col+j] = 0;
    }
    j=7;
    for(i=0; i<=7 && row+i <= 63 && col+j<=127; i++){
        cambus[row+i][col+j]=0;
    }
}

void decursor(const int row, const int col){
    int i, j;
    for(j=0; j <=7 && col+j <= 127; j++){
        for(i=0; i<=7 && row+i <=63; i++){
            cambus[row+i][col+j]=0;
        }
    }
}

void charbitmap(const uint8_t code, const int row, const int col){
    static const uint8_t bitmap[128][8] = {
        {}, //0x00, ''
        {}, //0x01, ''
        {}, //0x02, ''
        {}, //0x03, ''
        {}, //0x04, ''
        {}, //0x05, ''
        {}, //0x06, ''
        {}, //0x07, ''
        {}, //0x08, ''
        {}, //0x09, ''
        {}, //0x0a, ''
        {}, //0x0b, ''
        {}, //0x0c, ''
        {}, //0x0d, ''
        {}, //0x0e, ''
        {}, //0x0f, ''
        {}, //0x10, ''
        {}, //0x11, ''
        {}, //0x12, ''
        {}, //0x13, ''
        {}, //0x14, ''
        {}, //0x15, ''
        {}, //0x16, ''
        {}, //0x17, ''
        {}, //0x18, ''
        {}, //0x19, ''
        {}, //0x1a, ''
        {}, //0x1b, ''
        {}, //0x1c, ''
        {}, //0x1d, ''
        {}, //0x1e, ''
        {}, //0x1f, ''
        {0, 0, 0, 0, 0, 0, 0, 0}, //0x20, '' ' ' (white space)
        {0, 0, 8, 92, 92, 8, 0, 0}, //0x21, '!'
        {0, 2, 14, 0, 2, 14, 0, 0}, //0x22, '"'
        {20, 127, 127, 20, 127, 127, 20, 0}, //0x23, '#'
        {36, 46, 42, 127, 42, 58, 16, 0}, //0x24, '$'
        {64, 108, 58, 30, 108, 86, 50, 0}, //0x25, '%'
        {48, 122, 77, 77, 127, 50, 80, 0}, //0x26, '&'
        {0, 0, 0, 5, 3, 0, 0, 0}, //0x27, '''
        {0, 0, 28, 62, 99, 65, 0, 0}, //0x28, '('
        {0, 0, 65, 99, 62, 28, 0, 0}, //0x29, ')'
        {8, 42, 62, 28, 28, 62, 42, 8}, //0x2a, '*'
        {0, 8, 8, 62, 62, 8, 8, 0}, //0x2b, '+'
        {0, 0, 0, 160, 96, 0, 0, 0}, //0x2c, ','
        {0, 8, 8, 8, 8, 8, 8, 0}, //0x2d, '-'
        {0, 0, 0, 64, 64, 0, 0, 0}, //0x2e, '.'
        {64, 96, 48, 24, 12, 6, 3, 1}, //0x2f, '/'
        {62, 127, 121, 77, 71, 127, 62, 0}, //0x30, '0'
        {0, 68, 70, 127, 127, 64, 64, 0}, //0x31, '1'
        {98, 115, 81, 89, 73, 111, 102, 0}, //0x32, '2'
        {34, 99, 73, 73, 73, 127, 54, 0}, //0x33, '3'
        {24, 28, 22, 83, 127, 127, 80, 0}, //0x34, '4'
        {39, 103, 69, 69, 69, 125, 57, 0}, //0x35, '5'
        {62, 127, 73, 73, 73, 123, 50, 0}, //0x36, '6'
        {3, 3, 113, 121, 13, 7, 3, 0}, //0x37, '7'
        {54, 127, 73, 73, 73, 127, 54, 0}, //0x38, '8'
        {38, 111, 73, 73, 73, 127, 62, 0}, //0x39, '9'
        {0, 0, 0, 34, 34, 0, 0, 0}, //0x3a, ':'
        {0, 0, 0, 162, 98, 0, 0, 0}, //0x3b, ';'
        {0, 0, 8, 28, 54, 99, 65, 0}, //0x3c, '<'
        {0, 36, 36, 36, 36, 36, 36, 0}, //0x3d, '='
        {0, 65, 99, 54, 28, 8, 0, 0}, //0x3e, '>'
        {0, 2, 3, 81, 89, 15, 6, 0}, //0x3f, '?'
        {62, 65, 73, 85, 85, 93, 30, 0}, //0x40, '@'
        {126, 127, 9, 9, 9, 127, 126, 0}, //0x41, 'A'
        {65, 127, 127, 73, 73, 127, 54, 0}, //0x42, 'B'
        {62, 127, 65, 65, 65, 99, 34, 0}, //0x43, 'C'
        {65, 127, 127, 65, 65, 127, 62, 0}, //0x44, 'D'
        {65, 127, 127, 73, 93, 65, 99, 0}, //0x45, 'E'
        {65, 127, 127, 73, 29, 1, 3, 0}, //0x46, 'F'
        {62, 127, 65, 65, 81, 119, 118, 0}, //0x47, 'G'
        {127, 127, 8, 8, 8, 127, 127, 0}, //0x48, 'H'
        {0, 0, 65, 127, 127, 65, 0, 0}, //0x49, 'I'
        {48, 112, 64, 65, 127, 63, 1, 0}, //0x4a, 'J'
        {65, 127, 127, 8, 28, 119, 99, 0}, //0x4b, 'K'
        {65, 127, 127, 65, 64, 96, 112, 0}, //0x4c, 'L'
        {127, 126, 12, 24, 12, 126, 127, 0}, //0x4d, 'M'
        {127, 127, 6, 12, 24, 127, 127, 0}, //0x4e, 'N'
        {62, 127, 65, 65, 65, 127, 62, 0}, //0x4f, 'O'
        {65, 127, 127, 73, 9, 15, 6, 0}, //0x50, 'P'
        {62, 127, 65, 113, 97, 255, 190, 0}, //0x51, 'Q'
        {65, 127, 127, 9, 9, 127, 118, 0}, //0x52, 'R'
        {38, 111, 73, 73, 73, 123, 50, 0}, //0x53, 'S'
        {0, 7, 65, 127, 127, 65, 7, 0}, //0x54, 'T'
        {63, 127, 64, 64, 64, 127, 63, 0}, //0x55, 'U'
        {15, 31, 48, 96, 48, 31, 15, 0}, //0x56, 'V'
        {127, 63, 24, 12, 24, 63, 127, 0}, //0x57, 'W'
        {65, 99, 62, 28, 62, 99, 65, 0}, //0x58, 'X'
        {0, 7, 79, 120, 120, 79, 7, 0}, //0x59, 'Y'
        {71, 99, 113, 89, 77, 103, 115, 0}, //0x5a, 'Z'
        {0, 0, 127, 127, 65, 65, 0, 0}, //0x5b, '['
        {1, 3, 6, 12, 24, 48, 96, 0}, //0x5c, '\'
        {0, 0, 65, 65, 127, 127, 0, 0}, //0x5d, ']'
        {8, 12, 6, 3, 6, 12, 8, 0}, //0x5e, '^'
        {128, 128, 128, 128, 128, 128, 128, 0}, //0x5f, '_'
        {0, 0, 0, 3, 5, 0, 0, 0}, //0x60, '''
        {32, 116, 84, 84, 60, 120, 64, 0}, //0x61, 'a'
        {1, 127, 127, 72, 72, 120, 48, 0}, //0x62, 'b'
        {56, 124, 68, 68, 68, 108, 40, 0}, //0x63, 'c'
        {48, 120, 72, 73, 63, 127, 64, 0}, //0x64, 'd'
        {56, 124, 84, 84, 84, 92, 24, 0}, //0x65, 'e'
        {0, 72, 126, 127, 73, 3, 2, 0}, //0x66, 'f'
        {12, 94, 82, 82, 124, 62, 2, 0}, //0x67, 'g'
        {65, 127, 127, 8, 4, 124, 120, 0}, //0x68, 'h'
        {0, 0, 68, 125, 125, 64, 0, 0}, //0x69, 'i'
        {0, 64, 192, 128, 136, 250, 122, 0}, //0x6a, 'j'
        {65, 127, 127, 16, 56, 108, 68, 0}, //0x6b, 'k'
        {0, 0, 65, 127, 127, 64, 0, 0}, //0x6c, 'l'
        {124, 124, 8, 120, 12, 124, 112, 0}, //0x6d, 'm'
        {4, 124, 120, 4, 4, 124, 120, 0}, //0x6e, 'n'
        {56, 124, 68, 68, 68, 124, 56, 0}, //0x6f, 'o'
        {132, 252, 248, 164, 36, 60, 24, 0}, //0x70, 'p'
        {24, 60, 36, 164, 252, 252, 128, 0}, //0x71, 'q'
        {68, 124, 120, 76, 4, 12, 12, 0}, //0x72, 'r'
        {8, 92, 84, 84, 84, 116, 32, 0}, //0x73, 's'
        {4, 4, 62, 127, 68, 36, 0, 0}, //0x74, 't'
        {60, 124, 64, 64, 60, 124, 64, 0}, //0x75, 'u'
        {12, 28, 48, 96, 48, 28, 12, 0}, //0x76, 'v'
        {60, 124, 96, 56, 96, 124, 60, 0}, //0x77, 'w'
        {68, 108, 56, 16, 56, 108, 68, 0}, //0x78, 'x'
        {156, 188, 160, 160, 252, 124, 0, 0}, //0x79, 'y'
        {0, 76, 100, 116, 92, 76, 100, 0}, //0x7a, 'z'
        {0, 0, 8, 62, 119, 65, 65, 0}, //0x7b, '{'
        {0, 0, 0, 119, 119, 0, 0, 0}, //0x7c, '|'
        {0, 65, 65, 119, 62, 8, 0, 0}, //0x7d, '}'
        {2, 3, 1, 3, 2, 3, 1, 0}, //0x7e, '~'
        {} //0x7f, (DEL)
    };

    int i, j;
    for (j=0; j <=7 && col+j <=127; j++){
        for (i=0; i <=7 && row+i <= 63; i++){
            cambus[row+i][col+j]=(bitmap[code][j]>>i)&((uint8_t)(1));
        }
    }
}


int print_str(int *row_start, int *col_start, const uint8_t *str, i2c_inst_t* i2c, uint8_t i2cAddr){
    int i, row_cursor, col_cursor;
    row_cursor = *row_start;
    col_cursor = *col_start;

    for(i=0; row_cursor <=63 && str[i] != '\0'; i++){
        if(str[i]=='\n'){
            decursor(row_cursor, col_cursor);
            row_cursor +=8;
            col_cursor = 0;
        }else{
            charbitmap(str[i], row_cursor, col_cursor);

            col_cursor +=8;
        }
        if(col_cursor > 127){
            col_cursor = 0;
            row_cursor += 8;
        }
        cursor(row_cursor, col_cursor);
        ssd1306_output(i2c, i2cAddr);
        sleep_ms(5);
    }

    *row_start = row_cursor;
    *col_start = col_cursor;

    return i;
}