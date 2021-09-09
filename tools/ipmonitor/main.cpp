/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>
#include <QScopeGuard>

#include <net/ethernet.h>
#ifdef MVPN_LINUX
#  include <netinet/ether.h>
#elif MVPN_MACOS
#  include <netinet/if_ether.h>
#endif
#include <pcap.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#include "rfc/rfc1918.h"

// IP header. See https://www.tcpdump.org/pcap.html
struct sniff_ip {
  u_char ip_vhl;
  u_char ip_tos;
  u_short ip_len;
  u_short ip_id;
  u_short ip_off;
  u_char ip_ttl;
  u_char ip_p;
  u_short ip_sum;
  in_addr ip_src, ip_dst;
#define IP_HL(ip) (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip) (((ip)->ip_vhl) >> 4)
};

class Output final : public QTextStream {
 public:
  enum Preset {
    eNone,
    eDone,
    eError,
    eInfo,
  };

  Output(Preset preset = eNone) : QTextStream(&m_buffer) {
    switch (preset) {
      case eNone:
        break;
      case eDone:
        *this << Output::bold() << Output::green();
        break;
      case eError:
        *this << Output::bold() << Output::red();
        break;
      case eInfo:
        *this << Output::yellow();
        break;
    }
  }

  ~Output() {
    // Reset of the sequences.
    *this << reset();
    QTextStream(stdout) << m_buffer;
  }

  void cleanTo(uint32_t pos) {
    for (uint32_t i = m_buffer.length(); i < pos; ++i) {
      m_buffer.append(" ");
    }
  }

  static QString fixSize(const QString& input, uint32_t size) {
    QString buffer;
    for (uint32_t i = input.length(); i < size; ++i) {
      buffer.append(" ");
    }
    buffer.append(input);
    return buffer;
  }

  static const char* reset() { return "\033[0m"; }

  static const char* hideCursor() { return "\033[?25l"; }
  static const char* showCursor() { return "\033[?25h"; }

  static const char* prevLine() { return "\033[1F"; }

  static const char* bold() { return "\033[1m"; }
  static const char* italic() { return "\033[3m"; }
  static const char* underline() { return "\033[4m"; }

  static const char* red() { return "\033[31m"; }
  static const char* green() { return "\033[32m"; }
  static const char* yellow() { return "\033[33m"; }
  static const char* blue() { return "\033[34m"; }
  static const char* magenta() { return "\033[35m"; }
  static const char* cyan() { return "\033[36m"; }

 private:
  QString m_buffer;
};

struct Server final {
  QString countryName;
  QString cityName;
  QString ipv4AddrIn;
};

bool parseServerListFile(const QString& fileName,
                         QHash<QString, Server*>& list) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    Output(Output::eError) << "Failed to open the file";
    return false;
  }

  QByteArray buffer = file.readAll();
  QJsonDocument json = QJsonDocument::fromJson(buffer);
  if (!json.isArray()) {
    Output(Output::eError)
        << "The file is not valid. A JSON array is expected.";
    return false;
  }

  for (QJsonValue countryValue : json.array()) {
    QJsonObject countryObj = countryValue.toObject();
    QString countryName = countryObj["name"].toString();
    for (QJsonValue cityValue : countryObj["cities"].toArray()) {
      QJsonObject cityObj = cityValue.toObject();
      QString cityName = cityObj["name"].toString();
      for (QJsonValue serverValue : cityObj["servers"].toArray()) {
        QJsonObject serverObj = serverValue.toObject();
        QString ipv4AddrIn = serverObj["ipv4-addr-in"].toString();
        list.insert(ipv4AddrIn, new Server{countryName, cityName, ipv4AddrIn});
      }
    }
  }

  return true;
}

void logPackage(QFile* logger, const QString& ipSrc, const QString& ipDst) {
  QTextStream out(logger);
  out << QDateTime::currentDateTime().toString() << " - Src: " << ipSrc
      << " - Dest: " << ipDst << Qt::endl;
}

int32_t stdinToInt32() {
  QTextStream in(stdin);
  bool ok = false;
  int32_t id = in.readLine().toInt(&ok, 10);
  if (!ok) return -1;
  return id;
}

