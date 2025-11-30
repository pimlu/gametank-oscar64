#pragma once

#ifdef __OSCAR64C__
// oscar64 only supports float, not double
// Map double to float for compatibility
#define double float
#endif
