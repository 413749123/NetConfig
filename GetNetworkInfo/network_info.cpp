// GetNetworkInfo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "common.h"
#include "net_config.h"
#include "network_info.h"

using namespace std;
#pragma comment(lib,"Iphlpapi.lib") //需要添加Iphlpapi.lib库


NetCardInfo::NetCardInfo() 
{

}

std::vector<std::shared_ptr<NetCardInfo>> NetCardInfo::GetNetworkInfo() {
  std::vector<std::shared_ptr<NetCardInfo>> info_list;

  {
    /*****************获取网卡名、mac地址、ip地址、子网掩码、默认网关**********************/
    //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
    PIP_ADAPTER_INFO adapter_info = new IP_ADAPTER_INFO();
    PIP_ADAPTER_INFO adapter = nullptr;
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    memset(adapter_info, 0, stSize);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    auto ret = GetAdaptersInfo(adapter_info, &stSize);
    if (ERROR_BUFFER_OVERFLOW == ret) {
      delete adapter_info;
      adapter_info = (PIP_ADAPTER_INFO)new BYTE[stSize];
      ret = GetAdaptersInfo(adapter_info, &stSize);
    }
    if (ERROR_SUCCESS == ret) { // 可能有多网卡,因此通过循环去判断
      adapter = adapter_info; // 保存链表头，等下需要释放空间
      while (adapter) {
        NetCardInfo info;
        if (info.ParseInfo(adapter)) {
          info_list.push_back(std::make_shared<NetCardInfo>(info)); // 转换成功才添加
        }
        adapter = adapter->Next;
      }
    }
    //释放内存空间
    if (adapter_info) {
      delete adapter_info;
    }
  }
  
  {
    /******************获取连接名和dns地址************************/
    PIP_ADAPTER_ADDRESSES addresses = nullptr, cur_addresses = nullptr;
    ULONG outBufLen = 15000; // Allocate a 15 KB buffer to start with.
    ULONG Iterations = 0, ret = 0;
    do {
      addresses = (IP_ADAPTER_ADDRESSES *)new BYTE[outBufLen];
      if (addresses != nullptr) {
        // 0x07，Skip_UNICAST、Skip_ANYCAST、Skip_MULTICAST
        ret = GetAdaptersAddresses(AF_UNSPEC, 0x07, nullptr, addresses, &outBufLen);
        if (ret == NO_ERROR) break;
        if (ret == ERROR_BUFFER_OVERFLOW) {
          delete addresses;
          addresses = nullptr;
        }
      }
      Iterations++;

    } while ((ret == ERROR_BUFFER_OVERFLOW) && (Iterations < 3));

    if (ret == NO_ERROR) {
      // If successful, output some information from the data we received
      cur_addresses = addresses;
      while (cur_addresses) {
        auto it = std::find_if(info_list.begin(), info_list.end(), [&](auto & info)
                               { // 根据适配器的GUID找到相应的对象，添加信息
                                 return info->info_.adapter_name == cur_addresses->AdapterName;
                               });
        if (it != info_list.end()) {
          (*it)->ParseInfo(cur_addresses);
        } else { // 没找到的只会是Loopback型的，因为环回地址不需要
#if _DEBUG
          std::cout << "Not found：" << cur_addresses->AdapterName << std::endl;
#endif
        }
        cur_addresses = cur_addresses->Next;
      }
    }

    if (addresses) delete addresses;
  }

  

  return std::move(info_list);
}

const NetCardInfo::Info & NetCardInfo::GetNetCardInfo()
{
  return info_;
}

void NetCardInfo::show() {
  std::cout << "网卡名：" << info_.adapter_name << std::endl;
  std::cout << "连接名：" << info_.friendly_name << std::endl;
  std::cout << "Mac地址：" << info_.physical_addr << std::endl;
  std::cout << "Ip地址：" << info_.ip << std::endl;
  std::cout << "子网掩码：" << info_.ip_mask << std::endl;
  std::cout << "默认网关：" << info_.gateway << std::endl;
  std::cout << "默认Dns：" << info_.default_dns << std::endl;
  std::cout << "备选Dns：" << info_.backup_dns << std::endl;
  std::cout << std::endl;
}

