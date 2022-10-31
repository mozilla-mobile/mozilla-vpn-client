/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import {
  fluentCombobox,
  fluentOption,
  provideFluentDesignSystem,
} from "https://unpkg.com/@fluentui/web-components@2.5.6/dist/web-components.min.js";

provideFluentDesignSystem()
  .register(
    fluentCombobox(),
    fluentOption(),
  );

console.log("Fluent Loaded!");
