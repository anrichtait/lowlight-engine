/**
  \file G3D-base.lib/include/G3D-base/netheaders.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_netheaders_h
#define G3D_netheaders_h

#include "G3D-base/platform.h"

#ifdef G3D_WINDOWS
#   include <winsock2.h>
#else
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   ifndef SOCKADDR_IN
#       define SOCKADDR_IN struct sockaddr_in
#   endif
#   ifndef SOCKET
#       define SOCKET int
#   endif
#endif

#endif