bool NetCardInfo::SetDns(const std::string & default_dns, const std::string & backup_dns) {
  if (config_ && config_->set_dns(default_dns, backup_dns)) {
    info_.default_dns = default_dns; // 更新信息
    info_.backup_dns = backup_dns;
    return true;
  }
  return false;
}

bool NetCardInfo::SetAutoDns() { // 设置autodns之后需要更新信息，目前未在设置里自动更新
  return config_ && config_->set_auto_dns();
}

bool NetCardInfo::SetIpConfig(const std::string & ip, const std::string & ip_mask)
{
  //设置静态ip之前要调用下enable_dhcp, 可以把以前设置的ip和网关都清空, 否则注册表会有多个ip信息
  if (!config_) return false;
  config_->enable_dhcp();
  if (config_->set_ip_config(ip, ip_mask)) {
    info_.ip = ip; // 设置成功就更新信息
    info_.ip_mask = ip_mask;
    return true;
  }
  return false;
}

bool NetCardInfo::SetGateway(const std::string &gateway) {
  if (!config_) return false; // 设置默认网关需要在静态ip的模式下才能成功,静态模式依靠SetIpConfig，
  bool flag = true; 
  if (gateway != "0.0.0.0") 
    flag = config_ && config_->set_gateway(gateway); // 如果要设置的gateway不为"0.0.0.0"，那就直接设置
  else if (info_.gateway != gateway) 
    flag = config_->set_gateway(info_.ip); // 设置当前正在使用的ip为网关，就能清空网关
  if (flag) info_.gateway = gateway; // 不论是清空还是设置网关，只要成功了，就需要更新当前的信息 
  return flag;
}

bool NetCardInfo::ParseInfo(PIP_ADAPTER_INFO adapter_info) {
  if (!adapter_info) return false;
  if (adapter_info->Type == MIB_IF_TYPE_LOOPBACK) return false; // 忽略环回地址

  info_.adapter_name = adapter_info->AdapterName;

  char mac[128] = { 0 };
  for (DWORD i = 0; i < adapter_info->AddressLength; i++)
    sprintf(mac + strlen(mac)
            , (i < adapter_info->AddressLength - 1 ? "%02X-" : "%02X")
            , adapter_info->Address[i]);
  info_.physical_addr = mac;
  info_.ip = adapter_info->IpAddressList.IpAddress.String; // 虽然一个网卡可能有多个ip和子网掩码，但是这里只需要第一个即可
  info_.ip_mask = adapter_info->IpAddressList.IpMask.String;  
  info_.gateway = adapter_info->GatewayList.IpAddress.String; 
  config_.reset(new NetConfig(info_.adapter_name)); // 根据网卡的GUID创建一个config对象，提供网卡属性的修改
  return true;
}

// 主要获取dns和适配器的名称，其余的由重载的另一个函数获取
bool NetCardInfo::ParseInfo(PIP_ADAPTER_ADDRESSES adapter_addresses) {
  auto dns_server = adapter_addresses->FirstDnsServerAddress;
  if (dns_server) {
    for (int count = 0; dns_server != nullptr; count++) {
      unsigned char *addr = (unsigned char *)(dns_server->Address.lpSockaddr->sa_data);
      std::string dns = "";
      for (int i = 2; i < 6; ++i) { // 将addr[2]-addr[5]转为字符串表示，每一个字节就是dns的一段
        dns += std::to_string(addr[i]);
        if (i != 5) dns += ".";
      }
      if (!count) {
        info_.default_dns = dns;
      } else {
        info_.backup_dns = dns;
        break;
      }
      dns_server = dns_server->Next;
    }
  }

  info_.friendly_name = CW2A(adapter_addresses->FriendlyName); // 获取名称
  return true;
}