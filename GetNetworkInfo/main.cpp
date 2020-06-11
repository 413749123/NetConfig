#include "common.h"
#include "network_info.h"

int main(int argc, char* argv[])
{
  auto netcards_info = NetCardInfo::GetNetworkInfo();
  int i = 0;
  std::string cmd = "";
  while (true) {
    if (cmd == "1") {
      std::cout << "������(ip ����)��";
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
        std::cout << "����ip����������ɹ�!" << std::endl;
      }
      if (netcards_info[0]->SetGateway(gateway)) {
        std::cout << (gateway == "0.0.0.0" ? "���" : "����") << "���سɹ�!" << std::endl;
      }
    } else if (cmd == "2") {
      std::cout << "���������ص�ַ��";
      std::string gateway;
      std::getline(std::cin, gateway);
      if (gateway == "") gateway = "0.0.0.0";
      if (netcards_info[0]->SetGateway(gateway)) {
        std::cout << (gateway == "0.0.0.0" ? "���" : "����") << "���سɹ�!" << std::endl;
      }
    } else if (cmd == "3") {
      std::cout << "������(Dns��ַ)";
      std::string dns, backup_dns = "";
      std::cin >> dns >> backup_dns;
      if (netcards_info[0]->SetDns(dns, backup_dns)) {
        std::cout << "����Dns�ɹ�" << std::endl;
      } else {
        netcards_info[0]->SetAutoDns();
      }
    } else if (cmd == "4") {
      if (netcards_info[0]->SetAutoDns()) {
        std::cout << "�����Զ�Dns�ɹ�";
      }
    } else if (cmd == "5") {
      for (auto &info : netcards_info) {
        info->show();
      }
    } else if (cmd == "6") {
      netcards_info = NetCardInfo::GetNetworkInfo();
      std::cout << "���¶�ȡ������Ϣ..." << std::endl;
      for (auto &info : netcards_info) {
        info->show();
      }
    } else {
      std::cout << "------------------------------------" << std::endl;
      std::cout << "|  1. ����IP�����������Լ�Ĭ������   " << std::endl;
      std::cout << "|  2. ����Ĭ������                  " << std::endl;
      std::cout << "|  3. ����DNS                      " << std::endl;
      std::cout << "|  4. �����Զ�DNS                  " << std::endl;
      std::cout << "|  5. ��ʾ������Ϣ                 " << std::endl;
      std::cout << "|  6. ˢ��������Ϣ                 " << std::endl;
      std::cout << "------------------------------------" << std::endl;
    }
    std::getline(std::cin, cmd);
  }
  return 0;
}