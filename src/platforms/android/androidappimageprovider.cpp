/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidappimageprovider.h"
#include "logger.h"
#include "leakdetector.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#include <jni.h>
#include <android/bitmap.h>

namespace {
Logger logger(LOG_CONTROLLER, "AndroidAppImageProvider");
}

AndroidAppImageProvider::AndroidAppImageProvider(QObject* parent)
    : QQuickImageProvider(QQuickImageProvider::Image,
                          QQmlImageProviderBase::ForceAsynchronousImageLoading),
      QObject(parent) {
  MVPN_COUNT_CTOR(AndroidAppImageProvider);
}

AndroidAppImageProvider::~AndroidAppImageProvider() {
  MVPN_COUNT_DTOR(AndroidAppImageProvider);
}

// from QQuickImageProvider
QImage AndroidAppImageProvider::requestImage(const QString& id, QSize* size,
                                             const QSize& requestedSize) {
  QAndroidJniObject activity = QtAndroid::androidActivity();
  Q_ASSERT(activity.isValid());

  auto jniString = QAndroidJniObject::fromString(id);

  logger.log() << " Request image";

  QAndroidJniObject drawable = QAndroidJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "getAppIcon",
      "(Landroid/content/Context;Ljava/lang/String;)Landroid/graphics/drawable/"
      "Drawable;",
      activity.object(), jniString.object());

  int width = drawable.callMethod<int>("getIntrinsicWidth");
  int height = drawable.callMethod<int>("getIntrinsicHeight");
  size->setHeight(height);
  size->setWidth(width);

  if (requestedSize.isValid()) {
    width = requestedSize.width();
    height = requestedSize.height();
  }

  QImage out = toImage(drawable, QRect(0, 0, width, height));
  logger.log() << "Created image w" << out.size().width() << "  h "
               << out.size().height();
  return out;
}

QImage AndroidAppImageProvider::toImage(const QAndroidJniObject& bitmap) {
  QAndroidJniEnvironment env;
  AndroidBitmapInfo info;
  if (AndroidBitmap_getInfo(env, bitmap.object(), &info) !=
      ANDROID_BITMAP_RESULT_SUCCESS)
    return QImage();

  QImage::Format format;
  switch (info.format) {
    case ANDROID_BITMAP_FORMAT_RGBA_8888:
      format = QImage::Format_RGBA8888;
      break;
    case ANDROID_BITMAP_FORMAT_RGB_565:
      format = QImage::Format_RGB16;
      break;
    case ANDROID_BITMAP_FORMAT_RGBA_4444:
      format = QImage::Format_ARGB4444_Premultiplied;
      break;
    case ANDROID_BITMAP_FORMAT_A_8:
      format = QImage::Format_Alpha8;
      break;
    default:
      return QImage();
  }

  void* pixels;
  if (AndroidBitmap_lockPixels(env, bitmap.object(), &pixels) !=
      ANDROID_BITMAP_RESULT_SUCCESS)
    return QImage();

  QImage image(info.width, info.height, format);

  if (info.stride == uint32_t(image.bytesPerLine())) {
    memcpy((void*)image.constBits(), pixels, info.stride * info.height);
  } else {
    uchar* bmpPtr = static_cast<uchar*>(pixels);
    const unsigned width = std::min(info.width, (uint)image.width());
    const unsigned height = std::min(info.height, (uint)image.height());
    for (unsigned y = 0; y < height; y++, bmpPtr += info.stride)
      memcpy((void*)image.constScanLine(y), bmpPtr, width);
  }

  if (AndroidBitmap_unlockPixels(env, bitmap.object()) !=
      ANDROID_BITMAP_RESULT_SUCCESS)
    return QImage();

  return image;
}

QAndroidJniObject AndroidAppImageProvider::createBitmap(int width, int height) {
  QAndroidJniObject config = QAndroidJniObject::getStaticObjectField(
      "android/graphics/Bitmap$Config", "ARGB_8888",
      "Landroid/graphics/Bitmap$Config;");

  return QAndroidJniObject::callStaticObjectMethod(
      "android/graphics/Bitmap", "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;", width,
      height, config.object());
}

QImage AndroidAppImageProvider::toImage(const QAndroidJniObject& drawable,
                                        const QRect& bounds) {
  QAndroidJniObject bitmap = createBitmap(bounds.width(), bounds.height());
  QAndroidJniObject canvas("android/graphics/Canvas",
                           "(Landroid/graphics/Bitmap;)V", bitmap.object());
  drawable.callMethod<void>("setBounds", "(IIII)V", bounds.left(), bounds.top(),
                            bounds.right(), bounds.bottom());
  drawable.callMethod<void>("draw", "(Landroid/graphics/Canvas;)V",
                            canvas.object());
  return toImage(bitmap);
}
