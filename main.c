/* 
 * File:   main.c
 * Author: zikesjan
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <byteswap.h>
#include <getopt.h>
#include <inttypes.h>
#include "kbd_hw.h"
#include "chmod_lcd.h"
#include "spec.h"

char *mem;

struct dirent **files;
int position = 0;
int count = 0;

char * read_mem(int offset);
void write_mem(char data, int offset);
char RD(char value);
char WR(char value);
char CS0(char value);
char * read_data(char offset);
void write_data(char data, char offset);
/*
void write_sound(unsigned char data);
void write_LED(unsigned char data);
unsigned char * read_LCD_stat();
void write_LCD_inst(unsigned char data);
unsigned char * read_LCD();
void write_LCD(unsigned char data);
unsigned char * read_KBD();
void write_KBD(unsigned char data);
*/
void init_LCD();
void write_LCD(unsigned char *text);
void show_LED(unsigned char value);
void sound();
void debounce(char key, key_ctrl *ctrl);
void press(char key);
void manage_KBD();
boolean check_device(unsigned char *buffer);
uintptr_t find_device();
void init_device(uintptr_t BAR);
void close_device(int status);

/*
 * function reads data from the memory using the given offset
 */
char *read_mem(int offset) {
    char *data = *(mem + offset);
    usleep(RW_TIME);
    return data;
}

/*
 * procedure writes the given data in the memory using the given offset
 */
void write_mem(char data, int offset) {
    *(mem + offset) = data;
    usleep(RW_TIME);
}

char RD(char value) {
    return value^EMUL_BUS_CTRL_RD_m;
}

char WR(char value) {
    return value^EMUL_BUS_CTRL_WR_m;
}

char CS0(char value) {
    return value^EMUL_BUS_CTRL_CS0_m;
}

/*
 * function simulating bus cycle and reading from component with given offset
 * 1. say we want to read
 * 2. confirm the component selection
 * 3. read data
 */
char * read_data(char offset) {
     char tmp = RD(EMUL_BUS_CTRL_INACTIVE_AND_PWR_ON_m | offset);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o); 
    tmp = CS0(tmp);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);
    // read from DATA_IN
    char *data = read_mem(EMUL_BUS_DATA_IN_o);
    tmp = CS0(tmp);
    // write in CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);
    tmp = RD(tmp);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);

    return data;
}

/*
 * procedure simulating the bus cycle and writing given data into memory with the given offset
 * 1. write the data
 * 2. say there are data
 * 3. confirm the component selection
 */
void write_data(char data, char offset) {
    // write on DATA_OUT
    write_mem(data, EMUL_BUS_DATA_OUT_o);
    unsigned char tmp = WR(EMUL_BUS_CTRL_INACTIVE_AND_PWR_ON_m | offset);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);
    tmp = CS0(tmp);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);
    usleep(RW_TIME);

    tmp = CS0(tmp);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);
    tmp = WR(tmp);
    // write on CTRL
    write_mem(tmp, EMUL_BUS_CTRL_o);
}
/*

unsigned char * read_LCD_stat() {
    return read_data(BUS_LCD_STAT_o);
}

unsigned char * read_LCD() {
    return read_data(BUS_LCD_RDATA_o);
}
*/

/*
 * procedure initializing LCD
 * 1. write MOD 2x
 * 2. write CLR
 * 3. write BON | DON
 * all on INST_o with usleep(RW_TIME) 
 */
void init_LCD() {
    write_data(CHMOD_LCD_MOD, BUS_LCD_INST_o);
    usleep(RW_TIME);
    write_data(CHMOD_LCD_MOD, BUS_LCD_INST_o);
    usleep(RW_TIME);
    write_data(CHMOD_LCD_CLR, BUS_LCD_INST_o);
    usleep(RW_TIME);
    write_data(CHMOD_LCD_BON | CHMOD_LCD_DON, BUS_LCD_INST_o);
    usleep(RW_TIME);
}

