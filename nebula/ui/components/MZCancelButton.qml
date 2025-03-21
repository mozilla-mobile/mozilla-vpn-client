/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Mozilla.Shared 1.0
import components 0.1

//MZCancelButton
MZLinkButton {
    labelText: MZI18n.InAppSupportWorkflowSupportSecondaryActionText // "Cancel"
    fontName: MZTheme.theme.fontBoldFamily
    linkColor: MZTheme.colors.destructiveButton
}
