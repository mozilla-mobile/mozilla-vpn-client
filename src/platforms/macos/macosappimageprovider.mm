/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosappimageprovider.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include <QIcon>
#include <QString>

#include "leakdetector.h"

MacOSAppImageProvider::MacOSAppImageProvider(QObject* parent)
    : AppImageProvider(parent, QQuickImageProvider::Image,
                       QQmlImageProviderBase::ForceAsynchronousImageLoading) {
  MZ_COUNT_CTOR(MacOSAppImageProvider);
}

MacOSAppImageProvider::~MacOSAppImageProvider() {
  MZ_COUNT_DTOR(MacOSAppImageProvider);
}

static void dataCleanupCallback(void* ctx) {
  CFRelease(ctx);
}

static QImage::Format parseImageFormat(CGImageRef image) {
  //CGBitmapInfo info = CGImageGetBitmapInfo(image);
  CGImageAlphaInfo alpha = CGImageGetAlphaInfo(image);
  CGImagePixelFormatInfo pixfmt = CGImageGetPixelFormatInfo(image);

  // Common case - 8-bits per component... typically one of the RGB formats.
  if ((CGImageGetBitsPerComponent(image) == 8) &&
      (pixfmt == kCGImagePixelFormatPacked)) {
    switch (alpha) {
      case kCGImageAlphaFirst:
        return QImage::Format_ARGB32;
      case kCGImageAlphaLast:
        return QImage::Format_RGBA8888;
      case kCGImageAlphaNone:
        return QImage::Format_RGB888;
      case kCGImageAlphaNoneSkipFirst:
        return QImage::Format_RGB32;
      case kCGImageAlphaOnly:
        return QImage::Format_Alpha8;
      case kCGImageAlphaPremultipliedFirst:
        return QImage::Format_ARGB32_Premultiplied;
      case kCGImageAlphaPremultipliedLast:
        return QImage::Format_RGBA8888_Premultiplied;
      
      case kCGImageAlphaNoneSkipLast:
       // There doesn't seem to be a Qt equivalent...
       // QImage::Format_RGBX8888 maybe?
       [[fallthrough]];
      default:
        return QImage::Format_Invalid;
    }
  }

  // Check for odd pixel formats.
  switch (pixfmt) {
    case kCGImagePixelFormatRGB555:
      return QImage::Format_RGB555;
    case kCGImagePixelFormatRGB565:
      return QImage::Format_RGB16;
    case kCGImagePixelFormatRGB101010:
      if (alpha == kCGImageAlphaPremultipliedFirst) {
        return QImage::Format_A2RGB30_Premultiplied;
      } else {
        // FIXME: Theoretically there can be alpha data in the 2msb but we have
        // no corresponding Qt format for it.
        return QImage::Format_RGB30;
      }
    case kCGImagePixelFormatRGBCIF10:
      return QImage::Format_RGB30;
    default:
      break;
  }

  // TODO: It's not clear how to handle kCGBitmapFloatComponents

  // If we got this far, we cannot convert the pixel format.
  return QImage::Format_Invalid;
}

// from QQuickImageProvider
QImage MacOSAppImageProvider::requestImage(const QString& id, QSize* size,
                                           const QSize& requestedSize) {

  // Find the application bundle.
  NSBundle* bundle = [NSBundle bundleWithIdentifier: id.toNSString()];
  if (bundle == nil) {
    return QImage();
  }

  // Get the application's CFBundleIconName key.
  NSString* iconName = [bundle objectForInfoDictionaryKey:@"CFBundleIconName"];
  if (iconName == nil) {
    return QImage();
  }
  NSImage* icon = [bundle imageForResource:iconName];
  if (icon == nil) {
    return QImage();
  }

  CGRect region = CGRectMake(0, 0, requestedSize.width(), requestedSize.height());
  CGImageRef image = [icon CGImageForProposedRect:&region context:nil hints:nil];
  if (!image) {
    return QImage();
  }
  auto guard = qScopeGuard([image]() { CFRelease(image); });
  size->setWidth(CGImageGetWidth(image));
  size->setHeight(CGImageGetHeight(image));

  // Copy the image data.
  CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(image));
  return QImage((const uchar*)CFDataGetBytePtr(data), size->width(),
                size->height(), CGImageGetBytesPerRow(image),
                parseImageFormat(image), dataCleanupCallback, (void*)data);
}