void write_LCD(char *text) {
    // clear the display
    write_data(CHMOD_LCD_CLR, BUS_LCD_INST_o);
    // set the cursor at the beginning
    write_data(CHMOD_LCD_HOME, BUS_LCD_INST_o);
    int i = 0;
    // while there is some text to be written and there is some space on the LCD
    while ((i < 2 * LCD_LINE_LEN) && (*text != '\0')) {
        if (i == LCD_LINE_LEN) {
            // jump to next line
            write_data(CHMOD_LCD_POS + LCD_NEXT_LINE, BUS_LCD_INST_o);
        }
        i++;
        // if(i >= 2 * LCD_LINE_LEN){writeOnLCDI(CHMOD_LCD_HOME);i=0;}
        // write the text
        write_data(*(text++), BUS_LCD_WDATA_o);
    }
}

/*
 * procedure enabling some LEDs by given value on some predefined time
 */
void show_LED(char value) {
    write_data(value, BUS_LED_WR_o);
    usleep(LED_TIME);
}

/*
 * procedure turning on the peizoelement on some predefined time
 */
void sound() {
    write_data(EMUL_BUS_CTRL_PWR_m, BUS_KBD_WR_o);
    usleep(SOUND_TIME);
    write_data(OFF, BUS_KBD_WR_o);
}

//what to do if key c is considered pressed (or auto-pressed)

void press(char key) {
    sound();
    switch (key) {
        case '1':
        {
            write_LCD("key1"); // test
            //volumeUp();
        }
            break;
        case '2':
        {
            write_LCD("key2"); // test
            //moveUp();
        }
            break;
        case '4':
        {
            write_LCD("key4"); // test
            //playPause();
        }
            break;
        case '5':
        {
            write_LCD("key5"); // test
            //openCurrentFile();
        }
            break;
        case '7':
        {
            write_LCD("key7"); // test
            //volumeDown();
        }
            break;
        case '8':
        {
            write_LCD("key8"); // test
            //moveDown();
        }
            break;
        case '0':
        {
            write_LCD("key0"); // test
        }
        case 'E':
        {
            write_LCD("keyE"); // test
            close_device(EXIT_SUCCESS);
        }
            break;
        case '*':
        {
            write_LCD("key*"); // test
            //quitTotem();
        }
            break;
        case 'B':
        {
            write_LCD("keyB"); // test
            //cd("..");
            //ls();
            //showFirst();
        }
            break;
    }
}

//zakmity a opakovane stisky

void debounce(char key, key_ctrl *ctrl) {
    ctrl->pressed = true;
    if (key == ctrl->last_key) {
        
        printf("test - debouncer if\n");
        
        ctrl->count++;
        if (ctrl->count >= KEY_CHECK_CONST) {
            //klavesa je opravdu stisknuta
            ctrl->count = 0;
            ctrl->repeat_count++;

            //prvni stisk klavesy
            if (ctrl->repeat_count == 1) {
                press(key);
            }//opakovat akci
            else if ((ctrl->repeat_count > KEY_CHECK_CONST+1) && (ctrl->repeat_count % 2 == 0)) {
                press(key);
            }
        }

    } else {
        
        printf("test - debouncer else\n");
        
        ctrl->last_key = key;
        ctrl->count = 0;
        ctrl->repeat_count = 0;
    }
    
}

//which key was pressed?

