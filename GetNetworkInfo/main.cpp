#include "common.h"
#include "network_info.h"

int main(int argc, char* argv[])
{
  auto netcards_info = NetCardInfo::GetNetworkInfo();
  int i = 0;
  std::string cmd = "";
  while (true) {
    if (cmd == "1") {
      std::cout << "请输入(ip 掩码)：";
      std::string str;
      std::getline(std::cin, str);
      std::vector<std::string> v;
      size_t pos = 0U, pos_2 = str.find(' ');
      while (std::string::npos != pos_2) {
        v.push_back(str.substr(pos, pos_2 - pos));
        pos = pos_2 + 1;
        pos_2 = str.find(' ', pos);
      }
      if (pos != str.length()) {
        v.push_back(str.substr(pos));
      }
      auto ip = v[0], mask = v[1];
      std::string gateway = "";
      if (v.size() == 3) gateway = v[2];
      if (gateway == "") gateway = "0.0.0.0";
      if (netcards_info[0]->SetIpConfig(ip, mask)) {
        std::cout << "设置ip和子网掩码成功!" << std::endl;
      }
      if (netcards_info[0]->SetGateway(gateway)) {
        std::cout << (gateway == "0.0.0.0" ? "清空" : "设置") << "网关成功!" << std::endl;
      }
    } else if (cmd == "2") {
      std::cout << "请输入网关地址：";
      std::string gateway;
      std::getline(std::cin, gateway);
      if (gateway == "") gateway = "0.0.0.0";
      if (netcards_info[0]->SetGateway(gateway)) {
        std::cout << (gateway == "0.0.0.0" ? "清空" : "设置") << "网关成功!" << std::endl;
      }
    } else if (cmd == "3") {
      std::cout << "请输入(Dns地址)";
      std::string dns, backup_dns = "";
      std::cin >> dns >> backup_dns;
      if (netcards_info[0]->SetDns(dns, backup_dns)) {
        std::cout << "设置Dns成功" << std::endl;
      } else {
        netcards_info[0]->SetAutoDns();
      }
    } else if (cmd == "4") {
      if (netcards_info[0]->SetAutoDns()) {
        std::cout << "设置自动Dns成功";
      }
    } else if (cmd == "5") {
      for (auto &info : netcards_info) {
        info->show();
      }
    } else if (cmd == "6") {
      netcards_info = NetCardInfo::GetNetworkInfo();
      std::cout << "重新读取网卡信息..." << std::endl;
      for (auto &info : netcards_info) {
        info->show();
      }
    } else {
      std::cout << "------------------------------------" << std::endl;
      std::cout << "|  1. 设置IP和子网掩码以及默认网关   " << std::endl;
      std::cout << "|  2. 设置默认网关                  " << std::endl;
      std::cout << "|  3. 设置DNS                      " << std::endl;
      std::cout << "|  4. 设置自动DNS                  " << std::endl;
      std::cout << "|  5. 显示网卡信息                 " << std::endl;
      std::cout << "|  6. 刷新网卡信息                 " << std::endl;
      std::cout << "------------------------------------" << std::endl;
    }
    std::getline(std::cin, cmd);
  }
  return 0;
}