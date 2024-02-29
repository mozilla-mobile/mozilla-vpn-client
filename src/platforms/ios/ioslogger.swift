/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import os.log
import OSLog

public class IOSLoggerImpl : NSObject {
    private let log: OSLog
    
    private lazy var dateFormatter: DateFormatter = {
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "dd.MM.yyyy HH:mm:ss"
        return dateFormatter
    }()
    
    private static let logger = IOSLoggerImpl(tag: "IOSLoggerImpl")
    private static var appexLogFileURL: URL? {
        get {
            let containerURL = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: Constants.appGroupIdentifier)
            return containerURL?.appendingPathComponent(Constants.networkExtensionLogFileName, isDirectory: false)
        }
    }

    private static var appSwiftLogFileURL: URL? {
        get {
            let containerURL = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: Constants.appGroupIdentifier)
            return containerURL?.appendingPathComponent(Constants.appLogFileName, isDirectory: false)
        }
    }

    @objc init(tag: String) {
        self.log = OSLog(
            subsystem: Bundle.main.bundleIdentifier!,
            category: tag
        )
    }

    @objc func debugToConsole(message: String) {
        os_log("%{public}@", log: self.log, type: .debug, message)
    }

    @objc func infoToConsole(message: String) {
        os_log("%{public}@", log: self.log, type: .info, message)
    }

    @objc func errorToConsole(message: String) {
        os_log("%{public}@", log: self.log, type: .error, message)
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
            if (Bundle.main.bundlePath.hasSuffix(".appex")) {
                let _ = IOSLoggerImpl.withAppexLogFile { logFileHandle in
                    logFileHandle.seekToEndOfFile()
                    logFileHandle.write(data)
                }
            } else {
                let _ = IOSLoggerImpl.withAppSwiftLogFile { logFileHandle in
                    logFileHandle.seekToEndOfFile()
                    logFileHandle.write(data)
                }
            }
        }
    }

    @objc static func getAppexLogs(callback: @escaping (String) -> Void) {
        var backendLogs: String = ""
        var swiftLogs: String = ""

        withAppexLogFile { logFileHandle in
            if let contents = String(data: logFileHandle.readDataToEndOfFile(), encoding: .utf8) {
                backendLogs = contents
            }
        }
        withAppSwiftLogFile { logFileHandle in
            if let contents = String(data: logFileHandle.readDataToEndOfFile(), encoding: .utf8) {
                swiftLogs = contents
            }
        }
        callback(backendLogs + swiftLogs);
    }
    
    @objc static func clearAppexLogs() {
        withAppexLogFile { logFileHandle in
            logFileHandle.truncateFile(atOffset: 0)
        }

        withAppSwiftLogFile { logFileHandle in
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

    private static func withAppSwiftLogFile(_ f: (_ handle: FileHandle) throws -> Void) {
        guard let appSwiftLogFileURL = IOSLoggerImpl.appSwiftLogFileURL else {
            logger.error(message: "IMPOSSIBLE: No known app log file.")
            return
        }

        do {
            if !FileManager.default.fileExists(atPath: appSwiftLogFileURL.path) {
                // Create an empty file with appropriate headers
                if let data = "\n\nApp Swift Logs\n==============\n".data(using: .utf8) {
                    try data.write(to: appSwiftLogFileURL)
                } else {
                    logger.error(message: "Unable to create log file at \(appSwiftLogFileURL)")
                    return
                }
            }

            let fileHandle = try FileHandle(forUpdating: appSwiftLogFileURL)
            try f(fileHandle)
            fileHandle.closeFile()
        } catch {
            logger.error(message: "Unable to access log file at \(appSwiftLogFileURL): \(error)")
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