void manage_KBD() {
    printf("nejvyse1\n");
    key_ctrl ctrl;// maybe global variable
    ctrl.count = 0; // check if it is working
    ctrl.last_key = ' ';
    ctrl.pressed = false;
    ctrl.repeat_count = 0; // 
    printf("nejvyse\n");
    char *buffer = (char *) malloc(sizeof (buffer));
    char key;
    
    // first column - write on KBD
    printf("vyse\n");
    write_data(FIRST_COLUMN, BUS_KBD_WR_o); 
    
    // read form KBD
    printf("tady\n");
    buffer = read_data(BUS_KBD_RD_o);
    printf("tisk\n");
    key = *(buffer) ^0xff;
    printf("sakjdkasj\n");
    if (key != 0) {
        
        printf("test - first column\n");
        
        switch (key) {
            case 1:
            {
                debounce('1', &ctrl);
            }
                break;
            case 2:
            {
                debounce('4', &ctrl);
            }
                break;
            case 4:
            {
                debounce('7', &ctrl);
            }
                break;
            case 8:
            {
                debounce('*', &ctrl);
            }
                break;
            case 16:
            {
                debounce('E', &ctrl);
            }
                break;
        }
    }

    //second column - write on KBD
    write_data(SECOND_COLUMN, BUS_KBD_WR_o);
    // read from KBD
    buffer = read_data(BUS_KBD_RD_o);
    key = *(buffer)^MASK;
    if (key != 0) {
        
        printf("test - 2nd column\n");
        
        switch (key) {
            case 1:
            {
                debounce('2', &ctrl);
            }
                break;
            case 2:
            {
                debounce('5', &ctrl);
            }
                break;
            case 4:
            {
                debounce('8', &ctrl);
            }
                break;
            case 8:
            {
                debounce('0', &ctrl);
            }
                break;
            case 16:
            {
                debounce('B', &ctrl);
            }
                break;
        }
    }

    // third column - write on KBD
    write_data(THIRD_COLUMN, BUS_KBD_WR_o);
    // read from KBD
    buffer = read_data(BUS_KBD_RD_o);
    key = *(buffer)^MASK;
    if (key != 0) {
        
        printf("test - 3rd column\n");
        
        switch (key) {
            case 1:
            {
                debounce('3', &ctrl);
            }
                break;
            case 2:
            {
                debounce('6', &ctrl);
            }
                break;
            case 4:
            {
                debounce('9', &ctrl);
            }
                break;
            case 8:
            {
                debounce('#', &ctrl);
            }
                break;
            case 16:
            {
                fprintf(stdout, "xxx\n");
            }
                break;
        }
    }
    //no key pressed .....................................................maybe not necessary
    if (ctrl.pressed == true) {
        
        printf("test - no key pressed\n");
        
        ctrl.repeat_count = 0;
        ctrl.count--;
        if (ctrl.count < 0) {
            ctrl.count = 0;
        }
    }

}

/*
 * function checks whether content of the buffer matches the vendor-product specification of the desired device
 */
boolean check_device(unsigned char *buffer) {
    int i;
    for (i = 0; i < DEVICE_SPEC_READ_COUNT; i++) {
        if (buffer[i] != device_specification[i]) return false;
    }
    return true;
}

/*
 * procedure locating the device
 * returns the base address register (BAR)
 */
