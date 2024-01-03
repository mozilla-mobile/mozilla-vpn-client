#include "../tools/commandhandler.h"
#include "../tools/command.h"
#include "../inspector.h"

#ifndef MZ_LEGACY_INSPECTOR
#  define MZ_LEGACY_INSPECTOR


class InspectorLegacyCommands {
 public:
  // Those Commands have grown to become a lovely ball of spagetti.
  // I'm not going to bother untangling this.
  // Please do not consider adding new ones here,
  // Add them wherever the Commands are actually used.
  static void registerCommands(Inspector* i);
};


#endif  // !MZ_LEGACY_INSPECTOR