int main(int argc, char* argv[]) {
  char error[PCAP_ERRBUF_SIZE];

  QCoreApplication app(argc, argv);

  QCoreApplication::setApplicationName("Mozilla VPN - IP Monitor");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  QCommandLineParser parser;
  parser.setApplicationDescription("Mozilla VPN - IP Monitor");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("server.list", "The server list file");

  QCommandLineOption logFileOption(
      "l", "Log non-VPN and non-LAN packages into this file", "file");
  parser.addOption(logFileOption);

  parser.process(app);

  Output() << Output::bold() << QCoreApplication::applicationName() << " "
           << QCoreApplication::applicationVersion() << Qt::endl
           << Qt::endl;

  const QStringList args = parser.positionalArguments();
  if (args.isEmpty()) {
    Output(Output::eError) << "Please, the first argument is required."
                           << Qt::endl;
    Output() << "To generate the server list, run: mozillavpn servers -j > "
                "server.list"
             << Qt::endl;
    return 1;
  }

  if (!QFile::exists(args[0])) {
    Output(Output::eError)
        << "Please, the first argument must be the path of the server "
           "list file."
        << Qt::endl;
    return 1;
  }

  QFile* logger = nullptr;
  auto loggerGuard = qScopeGuard([&] {
    if (logger) {
      logger->close();
      delete logger;
    }
  });

  QString logFile = parser.value(logFileOption);
  if (!logFile.isEmpty()) {
    Output(Output::eInfo) << "Opening log file: ";
    logger = new QFile(logFile);
    if (!logger->open(QIODevice::WriteOnly | QIODevice::Text)) {
      Output(Output::eError) << "Failed to open the file";
      return 1;
    }
    Output(Output::eDone) << "done." << Qt::endl;
  }

  QHash<QString, Server*> servers;
  Output(Output::eInfo) << "Processing the list of servers: ";
  if (!parseServerListFile(args[0], servers)) {
    return 1;
  }
  if (servers.isEmpty()) {
    Output(Output::eError) << "No servers found in the server list file."
                           << Qt::endl;
    return 1;
  }
  Output(Output::eDone) << servers.count() << " servers." << Qt::endl;

  Output(Output::eInfo) << "Retrieving the interface list: ";
  pcap_if_t* interfaces = nullptr;
  if (pcap_findalldevs(&interfaces, error) == -1) {
    Output(Output::eError) << "Error in pcap findall devs" << error << Qt::endl;
    return 1;
  }
  Output(Output::eDone) << "done." << Qt::endl;

  auto interfacesGuard = qScopeGuard([&] { pcap_freealldevs(interfaces); });

  Output(Output::eInfo) << "The interfaces present on the system are:"
                        << Qt::endl;

  int i = 0;
  for (pcap_if_t* temp = interfaces; temp; temp = temp->next) {
    if (!(temp->flags & PCAP_IF_UP) || !(temp->flags & PCAP_IF_RUNNING))
      continue;

    if ((temp->flags & PCAP_IF_LOOPBACK)) continue;

    Output() << " " << ++i << " " << Output::cyan() << temp->name
             << Output::reset() << " - " << temp->description << Qt::endl;
  }

  Output(Output::eInfo) << "Which interface do you want to use? "
                        << Output::reset();
  int32_t id = stdinToInt32();
  if (id > i || id < 1) {
    Output(Output::eError) << "Sorry. I don't understand your answer."
                           << Qt::endl;
    return 1;
  }

  char* device = nullptr;
  QStringList addresses;
  for (pcap_if_t* temp = interfaces; temp; temp = temp->next) {
    if (--id == 0) {
      device = temp->name;
      for (pcap_addr* address = temp->addresses; address;
           address = address->next) {
        if (address->addr->sa_family != AF_INET) {
          continue;
        }
        const sockaddr_in* sin = (const sockaddr_in*)address->addr;
        addresses.append(inet_ntoa(sin->sin_addr));
      }
      break;
    }
  }

  Output(Output::eInfo) << "Opening device '" << device << "' ("
                        << addresses.join(",") << "): ";
  pcap_t* handle = pcap_open_live(device, BUFSIZ, 1, 1, error);
  if (handle == NULL) {
    Output(Output::eError) << "Couldn't open device: " << error << Qt::endl;
    return 1;
  }
  Output(Output::eDone) << "done." << Qt::endl;

  // Clean 3 lines.
  winsize w;
  if (!ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)) {
    Output out;
    out.cleanTo(w.ws_col);
    out << Qt::endl;
    out.cleanTo(w.ws_col);
    out << Qt::endl;
    out.cleanTo(w.ws_col);
  }

  uint64_t packageCount = 0;
  uint64_t vpnCount = 0;
  uint64_t outCount = 0;
  uint64_t lanCount = 0;

  while (true) {
    pcap_pkthdr header;
    const u_char* packet = pcap_next(handle, &header);
    if (!packet) continue;

    ether_header* eptr = (ether_header*)packet;
    // We care about IP packages. Let's ignore the rest.
    if (ntohs(eptr->ether_type) != ETHERTYPE_IP) continue;

    u_int length = header.len;
    length -= sizeof(ether_header);
    if (length < sizeof(sniff_ip)) {
      // truncate IP
      continue;
    }

    sniff_ip* ip = (sniff_ip*)(packet + sizeof(ether_header));
    if (IP_V(ip) != 4) {
      // Invalid IP package version
      continue;
    }

    if (IP_HL(ip) < 5) {
      // Bad length
      continue;
    }

    if (length < ntohs(ip->ip_len)) {
      // Truncate IP
      continue;
    }

    // check to see if we have the first fragment
    if ((ntohs(ip->ip_off) & 0x1fff)) {
      continue;
    }

    QString ipSrc = inet_ntoa(ip->ip_src);
    QString ipDst = inet_ntoa(ip->ip_dst);

    if (ipSrc.isEmpty() || ipDst.isEmpty()) continue;

    bool outcome = addresses.contains(ipSrc);
    bool income = addresses.contains(ipDst);

    char direction = '=';
    // Let's ignore local and unrelated packages
    if ((income && outcome) || (!income && !outcome)) continue;

    ++packageCount;

    if (income)
      direction = '>';
    else
      direction = '<';

    Server* server = nullptr;
    if (income)
      server = servers.value(ipSrc);
    else
      server = servers.value(ipDst);

    if (server)
      ++vpnCount;
    else {
      bool isLan = false;
      if (income)
        isLan = RFC1918::contains(QHostAddress(ipSrc));
      else
        isLan = RFC1918::contains(QHostAddress(ipDst));

      if (isLan)
        ++lanCount;
      else {
        ++outCount;
        if (logger) logPackage(logger, ipSrc, ipDst);
      }
    }

    // Let's check the terminal size each time in case the size changes.
    int lineLength = 0;
    if (!ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)) {
      lineLength = w.ws_col;
    }

    {
      Output line1;
      line1 << Output::hideCursor() << Output::prevLine() << Output::prevLine();
      line1 << "Total: " << Output::cyan() << packageCount << Output::reset();
      line1 << " - VPN: " << Output::bold() << Output::green() << vpnCount
            << Output::reset();
      line1 << " - LAN: " << Output::bold() << Output::magenta() << lanCount
            << Output::reset();
      line1 << " - OUT: " << Output::bold() << Output::red() << outCount
            << Output::reset();
      line1.cleanTo(lineLength);
      line1 << Qt::endl;
    }

    {
      Output line2;
      line2 << "D: " << direction;
      line2 << " - Src: " << Output::bold() << Output::fixSize(ipSrc, 15)
            << Output::reset();
      line2 << " - Dest: " << Output::bold() << Output::fixSize(ipDst, 15)
            << Output::reset();
      line2.cleanTo(lineLength);
      line2 << Qt::endl;
    }

    {
      Output line3;
      if (server) {
        line3 << "Loc: " << Output::bold() << server->countryName << ", "
              << server->cityName << Output::reset();
      }
      line3.cleanTo(lineLength);
    }
  }

  pcap_close(handle);
  return 0;
}
