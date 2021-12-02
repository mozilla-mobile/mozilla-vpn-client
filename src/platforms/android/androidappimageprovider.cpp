/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidappimageprovider.h"
#include "logger.h"
#include "leakdetector.h"
#include "androidjnicompat.h"
#include "androidutils.h"

#include <jni.h>
#include <android/bitmap.h>


namespace {
Logger logger(LOG_CONTROLLER, "AndroidAppImageProvider");
}

AndroidAppImageProvider::AndroidAppImageProvider(QObject* parent)
    : AppImageProvider(parent, QQuickImageProvider::Image,
                       QQmlImageProviderBase::ForceAsynchronousImageLoading) {
  MVPN_COUNT_CTOR(AndroidAppImageProvider);
}

AndroidAppImageProvider::~AndroidAppImageProvider() {
  MVPN_COUNT_DTOR(AndroidAppImageProvider);
}

// from QQuickImageProvider
QImage AndroidAppImageProvider::requestImage(const QString& id, QSize* size,
                                             const QSize& requestedSize) {
  QJniObject activity = AndroidUtils::getActivity();
  Q_ASSERT(activity.isValid());

  auto jniString = QJniObject::fromString(id);

  logger.debug() << " Request image";

  QJniObject drawable = QJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "getAppIcon",
      "(Landroid/content/Context;Ljava/lang/String;)Landroid/graphics/drawable/"
      "Drawable;",
      activity.object(), jniString.object());

  int width = drawable.callMethod<int>("getIntrinsicWidth");
  int height = drawable.callMethod<int>("getIntrinsicHeight");
  size->setHeight(height);
  size->setWidth(width);

  if (requestedSize.height() > 0 && requestedSize.width() > 0) {
    width = requestedSize.width();
    height = requestedSize.height();
  }

  QImage out = toImage(drawable, QRect(0, 0, width, height));
  logger.debug() << "Created image w" << out.size().width() << "  h "
                 << out.size().height();
  return out;
}

QImage AndroidAppImageProvider::toImage(const QJniObject& bitmap) {
  QJniEnvironment env;
  AndroidBitmapInfo info;

  #if QT_VERSION < 0x060000
    auto res = AndroidBitmap_getInfo(env, bitmap.object(), &info);
  #else 
    auto res = AndroidBitmap_getInfo(env.jniEnv(), bitmap.object(), &info);
  #endif

  if (res != ANDROID_BITMAP_RESULT_SUCCESS)
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
  #if QT_VERSION < 0x060000
    res = AndroidBitmap_lockPixels(env, bitmap.object(), &pixels);
  #else 
    res = AndroidBitmap_lockPixels(env.jniEnv(), bitmap.object(), &pixels);
  #endif


  if (res != ANDROID_BITMAP_RESULT_SUCCESS)
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

  #if QT_VERSION < 0x060000
    res = AndroidBitmap_unlockPixels(env, bitmap.object());
  #else 
    res = AndroidBitmap_unlockPixels(env.jniEnv(), bitmap.object());
  #endif

  if (res != ANDROID_BITMAP_RESULT_SUCCESS)
    return QImage();

  return image;
}

QJniObject AndroidAppImageProvider::createBitmap(int width, int height) {
  QJniObject config = QJniObject::getStaticObjectField(
      "android/graphics/Bitmap$Config", "ARGB_8888",
      "Landroid/graphics/Bitmap$Config;");

  return QJniObject::callStaticObjectMethod(
      "android/graphics/Bitmap", "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;", width,
      height, config.object());
}

QImage AndroidAppImageProvider::toImage(const QJniObject& drawable,
                                        const QRect& bounds) {
  QJniObject bitmap = createBitmap(bounds.width(), bounds.height());
  QJniObject canvas("android/graphics/Canvas", "(Landroid/graphics/Bitmap;)V",
                    bitmap.object());
  drawable.callMethod<void>("setBounds", "(IIII)V", bounds.left(), bounds.top(),
                            bounds.right(), bounds.bottom());
  drawable.callMethod<void>("draw", "(Landroid/graphics/Canvas;)V",
                            canvas.object());
  return toImage(bitmap);
}
