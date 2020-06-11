#pragma once

class NetConfig
{
public:
	NetConfig() {}
	NetConfig(const std::string &key) 
		:key_(key){ }
	~NetConfig();

	//���������豸GUID
	void set_key(const std::string &key) {
		clear();
		key_ = key;
	}
	//����DHCP
	bool enable_dhcp();
	//������̬IP,����IP,����,����
	bool set_ip_config(const std::string &ip, const std::string &mask, const std::string &gateway);
	bool set_ip_config(const std::string &ip, const std::string &mask);
	//��������
	bool set_gateway(const std::string &gateway);
	//����DNS��ַ
	bool set_dns(const std::string &default_dns, const std::string &backup_dns);
	//�����Զ�DNS
	bool set_auto_dns();

	int get_last_error_code()const {
		return last_error_code_;
	}

	void clear();

private:
	NetConfig(const NetConfig &rhs) = delete;
	NetConfig &operator = (const NetConfig &rhs) = delete;

private:
	//��ʼ��
	bool init();
	//����COM����
	std::shared_ptr<SAFEARRAY> create_SAFEARRAY(const std::vector<std::string> &args);
	bool set_dns_base(bool is_auto, const std::string &default_dns, const std::string &backup_dns);
	bool exec_method(const wchar_t *method, IWbemClassObject *params_instance);

private:
	std::string key_;
	bool is_init_ = false;
	IWbemLocator* p_instance_ = nullptr;
	IWbemServices* p_service_ = nullptr;
	IEnumWbemClassObject* p_enum_ = nullptr;
	IWbemClassObject *p_obj_ = nullptr;
	IWbemClassObject *p_config = nullptr;
	VARIANT path_;
	int last_error_code_ = 0;
};