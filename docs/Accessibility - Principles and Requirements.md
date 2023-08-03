# Mozilla VPN Accessibility -- Principles and Requirements

## Summary

This document outlines principles and requirements for ensuring
accessibility in Mozilla VPN. For more information about these
requirements and testing procedures, please refer to [Mozilla VPN
Accessibility \-- Resources for
Testing](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/docs/Accessibility%20-%20Testing%20Resources.md).

## Motivation

Accessibility is an important feature that ensures that all users,
including those with disabilities, can use the Mozilla VPN Client
effectively. Following Accessibility principles and requirements can
enhance the user experience, reach a broader audience, and comply with
accessibility standards across all supported platforms.

## Principles

1. **Inclusive design:** The VPN Client should be designed for use by
      all people, as far as possible, without the need for specialized
      adaptations. By considering the diverse needs of users from the
      start, we can create a more inclusive and higher quality user
      experience.

2. **Compatibility with major Assistive Technologies (ATs):** The VPN
      Client should be compatible with the major ATs of the platforms
      that it runs on, including Text Size, Screen Readers, Keyboard
      Access, Text Contrast, Screen Magnification, and others.

3. **Alignment with Established Standards:** The VPN Client should try
      to meet accessibility standards like
      [WCAG](https://www.w3.org/WAI/standards-guidelines/wcag/)
      where applicable, even though the VPN Client is not a Web App.

4. **Foster Collaborative Development and Testing:** Promote a
      collaborative approach among developers, testers and accessibility
      experts during the development and testing of Accessibility
      features in the VPN client. Test with real users. This will result
      in higher quality implementations, continuous improvement and
      early detection of accessibility issues.

## Accessibility Requirements for VPN Client

For more information about these requirements, testing, and
prioritization principles and guidelines, please refer to [Mozilla VPN
Accessibility \-- Resources for
Testing](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/docs/Accessibility%20-%20Testing%20Resources.md).

### High Priority Requirements

1. **Text Resizing:**

     - Users should be able to adjust the text size through the platform\'s
      settings while ensuring that the application text remains readable
      and all functionality remains intact.

2. **Screen Reader Compatibility:**

     - All screens must be compatible with major screen readers, allowing
      users with vision impairments to access the content and obtain
      information about interactive elements.

     - All screens must be navigable using Screen Reader keyboard
      shortcuts.

3. **Keyboard Access:**

     - All functionality and interactive elements must be accessible solely
      using a keyboard, without relying on mouse or touch interactions.

4. **Text Contrast Compliance:**

     - The default text contrast against its background must meet the
      standards set by
      [WCAG](https://www.w3.org/TR/2008/REC-WCAG20-20081211/#visual-audio-contrast-contrast).

5. **High Contrast Themes and Color Filter Compatibility:**

     - Users with vision impairments should be able to apply the
      platform\'s High Contrast themes or Color Filters without
      compromising readability within the application.

6. **Screen Magnification:**

     - All screens must be readable when using the screen magnification
      tool .

     - Any focused element should automatically scroll into the
      magnification tool\'s viewport.

7. **Animation Suppression:**

     - If the user has disabled animations, the application should suppress
      them accordingly, supporting users who want to reduce distraction
      or visual stimuli, for neurodivergence or for other reasons.

### Platform-Handled Accessibility Requirements: Lower Priority Due to Platform Support

These requirements are generally handled by the platform, and additional
work is typically not needed in the VPN Client, provided that the
application has been tested and proven to be compatible with a screen
reader.

1. **Text Cursor and Mouse Pointer Compatibility:**

     - Editable elements must support text cursor personalizations, such as
      size, shape, and color, to enhance visibility for users.

     - The application should work well with the user\'s personalized mouse
      cursor.

2. **Auto-Dismiss Notification Duration:**

     - Auto-dismissed notifications must accommodate user-configured
      durations, allowing users to have sufficient time to read them.

3. **Audio Notification Cues:**

     - For users with hearing impairments, personalized visual or vibration
      cues should accompany audio notifications.

4. **Notification Suppression:**

     - System notifications should not be displayed if the user has opted
      to turn them off to maintain focus or for neurodivergence
      requirements.

5. **Alternate Navigation and Input Modes:**

     - The application must support alternate modes for navigation and
      input, such as Voice Access, Voice Typing, Eye Control, and Switch
      Control, to meet the needs of users with mobility impairments.
