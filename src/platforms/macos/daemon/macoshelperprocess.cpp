/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoshelperprocess.h"

#include <CoreFoundation/CoreFoundation.h>
#include <Security/SecCertificate.h>
#include <Security/SecRequirement.h>
#include <Security/SecStaticCode.h>
#include <Security/SecTask.h>

#include <QCoreApplication>
#include <QDir>
#include <QScopeGuard>
#include <QUrl>

#include "leakdetector.h"

MacOSHelperProcess::MacOSHelperProcess(const QString& name, QObject* parent) :
    QProcess(parent), m_logger(name) {
  MZ_COUNT_CTOR(MacOSHelperProcess);

  // The helper tool must exist in the same location as the daemon.
  setProgram(QDir(QCoreApplication::applicationDirPath()).filePath(name));

  // Set a handler to receive and log the tunnel stdout/stderr
  setProcessChannelMode(QProcess::MergedChannels);
  connect(this, &QProcess::readyReadStandardOutput, this,
          &MacOSHelperProcess::stdoutReady);

  m_codesignRequires = genericCodesignRequirements();
#ifdef MZ_DEBUG
  m_logger.debug() << "codesign requires:" << m_codesignRequires.join(" and ");
#endif
}

MacOSHelperProcess::~MacOSHelperProcess() {
  MZ_COUNT_DTOR(MacOSHelperProcess);
}

void MacOSHelperProcess::start() {
  m_codesignResult = enforceCodesign();
  if (m_codesignResult != errSecSuccess) {
    CFStringRef msg = SecCopyErrorMessageString(m_codesignResult, nullptr);
    m_logger.debug() << "codesign failed:" << msg;
    CFRelease(msg);
    emit errorOccurred(QProcess::FailedToStart);
    return;
  }

  QProcess::start();
}

QProcess::ProcessError MacOSHelperProcess::error() const {
  if (m_codesignResult != errSecSuccess) {
    return QProcess::FailedToStart;
  } else {
    return QProcess::error();
  }
}

void MacOSHelperProcess::stdoutReady() {
  for (;;) {
    QByteArray line = readLine();
    if (line.length() <= 0) {
      break;
    }
    m_logger.debug() << QString::fromUtf8(line);
  }
}

// static
QStringList MacOSHelperProcess::genericCodesignRequirements() {
  static QStringList requirements;
  if (!requirements.isEmpty()) {
    return requirements;
  }

  SecCodeRef code = nullptr;
  if (SecCodeCopySelf(kSecCSDefaultFlags, &code) != errSecSuccess) {
    return QStringList();
  }

  CFDictionaryRef dict = nullptr;
  OSStatus status = SecCodeCopySigningInformation(code, kSecCSSigningInformation, &dict);
  CFRelease(code);
  if (status != errSecSuccess) {
    return QStringList();
  }
  auto guard = qScopeGuard([dict]() { CFRelease(dict); });

  // Build the signing requirements.
  requirements << "anchor apple generic";
  CFTypeRef value = CFDictionaryGetValue(dict, kSecCodeInfoTeamIdentifier);
  if ((value != nullptr) && (CFGetTypeID(value) == CFStringGetTypeID())) {
    QString team = QString::fromCFString(static_cast<CFStringRef>(value));
    requirements << QString("certificate leaf[subject.OU] = \"%1\"").arg(team);
  }

  value = CFDictionaryGetValue(dict, kSecCodeInfoCertificates);
  if ((value != nullptr) && (CFGetTypeID(value) == CFArrayGetTypeID()) &&
      (CFArrayGetCount(static_cast<CFArrayRef>(value)) != 0)) {
    CFTypeRef leaf = CFArrayGetValueAtIndex(static_cast<CFArrayRef>(value), 0);
    if ((leaf != nullptr) && (CFGetTypeID(leaf) == SecCertificateGetTypeID())) {
      CFStringRef name;
      QString nameReqTemplate = "certificate leaf[subject.CN] = \"%1\"";
      SecCertificateCopyCommonName((SecCertificateRef)leaf, &name);
      requirements << nameReqTemplate.arg(QString::fromCFString(name));
      CFRelease(name);
    }
  }

  return requirements;
}

OSStatus MacOSHelperProcess::enforceCodesign() {
  if (m_codesignRequires.isEmpty()) {
    // If the daemon is not signed, then we shouldn't expect the helper tool to be.
    return errSecSuccess;
  }

  // Prepare the codesign requirements.
  OSStatus status = errSecSuccess;
  SecRequirementRef codesignRequirements = nullptr;
  CFStringRef codesignString = m_codesignRequires.join(" and ").toCFString();
  status = SecRequirementCreateWithString(codesignString, kSecCSDefaultFlags,
                                          &codesignRequirements);
  CFRelease(codesignString);
  if (status != errSecSuccess) {
    return status;
  }

  // Get the static code object
  SecStaticCodeRef code = nil;
  CFURLRef url = QUrl::fromLocalFile(program()).toCFURL();
  status = SecStaticCodeCreateWithPath(url, kSecCSDefaultFlags, &code);
  CFRelease(url);
  if (status != errSecSuccess) {
    CFRelease(codesignRequirements);
    return status;
  }

  // Verify the static code object.
  status = SecStaticCodeCheckValidityWithErrors(code, kSecCSDefaultFlags,
                                                codesignRequirements, nullptr);
  CFRelease(code);
  CFRelease(codesignRequirements);
  return status;
}
