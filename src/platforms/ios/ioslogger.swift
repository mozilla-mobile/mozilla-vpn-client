/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import os.log
import OSLog

public class IOSLoggerImpl : NSObject {
    enum LogType {
        case swift
        case networkExtension

        var filename: String {
            switch self {
            case .swift: return "mozillavpnswift.log"
            case .networkExtension: return "networkextension.log"
            }
        }

        var fileUrl: URL? {
            let containerURL = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: Constants.appGroupIdentifier)
            return containerURL?.appendingPathComponent(self.filename, isDirectory: false)
        }

        var newFileText: String {
            switch self {
            case .swift: return "\n\nApp Swift Logs\n==============\n"
            case .networkExtension: return ""
            }
        }
    }

    @objc enum MzLogLevel: Int {
        case debug, info, error

        var swiftVersion: OSLogType {
            switch self {
            case .debug: return .debug
            case .info: return .info
            case .error: return .error
            }
        }
    }

    private let log: OSLog
    
    private lazy var dateFormatter: DateFormatter = {
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "dd.MM.yyyy HH:mm:ss"
        return dateFormatter
    }()
    
    private static let logger = IOSLoggerImpl(tag: "IOSLoggerImpl")

    @objc init(tag: String) {
        self.log = OSLog(
            subsystem: Bundle.main.bundleIdentifier!,
            category: tag
        )
    }

    @objc func logToConsole(message: String, level: MzLogLevel) {
        os_log("%{public}@", log: self.log, type: level.swiftVersion, message)
    }

    func debug(message: String) {
        log(message, type: .debug)
    }

    func info(message: String) {
        log(message, type: .info)
    }

    func error(message: String) {
        log(message, type: .error)
    }

    func log(_ message: String, type: OSLogType) {
        os_log("%{public}@", log: self.log, type: type, message)

        let currentDate = Date()
        let formattedDateString = dateFormatter.string(from: currentDate)

        if let data = "[\(formattedDateString)] \(message)\n".data(using: .utf8) {
            let fileType: LogType = Bundle.main.bundlePath.hasSuffix(".appex") ? .networkExtension : .swift
            let _ = IOSLoggerImpl.withLogFile(for: .fileType) { logFileHandle in
                logFileHandle.seekToEndOfFile()
                logFileHandle.write(data)
            }
        }
    }

    @objc static func getLogs(callback: @escaping (String) -> Void) {
        var returnLogs: String = ""
        [FileType.networkExtension, FileType.swift].forEach {
            IOSLoggerImpl.withLogFile(for: $0) { logFileHandle in
                if let contents = String(data: logFileHandle.readDataToEndOfFile(), encoding: .utf8) {
                    returnLogs.append(contents)
                }
            }
        }
        callback(returnLogs);
    }
    
    @objc static func clearLogs() {
        [FileType.networkExtension, FileType.swift].forEach {
            IOSLoggerImpl.withLogFile(for: $0) { logFileHandle in
                logFileHandle.truncateFile(atOffset: 0)
            }
        }
    }

    private static func withLogFile(for type: LogType, _ f: (_ handle: FileHandle) throws -> Void) {
        guard let logFileURL = type.fileUrl else {
            logger.error(message: "IMPOSSIBLE: No known log file.")
            return
        }

        do {
            if !FileManager.default.fileExists(atPath: logFileURL.path) {
                // Create an empty file with appropriate headers
                if let data = type.newFileText.data(using: .utf8) {
                    try data.write(to: logFileURL)
                } else {
                    logger.error(message: "Unable to create log file at \(logFileURL)")
                    return
                }
            }

            let fileHandle = try FileHandle(forUpdating: logFileURL)
            try f(fileHandle)
            fileHandle.closeFile()
        } catch {
            logger.error(message: "Unable to access log file at \(logFileURL): \(error)")
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

