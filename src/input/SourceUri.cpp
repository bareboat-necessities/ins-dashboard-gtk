#include "input/SourceUri.h"

#include "util/StringUtil.h"

#include <stdexcept>

namespace ins_display::input {
namespace {
using util::split;
using util::starts_with;
using util::to_int;
using util::upper;
} // namespace

SourceConfig parse_source_uri(const std::string& uri) {
    SourceConfig c;
    c.uri = uri;

    if (uri == "demo://" || uri == "demo") {
        c.kind = SourceConfig::Kind::Demo;
        return c;
    }

    const std::string tcp_prefix = "tcp-nmea0183://";
    const std::string ser_prefix = "serial-nmea0183://";

    if (starts_with(uri, tcp_prefix)) {
        c.kind = SourceConfig::Kind::Tcp;
        std::string rest = uri.substr(tcp_prefix.size());
        if (rest.empty()) throw std::runtime_error("empty TCP source");

        if (rest.front() == '[') {
            auto close = rest.find(']');
            if (close == std::string::npos || close + 2 > rest.size() || rest[close + 1] != ':') {
                throw std::runtime_error("invalid IPv6 TCP source; expected tcp-nmea0183://[addr]:port");
            }
            c.host = rest.substr(1, close - 1);
            c.port = std::stoi(rest.substr(close + 2));
        } else {
            auto colon = rest.rfind(':');
            if (colon == std::string::npos) throw std::runtime_error("invalid TCP source; expected host:port");
            c.host = rest.substr(0, colon);
            c.port = std::stoi(rest.substr(colon + 1));
        }
        if (c.host.empty() || c.port <= 0 || c.port > 65535) {
            throw std::runtime_error("invalid TCP source host/port");
        }
        return c;
    }

    if (starts_with(uri, ser_prefix)) {
        c.kind = SourceConfig::Kind::Serial;
        std::string rest = uri.substr(ser_prefix.size());
        auto q = rest.find('?');
        c.path = q == std::string::npos ? rest : rest.substr(0, q);
        if (c.path.empty()) throw std::runtime_error("invalid serial source path");

        if (q != std::string::npos) {
            std::string query = rest.substr(q + 1);
            for (const auto& kv : split(query, '&')) {
                auto eq = kv.find('=');
                if (eq == std::string::npos) continue;
                std::string k = upper(kv.substr(0, eq));
                std::string v = kv.substr(eq + 1);
                if (k == "BAUD") {
                    auto b = to_int(v);
                    if (!b) throw std::runtime_error("invalid baud value: " + v);
                    c.baud = *b;
                }
            }
        }
        return c;
    }

    throw std::runtime_error("unsupported source URI: " + uri);
}

} // namespace ins_display::input
