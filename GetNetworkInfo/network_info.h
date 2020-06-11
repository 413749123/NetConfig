#pragma once
#include "net_config.h"

class NetCardInfo {
public:
  NetCardInfo();

  struct Info {
    std::string adapter_name;      // ��������GUID
    std::string friendly_name;    // ������������
    std::string physical_addr;   // mac��ַ
    std::string ip;             // ip
    std::string ip_mask;       // ��������
    std::string gateway;      // ����
    std::string default_dns; // Ĭ��dns
    std::string backup_dns; // ��ѡdns
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
  Info info_; // ������Ϣ
  std::shared_ptr<NetConfig> config_;
};
