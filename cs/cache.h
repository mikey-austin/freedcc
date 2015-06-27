/**
 * @file cache.h
 * @brief Caches recent loco packets sent to the track.
 * @author Mikey Austin
 * @date 2012-2013
 *
 * The purpose of this module is to:
 * - Store last packet sent for each loco
 * - Store a list of "active" locos
 * - Contain logic to return a packet from the cache to be refreshed
 * - Provide interface to empty cache
 */

#ifndef CACHE_DEFINED
#define CACHE_DEFINED

#include "dcc.h"

extern void         Cache_module_init(void);
extern void         Cache_clear(void);
extern void         Cache_update(DCC_packet_T packet);
extern DCC_packet_T Cache_get_next_packet(void);
extern DCC_packet_T Cache_get(int address);
extern int          Cache_report_current_size(void);
extern int          Cache_report_total_size(void);

#endif
