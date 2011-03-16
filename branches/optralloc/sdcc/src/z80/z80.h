/** @file z80/z80.h
    Common definitions between the z80 and gbz80 parts.
*/
#include "common.h"
#include "ralloc.h"
#include "gen.h"
#include "peep.h"
#include "support.h"

typedef enum
  {
    SUB_Z80,
    SUB_GBZ80
  }
Z80_SUB_PORT;

typedef struct
  {
    Z80_SUB_PORT sub;
    int calleeSavesBC;
    int port_mode;
    int port_back;
    int reserveIY;
    int optralloc_all;
    int optralloc_hl;
    int dump_graphs;
  }
Z80_OPTS;

extern Z80_OPTS z80_opts;

#define IS_GB  (z80_opts.sub == SUB_GBZ80)
#define IS_Z80 (z80_opts.sub == SUB_Z80)
#define IY_RESERVED (z80_opts.reserveIY)
#define OPTRALLOC_ALL (z80_opts.optralloc_all)

#define OPTRALLOC_ALL 1

#define OPTRALLOC_HL (z80_opts.optralloc_all || z80_opts.optralloc_hl)
#define OPTRALLOC_A (OPTRALLOC_ALL || !OPTRALLOC_HL || IS_GB)

enum
  {
    ACCUSE_A = 1,
    ACCUSE_SCRATCH,
    ACCUSE_IY
  };
