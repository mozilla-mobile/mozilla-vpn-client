# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Read and re-generate lottie.mjs
file(READ ${CMAKE_CURRENT_LIST_DIR}/lottie.min.js LOTTIEJS)
configure_file(${CMAKE_CURRENT_LIST_DIR}/lottie_wrap.js.template lottie.mjs)