uintptr_t find_device() {

    unsigned char *buffer;
    unsigned char *device_addr;
    uintptr_t device;
    DIR *dirp;
    struct dirent *dptr;
    FILE *file;
    size_t read_size;
    int i;

    DIR *pci;
    struct dirent *pcis;

    // open the directory
    if ((dirp = opendir(PCI_DIR)) == NULL) {
        fprintf(stderr, "Error: Cannot open %s\n", PCI_DIR);
        exit(EXIT_FAILURE);
    }

    // read the directory
    while ((dptr = readdir(dirp)) != NULL) {

        // tmp = PCI_DIRECTORY + "/"
        char tmp[STR_LEN];
        strcpy(tmp, PCI_DIR);
        char *str = strcat(tmp, "/");
        strcpy(tmp, str);

        // path = str + dptr->name
        char *path = strcat(tmp, dptr->d_name);

        // open
        if ((pci = opendir(path)) == NULL) {
            fprintf(stderr, "Error: Cannot open %s\n", path);
            exit(EXIT_FAILURE);
        } else {
            while ((pcis = readdir(pci)) != NULL) {

                // file_path = (path + "/") + pcis->name
                char tmp2[STR_LEN];
                strcpy(tmp2, path);
                char *str2 = strcat(tmp2, "/");
                strcpy(tmp2, str2);
                char *file_path = strcat(tmp2, pcis->d_name);

                if (!(strcmp(dptr->d_name, ".") == 0 || strcmp(dptr->d_name, "..") == 0 || strcmp(pcis->d_name, ".") == 0 || strcmp(pcis->d_name, "..") == 0)) {
                    // open file
                    if ((file = fopen(file_path, "r")) == NULL) {
                        fprintf(stderr, "Error: Cannot open %s\n", file_path);
                        exit(EXIT_FAILURE);
                    } else {

                        // allocate memory for buffer
                        buffer = (unsigned char *) malloc(sizeof (unsigned char) * DEVICE_SPEC_READ_COUNT);
                        if (buffer == NULL) {
                            fprintf(stderr, "Error: Cannot allocate memory\n");
                            exit(EXIT_FAILURE);
                        }

                        // copy the file into the buffer
                        read_size = fread(buffer, 1, DEVICE_SPEC_READ_COUNT, file);
                        if (read_size != DEVICE_SPEC_READ_COUNT) {
                            fprintf(stderr, "Error: Cannot read %s\n", file_path);
                            exit(EXIT_FAILURE);
                        }

                        // device check
                        if (check_device(buffer)) {

                            // allocate new buffer
                            free(buffer);
                            buffer = (unsigned char *) malloc(sizeof (unsigned char) * DEVICE_ADDR_READ_COUNT);
                            if (buffer == NULL) {
                                fprintf(stderr, "Error: Cannot allocate memory\n");
                                exit(EXIT_FAILURE);
                            }

                            // read and get the base address of the device
                            int n = fread(buffer, 1, DEVICE_ADDR_READ_COUNT, file);
                            if (n != DEVICE_ADDR_READ_COUNT) {
                                fprintf(stderr, "Error: Cannot read %s\n", file_path);
                                exit(EXIT_FAILURE);
                            }

                            // allocate memory for device address
                            device_addr = (unsigned char *) malloc(sizeof (unsigned char) * DEVICE_SPEC_READ_COUNT);
                            if (device_addr == NULL) {
                                fprintf(stderr, "Error: Cannot allocate memory\n");
                                exit(EXIT_FAILURE);
                            }

                            device = 0;

                            int j = 0;
                            for (i = DEVICE_ADDR_READ_COUNT - 1; i > (DEVICE_ADDR_READ_COUNT - DEVICE_SPEC_READ_COUNT - 1); i--) {
                                device_addr[j] = buffer[i];
                                buffer[i] &= MASK;
                                //device_address += (buffer[i] * (unsigned long) powl(2, MULTIPLICATION_CONST * j++));
                                j++;
                            }

                            device += (buffer[15] * 16777216);
                            device += (buffer[14] * 65536);
                            device += (buffer[13] * 256);
                            device += buffer[12];

                            device &= MASK2;

                            // closing and releasing
                            free(buffer);
                            fclose(file);
                            close(pcis);
                            closedir(dirp);

                            return device;
                        }
                        free(buffer);
                        fclose(file);
                    }
                }
                close(pcis);
            }
        }
    }
    closedir(dirp);
}

/*
 * procedure initializing the device
 * opens DEV_MEM and maps the memory by using BAR onto pointer mem
 * turns on the device
 */

void init_device(uintptr_t BAR) {
    int fd = open(DEV_MEM, O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open %s\n", DEV_MEM);
        exit(EXIT_FAILURE);
    }

    mem = mmap(NULL, MEM_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BAR);

    write_mem(ON, EMUL_BUS_CTRL_o);
}

/*
 * procedure closing the communication with the device and the whole program
 * the procedure sends the status from its argument to the OS as the result of the program
 */
void close_device(int status) {
    write_mem(OFF, EMUL_BUS_CTRL_o);
    exit(status);
}

/*
 * function main determines the behavior of the program
 */
int main() {

    // get the base address register for the specified device   
    uintptr_t BAR = find_device();

    // initialize the device by using BAR
    init_device(BAR);

    char work_dir[MAXPATHLEN];
    //get current working directory
    if (getcwd(work_dir, MAXPATHLEN) == NULL) {
        fprintf(stderr, "Error: Cannot get path of the working directory\n");
        close_device(EXIT_FAILURE);
    }

    // initialize LCD
    init_LCD();
    
    //close_device(EXIT_SUCCESS);
    // test LCD
    write_LCD("welcome, ahoj");
   
    // test LED
    show_LED(13);
    
    //cd(work_dir);
    //ls();
    //showFirst();

    // I/O
/*
    while (true) {
        manage_KBD();
    }
*/

    for(;;){
        manage_KBD();
    }
    
    // terminate
    close_device(EXIT_SUCCESS);
}