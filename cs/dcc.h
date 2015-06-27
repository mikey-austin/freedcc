/**
 * @file dcc.h
 * @brief Defines the DCC packet management API.
 * @author Mikey Austin
 * @date 2010-2011
 */

#ifndef DCC_INCLUDED
#define DCC_INCLUDED

#define DCC_DIRECTION_FORWARD   1
#define DCC_DIRECTION_REVERSE   0
#define DCC_ADDRESS_MAX         128
#define DCC_MAX_SPEED_STEPS     29

#define T DCC_packet_T
typedef struct T *T;

/**
 * The DCC packet data structure.
 *
 * This is a standard DCC packet, the number of bytes in which must be
 * compiled in.
 */
struct T
{
    unsigned char *bytes;
    int size;
};

/**
 * This function creates an empty baseline DCC packet.
 *
 * The fresh DCC packet returned is initialised with all zeros.
 */
extern T DCC_baseline_packet_create(void);

/**
 * This function creates an empty DCC packet of specified size.
 *
 * The fresh DCC packet returned is initialised with all zeros.
 */
extern T DCC_packet_create(int size);

/**
 * Destroy a DCC packet created with <i>DCC_packet_create</i>.
 */
extern void DCC_packet_destroy(T packet);

/** 
 * Compare the speed of two DCC packets.
 *
 * @param p1 DCC packet 1.
 * @param p2 DCC packet 2.
 *
 * @return -1, 0 or 1 if the speed of <i>p1</i> is less than, equal to or greater 
 *  than the speed of <i>p2</i>, respectively.
 */
extern int DCC_compare_speed(T p1, T p2);

/** 
 * The special reset packet for all locos.
 */
extern void DCC_special_reset_packet(T packet);

/**
 * The special idle packet for all decoders.
 */
extern void DCC_special_idle_packet(T packet);

/**
 * The special broadcast stop packet for all decoders.
 */
extern void DCC_special_broadcast_stop_packet(T packet);

/**
 * Broadcast stop by telling decoders to stop delivering energy to motor.
 */
extern void DCC_special_emergency_stop_packet(T packet);

/**
 * Determins whether a packet is a broadcast stop packet.
 */
extern int DCC_is_broadcast_stop(T packet);

/** Set the normal non-programming DCC packet preamble. */
extern void DCC_set_preamble(T packet);

/** Set the loco address. */
extern void DCC_set_address(T packet, unsigned char address);

/** Set the speed and direction byte preamble. */
extern void DCC_set_speed_direction_preamble(T packet);

/** Set the direction. */
extern void DCC_set_direction(T packet, int direction);

/** Set the the speed. */
extern void DCC_set_speed(T packet, int step);

/** Calculate and set the packet checksum byte. */
extern void DCC_set_checksum(T packet);

/** Set the DCC packet end bit. */
extern void DCC_set_packet_end(T packet);

/** Return the loco address of the packet. */ 
extern unsigned char DCC_get_address(T packet);

/** Return the speed and direction byte of the packet. */ 
extern int DCC_get_speed_and_direction(T packet);

/** Return the corresponding speed step of the packet. */
extern int DCC_get_speed_step(T packet);

/** Return the direction of the packet. */
extern int DCC_get_direction(T packet);

/** Return a binary string representation of a packet. */
extern char *DCC_dump(T packet);

/** Return a hex string representation of a packet. */
extern char *DCC_hex_dump(T packet);

/** Print a packet to stdout for debugging. */
extern void DCC_packet_dump(T packet);

/** Print a byte to stdout for debugging. */
extern void DCC_byte_dump(unsigned char byte);

/** Print a packet to stdout for debugging in hexadecimal. */
extern void DCC_packet_dump_hex(T packet);

/** Print a byte to stdout for debugging in hexadecimal. */
extern void DCC_byte_dump_hex(unsigned char byte);

#undef T
#endif
