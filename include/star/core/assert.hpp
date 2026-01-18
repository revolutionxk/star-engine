#pragma once

#if defined(_DEBUG) || defined(DEBUG)
    #define STAR_DEBUG
    #define STAR_ASSERT(x, msg) assert((x) && (msg))
#else
    #define STAR_ASSERT(x, msg) ((void)0)
#endif
