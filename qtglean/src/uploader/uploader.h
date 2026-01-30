/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UPLOADER_H
#define UPLOADER_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vpn_ping_header {
    const char* name;
    const char* value;
};

struct vpn_ping_payload {
    const char* url;
    const void* body;
    size_t length;
    bool has_info;
    struct vpn_ping_header* headers;
};

int vpn_ping_upload(const struct vpn_ping_payload* payload);

#ifdef __cplusplus
}
#endif

#endif  // UPLOADER_H
