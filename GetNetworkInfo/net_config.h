#pragma once

class NetConfig
{
public:
	NetConfig() {}
	NetConfig(const std::string &key) 
		:key_(key){ }
	~NetConfig();

	//设置网络设备GUID
	void set_key(const std::string &key) {
		clear();
		key_ = key;
	}
	//启用DHCP
	bool enable_dhcp();
	//启动静态IP,设置IP,掩码,网关
	bool set_ip_config(const std::string &ip, const std::string &mask, const std::string &gateway);
	bool set_ip_config(const std::string &ip, const std::string &mask);
	//设置网关
	bool set_gateway(const std::string &gateway);
	//设置DNS地址
	bool set_dns(const std::string &default_dns, const std::string &backup_dns);
	//设置自动DNS
	bool set_auto_dns();

	int get_last_error_code()const {
		return last_error_code_;
	}

	void clear();

private:
	NetConfig(const NetConfig &rhs) = delete;
	NetConfig &operator = (const NetConfig &rhs) = delete;

private:
	//初始化
	bool init();
	//创建COM数组
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