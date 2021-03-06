//--------------------------------------------------------------------------
// Copyright (C) 2014-2020 Cisco and/or its affiliates. All rights reserved.
// Copyright (C) 2005-2013 Sourcefire, Inc.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------

// appid_config.h author Sourcefire Inc.

#ifndef APP_ID_CONFIG_H
#define APP_ID_CONFIG_H

#include <array>
#include <string>

#include "target_based/snort_protocols.h"

#include "app_info_table.h"
#include "client_plugins/client_discovery.h"
#include "detector_plugins/dns_patterns.h"
#include "detector_plugins/http_url_patterns.h"
#include "detector_plugins/sip_patterns.h"
#include "detector_plugins/ssl_patterns.h"
#include "host_port_app_cache.h"
#include "length_app_cache.h"
#include "service_plugins/service_discovery.h"
#include "tp_appid_module_api.h"

#define APP_ID_PORT_ARRAY_SIZE  65536

extern SnortProtocolId snortId_for_unsynchronized;
extern SnortProtocolId snortId_for_ftp_data;
extern SnortProtocolId snortId_for_http2;

class PatternClientDetector;
class PatternServiceDetector;

class AppIdConfig
{
public:
    AppIdConfig() = default;
    ~AppIdConfig();

    // FIXIT-L: DECRYPT_DEBUG - Move this to ssl-module
#ifdef REG_TEST
    // To manually restart appid detection for an SSL-decrypted flow (single session only),
    // indicate the first packet from where the flow is decrypted (usually immediately
    // after certificate-exchange). Such manual detection is disabled by default (0).
    uint32_t first_decrypted_packet_debug = 0;
#endif
    bool stats_logging_enabled = false;
    unsigned long app_stats_period = 300;
    unsigned long app_stats_rollover_size = 0;
    unsigned long app_stats_rollover_time = 0;
    const char* app_detector_dir = nullptr;
    std::string tp_appid_path = "";
    std::string tp_appid_config = "";
    bool tp_appid_stats_enable = false;
    bool tp_appid_config_dump = false;
    uint32_t instance_id = 0;
    size_t memcap = 0;
    bool debug = false;
    bool dump_ports = false;
    bool log_all_sessions = false;
};

class OdpContext
{
public:
    bool dns_host_reporting = true;
    bool referred_appId_disabled = false;
    bool mdns_user_reporting = true;
    bool chp_userid_disabled = false;
    bool http2_detection_enabled = false;
    bool is_host_port_app_cache_runtime = false;
    bool check_host_port_app_cache = false;
    bool check_host_cache_unknown_ssl = false;
    uint32_t ftp_userid_disabled = 0;
    uint32_t chp_body_collection_disabled = 0;
    uint32_t chp_body_collection_max = 0;
    uint32_t rtmp_max_packets = 15;
    uint32_t max_tp_flow_depth = 5;
    uint32_t tp_allow_probes = 0;
    uint32_t host_port_app_cache_lookup_interval = 10;
    uint32_t host_port_app_cache_lookup_range = 100000;
    uint32_t http_response_version_enabled = 0;
    bool allow_port_wildcard_host_cache = false;
    bool recheck_for_portservice_appid = false;

    OdpContext(AppIdConfig&, snort::SnortConfig*);
    void initialize();

    AppInfoManager& get_app_info_mgr()
    {
        return app_info_mgr;
    }

    ClientDiscovery& get_client_disco_mgr()
    {
        return client_disco_mgr;
    }

    ServiceDiscovery& get_service_disco_mgr()
    {
        return service_disco_mgr;
    }

    HostPortVal* host_port_cache_find(const snort::SfIp* ip, uint16_t port, IpProtocol proto)
    {
        return host_port_cache.find(ip, port, proto, *this);
    }

    bool host_port_cache_add(const snort::SfIp* ip, uint16_t port, IpProtocol proto, unsigned type, AppId appid)
    {
        return host_port_cache.add(ip, port, proto, type, appid);
    }

    AppId length_cache_find(const LengthKey& key)
    {
        return length_cache.find(key);
    }

    bool length_cache_add(const LengthKey& key, AppId val)
    {
        return length_cache.add(key, val);
    }

    DnsPatternMatchers& get_dns_matchers()
    {
        return dns_matchers;
    }

    HttpPatternMatchers& get_http_matchers()
    {
        return http_matchers;
    }

    SipPatternMatchers& get_sip_matchers()
    {
        return sip_matchers;
    }

    SslPatternMatchers& get_ssl_matchers()
    {
        return ssl_matchers;
    }

    PatternClientDetector& get_client_pattern_detector()
    {
        return *client_pattern_detector;
    }

    PatternServiceDetector& get_service_pattern_detector()
    {
        return *service_pattern_detector;
    }

    void add_port_service_id(IpProtocol, uint16_t, AppId);
    void add_protocol_service_id(IpProtocol, AppId);
    AppId get_port_service_id(IpProtocol, uint16_t);
    AppId get_protocol_service_id(IpProtocol);
    void display_port_config();

private:
    AppInfoManager app_info_mgr;
    ClientDiscovery client_disco_mgr;
    HostPortCache host_port_cache;
    LengthCache length_cache;
    DnsPatternMatchers dns_matchers;
    HttpPatternMatchers http_matchers;
    ServiceDiscovery service_disco_mgr;
    SipPatternMatchers sip_matchers;
    SslPatternMatchers ssl_matchers;
    PatternClientDetector* client_pattern_detector;
    PatternServiceDetector* service_pattern_detector;

    std::array<AppId, APP_ID_PORT_ARRAY_SIZE> tcp_port_only = {}; // port-only TCP services
    std::array<AppId, APP_ID_PORT_ARRAY_SIZE> udp_port_only = {}; // port-only UDP services
    std::array<AppId, 256> ip_protocol = {}; // non-TCP / UDP protocol services
};

class AppIdContext
{
public:
    AppIdContext(AppIdConfig& config) : config(config)
    { }

    ~AppIdContext() { }

    OdpContext& get_odp_ctxt() const
    {
        assert(odp_ctxt);
        return *odp_ctxt;
    }

    ThirdPartyAppIdContext* get_tp_appid_ctxt() const
    { return tp_appid_ctxt; }

    static void delete_tp_appid_ctxt()
    { delete tp_appid_ctxt; }

    void create_tp_appid_ctxt();
    bool init_appid(snort::SnortConfig*);
    static void pterm();
    void show();

    AppIdConfig& config;

private:
    static OdpContext* odp_ctxt;
    static ThirdPartyAppIdContext* tp_appid_ctxt;
};

#endif
