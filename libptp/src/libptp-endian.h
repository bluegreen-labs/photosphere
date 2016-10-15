/* This file is generated automatically by configure */
/* It is valid only for the system type x86_64-unknown-linux-gnu */

#ifndef __BYTEORDER_H
#define __BYTEORDER_H

/* ntohl and relatives live here */
#include <arpa/inet.h>

/* Define generic byte swapping functions */
#include <byteswap.h>
#define swap16(x) bswap_16(x)
#define swap32(x) bswap_32(x)
#define swap64(x) bswap_64(x)

/* The byte swapping macros have the form: */
/*   EENN[a]toh or htoEENN[a] where EE is be (big endian) or */
/* le (little-endian), NN is 16 or 32 (number of bits) and a, */
/* if present, indicates that the endian side is a pointer to an */
/* array of uint8_t bytes instead of an integer of the specified length. */
/* h refers to the host's ordering method. */

/* So, to convert a 32-bit integer stored in a buffer in little-endian */
/* format into a uint32_t usable on this machine, you could use: */
/*   uint32_t value = le32atoh(&buf[3]); */
/* To put that value back into the buffer, you could use: */
/*   htole32a(&buf[3], value); */

/* Define aliases for the standard byte swapping macros */
/* Arguments to these macros must be properly aligned on natural word */
/* boundaries in order to work properly on all architectures */
#define htobe16(x) htons(x)
#define htobe32(x) htonl(x)
#define be16toh(x) ntohs(x)
#define be32toh(x) ntohl(x)

#define HTOBE16(x) (x) = htobe16(x)
#define HTOBE32(x) (x) = htobe32(x)
#define BE32TOH(x) (x) = be32toh(x)
#define BE16TOH(x) (x) = be16toh(x)

/* On little endian machines, these macros are null */
#define htole16(x)      (x)
#define htole32(x)      (x)
#define htole64(x)      (x)
#define le16toh(x)      (x)
#define le32toh(x)      (x)
#define le64toh(x)      (x)

#define HTOLE16(x)      (void) (x)
#define HTOLE32(x)      (void) (x)
#define HTOLE64(x)      (void) (x)
#define LE16TOH(x)      (void) (x)
#define LE32TOH(x)      (void) (x)
#define LE64TOH(x)      (void) (x)

/* These don't have standard aliases */
#define htobe64(x)      swap64(x)
#define be64toh(x)      swap64(x)

#define HTOBE64(x)      (x) = htobe64(x)
#define BE64TOH(x)      (x) = be64toh(x)

/* Define the C99 standard length-specific integer types */
#include "libptp-stdint.h"

/* Here are some macros to create integers from a byte array */
/* These are used to get and put integers from/into a uint8_t array */
/* with a specific endianness.  This is the most portable way to generate */
/* and read messages to a network or serial device.  Each member of a */
/* packet structure must be handled separately. */

/* Non-optimized but portable macros */
#define be16atoh(x)     ((uint16_t)(((x)[0]<<8)|(x)[1]))
#define be32atoh(x)     ((uint32_t)(((x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3]))
#define be64atoh(x)     ((uint64_t)(((x)[0]<<56)|((x)[1]<<48)|((x)[2]<<40)| \
        ((x)[3]<<32)|((x)[4]<<24)|((x)[5]<<16)|((x)[6]<<8)|(x)[7]))
#define le16atoh(x)     ((uint16_t)(((x)[1]<<8)|(x)[0]))
#define le32atoh(x)     ((uint32_t)(((x)[3]<<24)|((x)[2]<<16)|((x)[1]<<8)|(x)[0]))
#define le64atoh(x)     ((uint64_t)(((x)[7]<<56)|((x)[6]<<48)|((x)[5]<<40)| \
        ((x)[4]<<32)|((x)[3]<<24)|((x)[2]<<16)|((x)[1]<<8)|(x)[0]))

#define htobe16a(a,x)   (a)[0]=(uint8_t)((x)>>8), (a)[1]=(uint8_t)(x)
#define htobe32a(a,x)   (a)[0]=(uint8_t)((x)>>24), (a)[1]=(uint8_t)((x)>>16), \
        (a)[2]=(uint8_t)((x)>>8), (a)[3]=(uint8_t)(x)
#define htobe64a(a,x)   (a)[0]=(uint8_t)((x)>>56), (a)[1]=(uint8_t)((x)>>48), \
        (a)[2]=(uint8_t)((x)>>40), (a)[3]=(uint8_t)((x)>>32), \
        (a)[4]=(uint8_t)((x)>>24), (a)[5]=(uint8_t)((x)>>16), \
        (a)[6]=(uint8_t)((x)>>8), (a)[7]=(uint8_t)(x)
#define htole16a(a,x)   (a)[1]=(uint8_t)((x)>>8), (a)[0]=(uint8_t)(x)
#define htole32a(a,x)   (a)[3]=(uint8_t)((x)>>24), (a)[2]=(uint8_t)((x)>>16), \
        (a)[1]=(uint8_t)((x)>>8), (a)[0]=(uint8_t)(x)
#define htole64a(a,x)   (a)[7]=(uint8_t)((x)>>56), (a)[6]=(uint8_t)((x)>>48), \
        (a)[5]=(uint8_t)((x)>>40), (a)[4]=(uint8_t)((x)>>32), \
        (a)[3]=(uint8_t)((x)>>24), (a)[2]=(uint8_t)((x)>>16), \
        (a)[1]=(uint8_t)((x)>>8), (a)[0]=(uint8_t)(x)

#endif /*__BYTEORDER_H*/
