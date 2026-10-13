/**
 * WARNING: Only comment out / uncomment whole #ifdef...#define...#endif blocks,
 * NEVER just the #define - an #ifdef...//#define...#endif block will segfault the game.
 */

//! Uncomment this for diag tracing (warning, very verbose, performance hit)
//#ifdef DIAG
//#define JM_COT_DIAG_LOGGING
//#endif

//! Uncomment this for debug logs
#ifdef DIAG
#define COT_DEBUGLOGS
#endif
