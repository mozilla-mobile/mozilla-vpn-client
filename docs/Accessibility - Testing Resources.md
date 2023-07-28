# Mozilla VPN Accessibility -- Testing Resources

## Summary

This document provides information about testing procedures and tools
that are used to validate accessibility features of the Mozilla VPN
Client.

## Motivation

Accessibility is an important feature that ensures that all users,
including those with disabilities, can use the Mozilla VPN Client
effectively. By implementing robust accessibility testing approaches, we
can enhance the user experience, reach a broader audience, and comply
with accessibility standards on all supported platforms.

## Goals

1. Compatibility with Major Accessibility Technologies (ATs)
    - Users with disabilities can use the VPN Client effectively with
    major ATs.

2. Compliance with Accessibility Standards on All Supported Platforms
    - Identify and address accessibility issues in the VPN Client to
    ensure compliance.

3. Enhance Usability for All Users

    - Accessible features benefit everyone. By implementing accessible
    features, the VPN Client will improve the user experience for all users.

## Overview of Accessibility Technologies to Support in Mozilla VPN

Accessibility Technologies supported in Mozilla VPN Client can be
classified into the following areas for users with disabilities: Vision,
Hearing, Mobility, and Neurodivergence/Learning.

An overview of these areas is listed below, and more information is
provided for each in the [Manual Accessibility
Testing](#manual-accessibility-testing) section.

**Vision:**

1. **Screen Reader support:** Allows the application\'s content to be
    read aloud to the user and allows navigation through the content
    by using keyboard shortcuts and touch gestures.

2. **Screen Magnification**: Enlarges content, making it easier to read
     and interact with. Support for automatically bringing the focused
     element into the magnifier's view improves usability.

3. **High Contrast Themes and Color Filters:** Adjusts for light and
     color sensitivity. Accommodates for color blindness.

4. **Personalization of Text Size,Text Cursor and Mouse Pointer:**
     Ensures better visibility and ease of use.

5. **Ability to Turn Off Animation:** Reduces distractions and visual
     stimuli.

6. **Personalization of the Notification Duration:** Allows
     customization of the duration of notifications that automatically
     dismiss, to allow users more time to read the notifications.

**Hearing:**

1. **Cues for Audio Notification:** Flashes screen/LED or vibrates
     during audio notification.

**Mobility:**

1. **Keyboard Access:** Keyboard access to all controls for users who
     cannot use other input devices.

2. **Voice Access, Voice Typing, Eye Control, Switch Control Support**:
     Other modes of interaction for users with mobility impairments.

**Neurodivergence/Do-not-disturb:**

1. **Notification Suppression:** Prevents system notifications from the
     VPN client from disturbing users while they are in do-not-disturb
     mode.

## Principles for Testing of Accessibility Features in the VPN Client

1. **Prioritize Testing based on User Impact and Customization:** Focus
 on Accessibility Technologies (ATs) that require customization or specific
 support in the VPN client, as these areas are more likely to reveal
 application issues.

   Some ATs, like Screen Magnification, Voice Typing, Eye Control, and
 Switch Control, typically don\'t need customized application support,
 as they integrate with core platform subsystems. Issues in these areas
 are usually attributable to problems in the AT or platform.

2. **Reduce Redundancy:** Minimize redundant testing by focusing on ATs
 like Screen Readers, which require the VPN Client to provide Accessibility
 information. Other ATs like Voice Access utilize the same Accessibility
 information as a screen reader, so they don't need thorough testing.

3. **Utilize Testing Tools for Automated Accessibility Testing:** Use testing
 tools whenever possible to quickly identify major Accessibility issues in the
 VPN Client. Refer to the
 [Accessibility Testing Tools](#accessibility-testing-tools)
 section for details on these tools and their capabilities.

4. **Alignment with Established Standards:** Ensure that the information in
 this document aligns with
 [WCAG](https://www.w3.org/WAI/standards-guidelines/wcag/)
 standards, even though the VPN Client is not a Web App.

5. **Foster Collaborative Development and Testing:** Promote a collaborative
 approach among developers, testers and
 accessibility experts during the development and testing of
 Accessibility features in the VPN client. This will result in higher
 quality implementations and early detection of accessibility issues in
 the VPN Client.

## Prioritized list of Accessibility Areas for Testing

Based on the [Principles for Testing](#principles-for-testing-of-accessibility-features-in-the-vpn-client)
section, the following is a suggested prioritized list for testing
Accessibility in the VPN Client:

1. Screen Reader

2. Keyboard Access

3. High Contrast Themes and Color Filters

4. Personalization of Text Size and Text Cursor

5. Notification Suppression

6. Ability to Turn Off Animation, Cues for Audio Notification

As resources become available, other areas can be tested to further
validate Accessibility support in the application.

The VPN Client uses the Qt cross-platform framework, which means that
any platform-specific problems can usually be attributed to either the
framework itself or the underlying platform.

## Manual Accessibility Testing

Many Accessibility Technologies need to be manually tested with the VPN
Client because of the complexity of the interactions with the ATs.

Some platforms offer accessibility testing tools to automatically
identify issues in specific areas. Details about these tools are
discussed in the [Accessibility Testing
Tools](#accessibility-testing-tools) section.

### Vision - Screen Reader Support

A screen reader reads an application's content aloud to the user. It
provides keyboard shortcuts or touch screen gestures to navigate through
an application, read content, discover functionality of an element and
to invoke that functionality. To achieve this, the application must make
this information and functionality available to the screen reader
through platform-specific standards. Failure to do so properly may
result in the screen reader being unable to access or read certain parts
of the application.

As the user tabs through the application interface or uses specialized
navigation shortcuts and gestures like swipes and taps on a touch
screen, the screen reader will read out information related to the
current element or perform operations using the current element.

Apart from audio output, many screen readers can also send their output
to braille displays, serving users who have both visual and hearing
impairments.

**Major Screen Readers:**

- Windows

  - [Narrator](https://support.microsoft.com/en-us/windows/complete-guide-to-narrator-e4397a0d-ef4f-b386-d8ae-c172f109bdb1):
         Built-in screen reader.

  - [JAWS](https://www.freedomscientific.com/products/software/jaws/):
         A leading screen reader on this platform. Paid product. Has a
         free trial.

  - [NVDA](https://www.nvaccess.org/): A leading
         screen reader on this platform. Free and open source.

- MacOS, iOS

  - [VoiceOver:](https://www.apple.com/accessibility/vision/)
         Built-in screen reader.

- Android

  - [TalkBack](https://support.google.com/accessibility/android/topic/3529932?hl=en&ref_topic=9078845&sjid=8526379012974574733-NA):
         Built-in screen reader.

- Linux

  - [Orca](https://help.ubuntu.com/stable/ubuntu-help/a11y-screen-reader.html.en)

**Usage Guides:**

Manual testing with screen readers requires a basic understanding of
their usage. Here are some introductory usage guides:

- [Narrator](https://support.microsoft.com/en-us/windows/chapter-1-introducing-narrator-7fe8fd72-541f-4536-7658-bfc37ddaf9c6)

- [NVDA](https://www.youtube.com/watch?v=Jao3s_CwdRU)

- [JAWS](https://webaim.org/articles/jaws/)

- [VoiceOver](https://support.apple.com/guide/iphone/turn-on-and-practice-voiceover-iph3e2e415f/16.0/ios/16.0)

- [TalkBack](https://support.google.com/accessibility/android/answer/6283677?hl=en)

- [Orca](https://help.ubuntu.com/stable/ubuntu-help/a11y-screen-reader.html.en)

At minimum, learn how to do the following with a screen reader:

- Start and stop the screen reader.

- Navigate through content using standard keys and gestures like tab,
     arrow keys, enter key, swipe gestures and tap.

- Navigate through content using specialized screen reader shortcuts
     to the next/previous button, control, text input field, etc.

- Use shortcuts to read all the text in a screen, and to read text
     from the current location.

- Use shortcuts to read the next/previous line, word or character in a
     block of text.

**Testing scenarios:**

1. Test each screen of the VPN client with the screen reader.

2. Navigate to each control using Tab, Shift-Tab, Arrow Keys, swipe
     left/right and verify that the control's accessible name is read.
     Verify that any primary functionality can be invoked by using the
     Enter/Spacebar key or a double tap.

3. Use the screen-reader's specialized keyboard shortcuts and touch
     gestures to move between controls, read text and read a control's
     type. For example, some screen readers have specialized shortcuts
     to move to the next or previous button, next or previous text
     input field etc.

4. Verify that all the text in the screen can be read using the screen
     reader.

5. Verify that the VPN's on/off/error state transitions are
     automatically announced when changes occur in the connection
     state.

6. Verify that lists or tabs indicate the number of items and position
     of the current item.

7. Verify that text input field changes are correctly read when
     characters and words are entered and deletions are made.

8. Verify that notifications are automatically read when they appear.

9. Verify that off-screen content in lists can be accessed using the
     screen reader.

10. Verify that the user can use platform shortcuts to activate another
     application and then reactivate the VPN client. Then verify that
     focus is restored to the previously focused element.

11. Verify that any information communicated using an image is available
     to a screen reader user.

12. Verify the reading of localized content, which typically entails
     downloading and installing a Screen Reader voice specifically
     designed for the target language. The steps required for this
     process may vary depending on the specific screen reader in use.

13. Testing with braille displays may not be essential as it requires
     specific braille hardware, and any issues related to braille
     functionality are usually attributed to Screen Reader and braille
     driver problems.

### Vision - Screen Magnification

A magnification tool enlarges part or all of the screen, improving the
visibility of text and images for the user. Additionally, it can be
configured to track the keyboard focus, enabling the focused element to
automatically scroll into the magnification tool's viewport.

**Usage Guides:**

- Windows

  - [Magnifier](https://support.microsoft.com/en-us/windows/use-magnifier-to-make-things-on-the-screen-easier-to-see-414948ba-8b1c-d3bd-8615-0e5e32204198)

- MacOS

  - [Zoom](https://support.apple.com/guide/mac-help/zoom-in-and-out-mchlp2975/mac)

- iOS

  - [Zoom](https://support.apple.com/guide/iphone/zoom-iph3e2e367e/ios)

- Android

  - [Magnification](https://support.google.com/accessibility/android/answer/6006949?hl=en)

- Linux

  - [Zoom](https://help.ubuntu.com/stable/ubuntu-help/a11y-mag.html.en)

**Testing Scenarios:**

1. Turn on the magnification tool and test that the VPN Client is
     properly magnified. Any issues related to the core magnification
     functionality are usually attributable to magnification tool
     problems.

2. Turn on the magnification tool, turn on its focus tracking, and test
     input into screens which have text input elements. Position the
     VPN client so that a focused text input field is outside the
     magnification tool's viewport. Then enter text and verify that the
     focused text input field is scrolled into the magnification tool's
     viewport.

### Vision - High Contrast Themes and Color Filters

Text with low contrast against its background can pose reading
difficulties for some users. Applying a high contrast theme to such text
can make it easier to read.

Color Filters allow on-screen colors to be adjusted for different types
of color blindness.

**Usage Guides:**

- Windows

  - [Contrast
         Themes](https://support.microsoft.com/en-us/windows/change-color-contrast-in-windows-fedc744c-90ac-69df-aed5-c8a90125e696),
         [Color
         Filters](https://support.microsoft.com/en-us/windows/use-color-filters-in-windows-43893e44-b8b3-2e27-1a29-b0c15ef0e5ce)

- MacOS

  - [Invert Colors and Color
         Filters](https://support.apple.com/lv-lv/guide/mac-help/mchl11ddd4b3/13.0/mac/13.0)

- iOS

  - [Invert Colors and Color
         Filters](https://support.apple.com/en-us/HT207025)

- Android

  - [High contrast text and Color
         Correction](https://support.google.com/accessibility/android/answer/11183305?hl=en&sjid=12389387769888064770-NA#zippy=%2Cuse-high-contrast-text%2Cuse-color-correction)

- Linux

  - [High
         Contrast](https://help.ubuntu.com/stable/ubuntu-help/a11y-contrast.html.en)

### Vision - Personalization of Text Size,Text Cursor and Mouse Pointer

Text size can be changed to improve readability.

The text cursor and mouse pointer can be difficult to see for some
users. Some platforms provide personalization to change the size, shape
and color of the text cursor and mouse pointer to improve visibility.

**Usage Guides:**

- Windows

  - [Text
         Size](https://support.microsoft.com/en-us/windows/change-the-size-of-text-in-windows-1d5830c3-eee3-8eaa-836b-abcc37d99b9a),
         [Text Cursor, Mouse
         Pointer](https://support.microsoft.com/en-us/windows/make-your-mouse-keyboard-and-other-input-devices-easier-to-use-10733da7-fa82-88be-0672-f123d4b3dcfe)

- MacOS

  - [Text
         Size](https://support.apple.com/guide/mac-help/text-items-screen-bigger-mchld786f2cd/mac),
         [Pointers](https://support.apple.com/guide/mac-help/pointers-in-macos-mh35695/mac)

- iOS

  - [Text
         Size](https://support.apple.com/guide/iphone/display-text-size-iph3e2e1fb0/ios),
         [Pointer](https://support.apple.com/guide/iphone/pointer-control-iphec6e1e60b/ios)

- Android

  - [Font
         size](https://support.google.com/accessibility/android/answer/11183305?hl=en),
         [Large Mouse
         Pointer](https://support.google.com/accessibility/android/answer/11183305?hl=en#zippy=%2Cuse-large-mouse-pointer)

- Linux

  - [Large
         Text](https://help.ubuntu.com/stable/ubuntu-help/a11y-font-size.html.en)

**Testing Scenarios:**

1. Change text size and verify every screen to ensure that text fits in
     the elements. Also verify this with VPN Client's localized
     languages like German, which typically have longer text.

2. Change the text cursor, enter text into text input fields and verify
     that the cursor has the correct attributes and moves to the
     correct position.

3. Any issues related to the mouse pointer are usually attributable to
     the platform or mouse driver.

### Vision - Ability to Turn Off Animation

Some users prefer to turn off animations to reduce distraction or visual
stimuli, for neurodivergence or for other reasons.

**Usage Guides:**

- Windows

  - [Animation
         Effects](https://mcmw.abilitynet.org.uk/how-to-turn-off-animations-in-windows-11)

- MacOS

  - [Reduce
         Motion](https://support.apple.com/guide/mac-help/reduce-screen-motion-mchlc03f57a1/mac)

- iOS

  - [Reduce
         Motion](https://support.apple.com/en-us/HT202655)

- Android

  - [Remove
         animations](https://mcmw.abilitynet.org.uk/how-to-disable-interface-animations-in-android-12-copy)

- Linux

  - [Enable
         Animations](https://www.omgubuntu.co.uk/2022/05/disable-animations-in-ubuntu-22-04)

**Testing Scenarios:**

1. Turn off animations, navigate to VPN Client screens that use
     animation and verify that animations are turned off. Verify again
     after animations are turned back on.

### Vision - Personalization of the Notification Duration

Some users need more time to read notifications that are auto-dismissed.
Many platforms allow the user to configure this duration.

**Usage Guides:**

- Windows

  - Use *Settings \ Accessibility \ Visual effects \ Dismiss
         notifications after this amount of time*

- MacOS

  - [Notification
         Style](https://support.apple.com/guide/mac-help/change-notifications-settings-mh40583/mac)

- iOS

  - *\[Is this supported?\]*

- Android

  - [Time to take
         action](https://support.google.com/accessibility/android/answer/9426889?hl=en)

- Linux

  - *\[Is this supported?\]*

**Testing Scenarios:**

1. Change the notification duration. Test that the VPN Client
     notifications which are raised during connection state changes use
     the new duration.

### Hearing - Cues for Audio Notification

For users with hearing impairments, having a visual or vibration cue
alongside audio notifications can be beneficial. Certain platforms
provide support for such cues.

**Usage Guides:**

- Windows

  - Use *Settings \ Accessibility \ Audio \ Flash my screen
         during audio notifications*

- MacOS

  - [Flash
         Screen](https://support.apple.com/guide/mac-help/change-audio-settings-for-accessibility-unac611/mac)

- iOS

  - [LED flash
         alerts](https://support.apple.com/en-us/HT210065),
         [Vibration](https://support.apple.com/en-il/HT208353)

- Android

  - [Vibration](https://support.google.com/accessibility/android/answer/9078946?hl=en)

- Linux

  - [Flash
         screen](https://help.ubuntu.com/stable/ubuntu-help/a11y-visualalert.html.en)

**Testing Scenarios:**

1. Change settings for Flash screen, LED flash or Vibration. Verify
     that these cues are used during VPN Client notifications which are
     raised with audio during its connection state changes.

### Mobility - Keyboard Access

Users who are unable to use the mouse or touch screen require full
keyboard access to all controls.

**Testing Scenarios:**

1. Connect a keyboard, if needed, and verify that Tab, Shift+Tab and
     arrow keys can be used to navigate to all controls in a screen.
     Verify that the Tab order is correct, direction of tabbing is
     correct and that tabs cycle through the screen.

2. Any issues related to Filter keys, that set the sensitivity of the
     keyboard, and Sticky keys, that allow multiple key shortcuts to be
     pressed one key at a time, are usually attributable to the
     platform, so need not be tested.

### Mobility - Voice Access, Voice Typing, Eye Control, Switch Control Support

These Accessibility Technologies are designed for users with mobility
impairments, enabling them to navigate and interact without a keyboard
or mouse.

Voice Access and Voice Typing are platform tools that are used to
navigate and input text using voice commands.

Eye Control utilizes a specialized Eye Tracking device to navigate and
input text through eye gaze.

Switch Control uses an adaptive device such as a single switch or
multiple switches, or a joystick to navigate and input text.

**Usage Guides:**

- Windows

  - [Voice
         Access](https://support.microsoft.com/en-us/topic/get-started-with-voice-access-bd2aa2dc-46c2-486c-93ae-3d75f7d053a4),
         [Eye
         Control](https://support.microsoft.com/en-us/windows/get-started-with-eye-control-in-windows-1a170a20-1083-2452-8f42-17a7d4fe89a9)

- MacOS

  - [Voice
         Control](https://support.apple.com/en-us/HT210539),
         [Dwell](https://support.apple.com/lv-lv/guide/mac-help/mchl437b47b0/mac),
         [Head
         Pointer](https://support.apple.com/lv-lv/guide/mac-help/mchlb2d4782b/mac),
         [Switch
         Control](https://support.apple.com/lv-lv/guide/mac-help/mh43607/13.0/mac/13.0)

- iOS

  - [Voice
         Control](https://support.apple.com/en-us/HT210417),
         [Head tracking and Switch
         Control](https://support.apple.com/en-us/HT201370)

- Android

  - [Voice
         Access](https://support.google.com/accessibility/android/answer/6151848?hl=en),
         [Eye
         Contro](https://support.google.com/accessibility/android/answer/11150722?hl=en)l,
         [Switch
         Access](https://support.google.com/accessibility/android/answer/6122836?hl=en)

- Linux

  - *\[Are these supported?\]*

**Testing Scenarios:**

1. These Accessibility Technologies (ATs) are designed to integrate
     with the core platform subsystems responsible for managing focus
     and input. Also, some of these ATs may utilize Accessibility information
     exposed by the VPN Client, which is also used by a screen reader. As
     a result, when the VPN Client has been tested and
     proven to be compatible with a screen reader, these ATs are
     expected to function with the VPN Client without the need for
     additional development work.
     Consequently, due to this shared compatibility and the varying
     hardware requirements, testing in these areas can be given a lower
     priority, and limited testing should generally be adequate.

### Neurodivergence/Do-not-disturb - Notification Suppression

In do-not-disturb mode, the VPN Client should not display system
notifications, to accommodate users who wish to maintain focus or have
specific neurodivergence requirements. The platform is expected to
automatically suppress such notifications in this mode.

**Usage Guides:**

- Windows

  - [Focus](https://support.microsoft.com/en-us/windows/how-to-use-focus-in-windows-11-cbcc9ddb-8164-43fa-8919-b9a2af072382)

- MacOS

  - [Focus](https://support.apple.com/guide/mac-help/turn-a-focus-on-or-off-mchl999b7c1a/mac)

- iOS

  - [Focus](https://support.apple.com/en-us/HT212608)

- Android

  - [Do Not
         Disturb](https://support.google.com/android/answer/9069335?sjid=3029603282803568608-NA)
         *\[Is flashing supported on notifications?\]*

- Linux

  - [Hiding
         notifications](https://help.ubuntu.com/stable/ubuntu-help/shell-notifications.html.en)

**Testing Scenarios:**

1. Change the do-not-disturb mode. Test that the VPN Client platform
     notifications which are raised during connection state changes are
     suppressed or shown based on the do-not-disturb setting. Any
     problems here are usually attributable to the platform.

## Accessibility Testing Tools

Certain platforms provide accessibility testing tools to help find
issues in the VPN Client. These tools can detect problems through
various means, such as highlighting potential issues when the user
hovers over an element or executing a comprehensive test suite to
compile a list of encountered problems.

**Test tools and usage guides:**

- Windows

  - [Accessibility
         Insights](https://accessibilityinsights.io/docs/windows/overview/)

    - [Live
             Inspect](https://accessibilityinsights.io/docs/windows/getstarted/inspect/)
             displays problems with the current element when the user
             hovers over it.

    - [FastPass](https://accessibilityinsights.io/docs/windows/getstarted/fastpass/)
             runs automated tests on an entire application screen and
             compiles a list of problems.

    - [FastPass](https://accessibilityinsights.io/docs/windows/getstarted/fastpass/)
             also assists with testing tab stops.

    - [Color Contrast
             Analyzer](https://accessibilityinsights.io/docs/windows/getstarted/colorcontrast/)
             detects text with contrast ratio that doesn't meet [WCAG
             standards](https://www.w3.org/TR/2008/REC-WCAG20-20081211/#visual-audio-contrast-contrast).

    - Accessibility events raised by the application can be
             monitored. Accessibility functionality exposed by the
             application can be run.

    - Usage Guides:
             [Documentation](https://accessibilityinsights.io/docs/windows/overview/),
             [Overview
             Video](https://www.youtube.com/watch?v=BIu9ONdMGGg),
             [Color Contrast Analyzer
             Video](https://www.youtube.com/watch?v=eEAYRBG4TOE)

- MacOS

  - [Accessibility Inspector and Accessibility
         Verifier](https://developer.apple.com/library/archive/documentation/Accessibility/Conceptual/AccessibilityMacOSX/OSXAXTestingApps.html)

- iOS

  - [Accessibility
         Inspector](https://developer.apple.com/library/archive/technotes/TestingAccessibilityOfiOSApps/TestingtheAccessibilityofiOSApps/TestingtheAccessibilityofiOSApps.html)

- Android

  - [Accessibility
         Scanner](https://developer.android.com/guide/topics/ui/accessibility/testing)

- Linux

  - [Testing
         Tools](https://wiki.ubuntu.com/Accessibility/Testing)
