/**
 * @file dcc.c
 * @brief Implements the DCC packet management API.
 * @author Mikey Austin
 * @date 2010-2011
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcc.h"

#define T DCC_packet_T

/*
 * The trailing number refers to the byte the
 * mask applies to, starting from 0.
 */
#define DCC_MASK_PREAMBLE_0     0x00
#define DCC_MASK_PREAMBLE_1     0x0F
#define DCC_MASK_ADDRESS_1      0xFC
#define DCC_MASK_ADDRESS_2      0x07
#define DCC_MASK_SD_PREAMBLE_2  0xFC
#define DCC_MASK_DIRECTION_3    0x7F
#define DCC_MASK_SPEED_3        0x83
#define DCC_MASK_CHECKSUM_3     0xFE
#define DCC_MASK_CHECKSUM_4     0x01
#define DCC_BASELINE_LEN        5

/**
 * DCC speed step lookup table.
 */
static unsigned char DCC_speed_steps[] = {
    0x00,   /**< Stop */
    0x02,   /**< Step 1 */
    0x12,   /**< Step 2 */
    0x03,   /**< Step 3 */
    0x13,   /**< Step 4 */
    0x04,   /**< Step 5 */
    0x14,   /**< Step 6 */
    0x05,   /**< Step 7 */
    0x15,   /**< Step 8 */
    0x06,   /**< Step 9 */
    0x16,   /**< Step 10 */
    0x07,   /**< Step 11 */
    0x17,   /**< Step 12 */
    0x08,   /**< Step 13 */
    0x18,   /**< Step 14 */
    0x09,   /**< Step 15 */
    0x19,   /**< Step 16 */
    0x0A,   /**< Step 17 */
    0x1A,   /**< Step 18 */
    0x0B,   /**< Step 19 */
    0x1B,   /**< Step 20 */
    0x0C,   /**< Step 21 */
    0x1C,   /**< Step 22 */
    0x0D,   /**< Step 23 */
    0x1D,   /**< Step 24 */
    0x0E,   /**< Step 25 */
    0x1E,   /**< Step 26 */
    0x0F,   /**< Step 27 */
    0x1F    /**< Step 28 */
};

extern T
DCC_packet_create(int size)
{
    T packet;

    packet = calloc(1, sizeof(*packet));
    packet->bytes = calloc(size, sizeof(char));
    packet->size = size;

    return packet;
}

extern T
DCC_baseline_packet_create(void)
{
    return DCC_packet_create(DCC_BASELINE_LEN);
}

extern void
DCC_packet_destroy(T packet)
{
    if(packet && packet->bytes)
        free(packet->bytes);

    packet->bytes = NULL;

    if(packet)
        free(packet);

    packet = NULL;

    return;
}

extern int
DCC_compare_speed(T p1, T p2)
{
    int s1, s2;

    if((s1 = DCC_get_speed_step(p1)) == (s2 = DCC_get_speed_step(p2)))
        return 0;

    return (s1 > s2 ? 1 : -1);
}

extern void
DCC_special_reset_packet(T packet)
{
    packet->bytes[0] = 0xFF;
    packet->bytes[1] = 0xF0;
    packet->bytes[2] = 0x00;
    packet->bytes[3] = 0x00;
    packet->bytes[4] = 0x01;
}

extern void
DCC_special_idle_packet(T packet)
{
    packet->bytes[0] = 0xFF;
    packet->bytes[1] = 0xF7;
    packet->bytes[2] = 0xF8;
    packet->bytes[3] = 0x01;
    packet->bytes[4] = 0xFF;
}

extern void
DCC_special_broadcast_stop_packet(T packet)
{
    packet->bytes[0] = 0xFF;
    packet->bytes[1] = 0xF0;
    packet->bytes[2] = 0x01;
    packet->bytes[3] = 0xC0;
    packet->bytes[4] = 0xE1;
}

extern void
DCC_special_emergency_stop_packet(T packet)
{
    packet->bytes[0] = 0xFF;
    packet->bytes[1] = 0xF0;
    packet->bytes[2] = 0x01;
    packet->bytes[3] = 0xC4;
    packet->bytes[4] = 0xE3;
}

extern int
DCC_is_broadcast_stop(T packet)
{
    if(packet->size != DCC_BASELINE_LEN)
        return 0;

    if(packet->bytes[0] == 0xFF
        && packet->bytes[1] == 0xF0
        && packet->bytes[2] == 0x01
        && (packet->bytes[3] == 0xC0
            || packet->bytes[3] == 0xC4)
        && (packet->bytes[4] == 0xE1
            || packet->bytes[4] == 0xE3))
    {
        return 1;
    }

    /* Not a broadcast stop packet. */
    return 0;
}

extern void
DCC_set_preamble(T packet)
{
    /* Clear the bits about to be set. */
    packet->bytes[0] &= DCC_MASK_PREAMBLE_0;
    packet->bytes[1] &= DCC_MASK_PREAMBLE_1;

    /* Set the bits. */
    packet->bytes[0] |= 0xFF;
    packet->bytes[1] |= 0xF0;

    return;
}

extern void
DCC_set_address(T packet, unsigned char address)
{
    address %= DCC_ADDRESS_MAX;

    /* Clear the bits about to be set. */
    packet->bytes[1] &= DCC_MASK_ADDRESS_1;
    packet->bytes[2] &= DCC_MASK_ADDRESS_2;

    /* Set the low 3 bits in byte 2. */
    packet->bytes[1] |= (address >> 5);

    /* Set the high 5 bits in byte 3. */
    packet->bytes[2] |= (address << 3);

    return;
}

