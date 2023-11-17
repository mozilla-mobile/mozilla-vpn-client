/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import os.log
import OSLog

extension OSLogType {
    func toString() -> String {
        switch self {
        case .info:
            return "Info"
        case .debug:
            return "Debug"
        case .error:
            return "Error"
        default:
            return "Unknown"
        }
    }
}

public class IOSLoggerImpl : NSObject {
    private let log: OSLog
    private let tag: String

    private lazy var dateFormatter: DateFormatter = {
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "dd.MM.yyyy HH:mm:ss"
        return dateFormatter
    }()

    private static let logger = IOSLoggerImpl(tag: "IOSLoggerImpl")
    private static var appexLogFileURL: URL? {
        get {
            guard let containerURL = FileManager.default.containerURL(
                forSecurityApplicationGroupIdentifier: Constants.appGroupIdentifier
            ) else {
                return nil
            }

            return containerURL.appendingPathComponent(Constants.networkExtensionLogFileName, isDirectory: false)
        }
    }

    @objc init(tag: String) {

        self.tag = tag
        self.log = OSLog(
            subsystem: Bundle.main.bundleIdentifier!,
            category: tag
        )
    }

    @objc func debug(message: String) {
        log(message, type: .debug)
    }

    @objc func info(message: String) {
        log(message, type: .info)
    }

    @objc func error(message: String) {
        log(message, type: .error)
    }

    func log(_ message: String, type: OSLogType) {
        NSLog("(\(tag)) \(type.toString()): \(message)")

        if (Bundle.main.bundlePath.hasSuffix(".appex")) {
            let currentDate = Date()
            let formattedDateString = dateFormatter.string(from: currentDate)

            if let data = "[\(formattedDateString)] (\(tag)) \(type.toString()): \(message)\n".data(using: .utf8) {
                let _ = IOSLoggerImpl.withAppexLogFile { logFileHandle in
                    logFileHandle.seekToEndOfFile()
                    logFileHandle.write(data)
                }
            }
        }
    }

    @objc static func getAppexLogs(callback: @escaping (String) -> Void) {
        withAppexLogFile { logFileHandle in
            if let contents = String(data: logFileHandle.readDataToEndOfFile(), encoding: .utf8) {
                callback(contents);
            }
        }
    }

    @objc static func clearAppexLogs() {
        withAppexLogFile { logFileHandle in
            logFileHandle.truncateFile(atOffset: 0)
        }
    }

    private static func withAppexLogFile(_ f: (_ handle: FileHandle) throws -> Void) {
        guard let appexLogFileURL = IOSLoggerImpl.appexLogFileURL else {
            logger.error(message: "IMPOSSIBLE: No known app extension log file.")
            return
        }


        do {
            if !FileManager.default.fileExists(atPath: appexLogFileURL.path) {
                // Create an empty file
                if let data = "".data(using: .utf8) {
                    try data.write(to: appexLogFileURL)
                } else {
                    logger.error(message: "Unable to create log file at \(appexLogFileURL)")
                    return
                }
            }

            let fileHandle = try FileHandle(forUpdating: appexLogFileURL)
            try f(fileHandle)
            fileHandle.closeFile()
        } catch {
            logger.error(message: "Unable to access log file at \(appexLogFileURL): \(error)")
        }
    }
}

// The following functions are used by Wireguard internally for logging.

let wireguardLogger = IOSLoggerImpl(tag: "Wireguard")

func wg_log(_ type: OSLogType, staticMessage: StaticString) {
    wireguardLogger.log("\(staticMessage)", type: type)
}

func wg_log(_ type: OSLogType, message: String) {
    wireguardLogger.log(message, type: type)
}
