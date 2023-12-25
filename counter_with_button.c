#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static struct termios init_setting, new_setting;
char seg_num[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};
char seg_dnum[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x58, 0x00, 0x10};

#define D1 0x08
#define D2 0x04
#define D3 0x02
#define D4 0x01

int seg2dec(x){
    if (x == 0xc0)
    return 0;
    if (x == 0xf9)
    return 1;
    if (x == 0xa4)
    return 2;
    if (x == 0xb0)
    return 3;
    if (x == 0x99)
    return 4;
    if (x == 0x92)
    return 5;
    if (x == 0x82)
    return 6;
    if (x == 0xd8)
    return 7;
    if (x == 0x80)
    return 8;
    if (x == 0x90)
    return 9;
}


void init_keyboard() 
{
    tcgetattr(STDIN_FILENO, &init_setting);
    new_setting = init_setting;
    new_setting.c_lflag &= ~ICANON;
    new_setting.c_lflag &= ~ECHO;
    new_setting.c_cc[VMIN] = 0;
    new_setting.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_setting);
}

void close_keyboard() 
{
    tcsetattr(0, TCSANOW, &init_setting);
}

char get_key() 
{
    char ch = -1;

    if(read(STDIN_FILENO, &ch, 1) != 1)
        ch = -1;
    return ch;
}

void print_menu() 
{
    printf("\n----------menu----------\n");
    printf("[u] : count up\n");
    printf("[d] : count down\n");
    printf("[p] : count setting\n");
    printf("[q] : program quit\n");
    printf("------------------------\n\n");
}

int main(int argc, char **argv) 
{
    unsigned short data[4];
    char key;
    int tmp_n;
    int delay_time;
    char buff[2]; 
    char temp[2] = {'0',};

    int dev = open("/dev/my_segment", O_RDWR);

    if (dev == -1) {
        printf("Opening was not possible! \n");
        return -1;
    }
    printf("device opening was successful! \n");

    int dev2 = open("/dev/my_gpio", O_RDWR);

    if (dev2 == -1) {
        printf("Opening was not possible! \n");
        return -1;
    }
    printf("device opening was successful! \n");


    init_keyboard();
    print_menu(); 
    tmp_n = 0;
    delay_time = 5000; 

    data[0] = (seg_num[0] << 4) | D1;
    data[1] = (seg_num[0] << 4) | D2;
    data[2] = (seg_num[0] << 4) | D3;
    data[3] = (seg_num[0] << 4) | D4;

    while (1) {
        key = get_key();

        temp[0] = buff[0];
        temp[1] = buff[1];
        read(dev2, &buff, 2);
                 
        if (key == 'q') {
            printf("Exit this program.\n"); 
            break;
        } 
        else if (key == 'p') { 
            data[0] = (seg_num[0] << 4) | D1;
            data[1] = (seg_num[0] << 4) | D2;
            data[2] = (seg_num[0] << 4) | D3;
            data[3] = (seg_num[0] << 4) | D4;
        }
        else if ((key == 'u') || (temp[0] == '0' && buff[0] == '1')) { 
            int temp = 0;
            temp = seg2dec((data[0] >> 4)) +
                   seg2dec((data[1] >> 4))*10 +
                   seg2dec((data[2] >> 4))*100 +
                   seg2dec((data[3] >> 4))*1000;

            temp++;
            if (temp > 9999){
                temp = 0;
            }

            data[0] = (seg_num[temp % 10] << 4) | D1;
            temp /= 10;
            data[1] = (seg_num[temp % 10] << 4) | D2;
            temp /= 10;
            data[2] = (seg_num[temp % 10] << 4) | D3;
            temp /= 10;
            data[3] = (seg_num[temp % 10] << 4) | D4;
        }
        else if ((key == 'd') || (temp[1] == '0' && buff[1] == '1')) { 
            int temp = 0;
            temp = seg2dec((data[0] >> 4)) +
                   seg2dec((data[1] >> 4))*10 +
                   seg2dec((data[2] >> 4))*100 +
                   seg2dec((data[3] >> 4))*1000;

            temp--;
            if (temp < 0){
                temp = 9999;
            }

            data[0] = (seg_num[temp % 10] << 4) | D1;
            temp /= 10;
            data[1] = (seg_num[temp % 10] << 4) | D2;
            temp /= 10;
            data[2] = (seg_num[temp % 10] << 4) | D3;
            temp /= 10;
            data[3] = (seg_num[temp % 10] << 4) | D4;
        }

        write(dev, &data[tmp_n], 2);
        usleep(delay_time);

        tmp_n++;
        if (tmp_n > 3) {
            tmp_n = 0;
        }
    }

    close_keyboard(); 
    write(dev, 0x0000, 2); 
    close(dev);
    close(dev2);
    return 0;
}