extern void
DCC_set_speed_direction_preamble(T packet)
{
    /* Clear bits for a speed & direction packet. */
    packet->bytes[2] &= DCC_MASK_SD_PREAMBLE_2;

    /* Set low 2 bits in byte 3 */
    packet->bytes[2] |= 0x01;

    return;
}

extern void
DCC_set_direction(T packet, int direction)
{
    /* Clear direction bit. */
    packet->bytes[3] &= DCC_MASK_DIRECTION_3;

    /* Set the direction bit if direction is positive. */
    packet->bytes[3] |= direction ? 0x80 : 0x00;

    return;
}

extern void
DCC_set_speed(T packet, int step)
{
    /* Make sure the step is in range. */
    step %= DCC_MAX_SPEED_STEPS;

    /* Clear the speed bits. */
    packet->bytes[3] &= DCC_MASK_SPEED_3;

    /* Set the speed bits, which start from bit 2. */
    packet->bytes[3] |= (DCC_speed_steps[step] << 2);

    return;
}

extern void
DCC_set_checksum(T packet)
{
    unsigned char address, speed, checksum;

    address = DCC_get_address(packet);
    speed = DCC_get_speed_and_direction(packet);

    /* Calculate checksum XOR. */
    checksum = address ^ speed;

    /* Clear & set checksum bits. */
    packet->bytes[3] &= DCC_MASK_CHECKSUM_3;
    packet->bytes[4] &= DCC_MASK_CHECKSUM_4;
    packet->bytes[3] |= (checksum >> 7);
    packet->bytes[4] |= (checksum << 1);

    return;
}

extern void
DCC_set_packet_end(T packet)
{
    packet->bytes[4] |= 0x1;

    return;
}

extern unsigned char
DCC_get_address(T packet)
{
    unsigned char address;

    /* Extract the address byte. */
    address = ((packet->bytes[1] & ~DCC_MASK_ADDRESS_1) << 5);
    address |= ((packet->bytes[2] & ~DCC_MASK_ADDRESS_2) >> 3);

    return address;
}

extern int
DCC_get_speed_and_direction(T packet)
{
    unsigned char speed;

    /* Extract the speed & direction byte. */
    speed = 0x40 | ((packet->bytes[3] & ~DCC_MASK_DIRECTION_3) >> 2);
    speed |= ((packet->bytes[3] & ~DCC_MASK_SPEED_3) >> 2);

    return speed;
}

extern int
DCC_get_speed_step(T packet)
{
    unsigned char speed;
    int i;

    speed = ((packet->bytes[3] & ~DCC_MASK_SPEED_3) >> 2);

    /* Find the corresponding speed step. */
    for(i=0; i < DCC_MAX_SPEED_STEPS; i++)
    {
        if(DCC_speed_steps[i] == speed)
            return i;
    }

    return 0;
}

extern int
DCC_get_direction(T packet)
{
    unsigned char direction;

    direction = ((packet->bytes[3] & ~DCC_MASK_DIRECTION_3) >> 7);

    return (direction == DCC_DIRECTION_FORWARD ?
                DCC_DIRECTION_FORWARD : DCC_DIRECTION_REVERSE);
}

extern char
*DCC_dump(T packet)
{
    int i, j, k=0, dump_len;
    char *dump;

    /* Calculate the dump length. */
    dump_len = (packet->size * 8) + (packet->size - 2);

    dump = (char*) malloc((sizeof(char) * dump_len) + 1);

    for(i=0; i < packet->size && k <= dump_len; i++)
    {
        for(j=7; j >= 0; j--)
            dump[k++] = (packet->bytes[i] & (1 << j)) ? '1' : '0';

        if(i < (packet->size - 1))
            dump[k++] = ' ';
    }
    dump[k] = '\0';

    return dump;
}

extern void
DCC_packet_dump(T packet)
{
    int i;

    for(i=0; i < packet->size; i++)
    {
        DCC_byte_dump(packet->bytes[i]);

        if(i < (packet->size - 1))
            printf(" ");
    }

    return;
}

extern void
DCC_byte_dump(unsigned char byte)
{
    int i;

    for(i=7; i >= 0; i--)
        printf("%c", (byte & (1 << i)) ? '1' : '0');

    return;
}

extern char
*DCC_hex_dump(T packet)
{
    int i, k=0, dump_len;
    char *dump;

    /* Calculate the dump length. */
    dump_len = (packet->size * 2) + (packet->size - 1);

    dump = (char*) malloc((sizeof(char) * dump_len) + 1);

    for(i=0; i < packet->size && k <= dump_len; i++)
    {
        /* Apart from the last byte, the null byte from snprintf will be overwritten. */
        snprintf(dump + k, 3, "%02x", packet->bytes[i]);
        k += 2;

        if(i < (packet->size - 1))
            dump[k++] = ' ';
    }

    return dump;
}

extern void
DCC_packet_dump_hex(T packet)
{
    int i;

    for(i=0; i < packet->size; i++)
    {
        DCC_byte_dump_hex(packet->bytes[i]);

        if(i < (packet->size - 1))
            printf(" ");
    }

    return;
}

extern void
DCC_byte_dump_hex(unsigned char byte)
{
    printf("%x", byte);

    return;
}
