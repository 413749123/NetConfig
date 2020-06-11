#pragma once
#include "net_config.h"

class NetCardInfo {
public:
  NetCardInfo();

  struct Info {
    std::string adapter_name;      // 适配器的GUID
    std::string friendly_name;    // 适配器的名字
    std::string physical_addr;   // mac地址
    std::string ip;             // ip
    std::string ip_mask;       // 子网掩码
    std::string gateway;      // 网关
    std::string default_dns; // 默认dns
    std::string backup_dns; // 备选dns
  };

  static std::vector<std::shared_ptr<NetCardInfo>> GetNetworkInfo();

  const Info &GetNetCardInfo();

  void show();

  bool SetDns(const std::string & default_dns, const std::string & backup_dns);

  bool SetAutoDns(); // To clear the gateway,call this function.

  bool SetIpConfig(const std::string &ip, const std::string &ip_mask);
  
  // To clear the gateway,set your gateway to the same IP you use.
  bool SetGateway(const std::string &gateway);

protected:
  bool ParseInfo(PIP_ADAPTER_INFO adapter_info);

  bool ParseInfo(PIP_ADAPTER_ADDRESSES adapter_addresses);
private:
  Info info_; // 网卡信息
  std::shared_ptr<NetConfig> config_;
};
