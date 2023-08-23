/* Modbus connection shared library
 * 
 * RPMarques (20may22)
 * 
 */

#ifndef mbclient_h__
#define mbclient_h__

#include <stdint.h>

extern int mbclient_connect(void);

extern int mbclient_disconnect(void);

extern int mbclient_read_coil(int, int, uint8_t*);

extern int mbclient_read_holdreg(int, int, uint16_t*);

extern int mbclient_read_discinp(int, int, uint8_t*);

extern int mbclient_read_inpreg(int, int, uint16_t*);

extern int mbclient_write_coil(int, int, uint8_t*);

extern int mbclient_write_holdreg(int, int, uint16_t*);

#endif  // mbclient_h__