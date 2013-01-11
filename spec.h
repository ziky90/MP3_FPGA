/* 
 * File:   spec.h
 * Author: zikesjan
 *
 */

#ifndef SPEC_H
#define	SPEC_H
    
#define VENDOR0 0x72
#define VENDOR1 0x11
#define DEVICE0 0x32
#define DEVICE1 0x1f
    
#define ON 0xff
#define OFF 0x00

#define LCD_NEXT_LINE 0x40
    
#define FIRST_COLUMN 0x03
#define SECOND_COLUMN 0x05
#define THIRD_COLUMN 0x06

#define MEM_LENGTH 0x10000
#define STR_LEN 20
#define LCD_LINE_LEN 16
#define MULTIPLICATION_CONST 8

#define RW_TIME 1000
#define SOUND_TIME 10
#define LED_TIME 50000

#define KEY_CHECK_CONST 5

#define PCI_DIR "/proc/bus/pci"
#define DEV_MEM "/dev/mem"

#define DEVICE_SPEC_READ_COUNT 4
#define DEVICE_ADDR_READ_COUNT 16

#define MASK 0xff
#define MASK2 0xffffffff

unsigned const char device_specification[] = {VENDOR0, VENDOR1, DEVICE0, DEVICE1};

typedef enum {
    false, true
} boolean;

typedef struct{
    boolean pressed;
    int count;
    int repeat_count;
    unsigned char last_key;    
}key_ctrl;
    

#endif	/* SPEC_H */

