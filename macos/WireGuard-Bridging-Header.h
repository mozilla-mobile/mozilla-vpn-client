#include "wireguard-go-version.h"
#include "ringlogger.h"
#include "key.h"

#import "TargetConditionals.h"
#if TARGET_OS_OSX
#include <libproc.h>
#endif
