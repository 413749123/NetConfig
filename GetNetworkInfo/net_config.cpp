#include "common.h"
#include "net_config.h"

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")

//#pragma execution_character_set("utf-8")

using namespace std;

NetConfig::~NetConfig()
{
	clear();
}

bool NetConfig::enable_dhcp()
{
	bool rt = false;
	if (!init())
		return rt;

	return exec_method(L"EnableDHCP", NULL);
}

bool NetConfig::set_ip_config(const std::string & ip, const std::string & mask, const std::string & gateway)
{
	if (set_ip_config(ip, mask)) {
		return set_gateway(gateway);
	}
	return false;
}

bool NetConfig::set_ip_config(const std::string & ip, const std::string & mask)
{
	bool rt = false;
	if (!init())
		return rt;

	IWbemClassObject *params = NULL;
	IWbemClassObject *paramsInst = NULL;
	p_config->GetMethod(_bstr_t("EnableStatic"), 0, &params, NULL);
	params->SpawnInstance(0, &paramsInst);

	auto p1 = create_SAFEARRAY({ ip });
	VARIANT paramVt;
	paramVt.vt = VT_ARRAY | VT_BSTR;
	paramVt.parray = p1.get();
	paramsInst->Put(L"IPAddress", 0, &paramVt, NULL);
	p1 = create_SAFEARRAY({ mask });
	paramVt.parray = p1.get();
	paramsInst->Put(L"SubnetMask", 0, &paramVt, NULL);

	rt = exec_method(L"EnableStatic", paramsInst);
	if (params) {
		params->Release();
	}
	return rt;
}

bool NetConfig::set_dns(const std::string & default_dns, const std::string & backup_dns)
{
	return set_dns_base(false, default_dns, backup_dns);
}

bool NetConfig::set_auto_dns()
{
	return set_dns_base(true, "", "");
}

bool NetConfig::set_gateway(const std::string & gateway)
{
	bool rt = false;
	if (!init())
		return rt;

	IWbemClassObject *params = NULL;
	IWbemClassObject *paramsInst = NULL;
	p_config->GetMethod(_bstr_t("SetGateways"), 0, &params, NULL);
	params->SpawnInstance(0, &paramsInst);

	auto p1 = create_SAFEARRAY({ gateway });
	VARIANT paramVt;
	paramVt.vt = VT_ARRAY | VT_BSTR;
	paramVt.parray = p1.get();
	paramsInst->Put(L"DefaultIPGateway", 0, &paramVt, NULL);
	paramVt.vt = VT_UINT;
	paramVt.uintVal = 1;
	paramsInst->Put(L"GatewayCostMetric", 0, &paramVt, NULL);

	rt = exec_method(L"SetGateways", paramsInst);
	if (params) {
		params->Release();
	}
	return rt;
}

void NetConfig::clear()
{
	if (p_config) {
		p_config->Release();
		p_config = nullptr;
	}
	if (p_obj_) {
		p_obj_->Release();
		p_obj_ = nullptr;
	}
	if (p_enum_) {
		p_enum_->Release();
		p_enum_ = nullptr;
	}
	if (p_service_) {
		p_service_->Release();
		p_service_ = nullptr;
	}
	if (p_instance_) {
		p_instance_->Release();
		p_instance_ = nullptr;
	}
	if (is_init_) {
		CoUninitialize();
	}
	is_init_ = false;
}

bool NetConfig::init()
{
	if (is_init_) {
		return true;
	}

	// Step 1: Initialize COM.
	HRESULT	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "CoInitializeEx failed: " << hres << endl;
#endif
		return false;
	}

	// Step 3:  Obtain the initial locator to WMI
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(LPVOID*)&p_instance_);
	//ASSERT_THROW(SUCCEEDED(hres), "CoCreateInstance failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "CoCreateInstance failed: " << hres << endl;
#endif
		return false;
	}

	// Step 4: Connect to the local root\cimv2 namespace and obtain pointer pSvc to make IWbemServices calls.
	hres = p_instance_->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,
		NULL,
		0,
		NULL,
		0,
		0,
		&p_service_
	);
	//ASSERT_THROW(SUCCEEDED(hres), "ConnectServer failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "ConnectServer failed: " << hres << endl;
#endif
		return false;
	}

	// Step 5:  Set security levels for the proxy
	hres = CoSetProxyBlanket(
		p_service_,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);
	//ASSERT_THROW(SUCCEEDED(hres), "CoSetProxyBlanket failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "CoSetProxyBlanket failed: " << hres << endl;
#endif
		return false;
	}

	// 通过适配器名称来找到指定的适配器对象.
	CComBSTR TheQuery = L"SELECT * FROM Win32_NetworkAdapterConfiguration WHERE SettingID = \"";
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conversion;
	TheQuery += conversion.from_bytes(key_).c_str();
	TheQuery += L"\"";
  // const BSTR lang = L"WQL";
	hres = p_service_->ExecQuery(
		SysAllocString(L"WQL"),
		// L"WQL",
		TheQuery,
		WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
		NULL,
		&p_enum_);
	//ASSERT_THROW(SUCCEEDED(hres), "ExecQuery failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "ExecQuery failed: " << hres << endl;
#endif
		return false;
	}
	
	// Get the adapter object.
	ULONG num = 0;
	hres = p_enum_->Next(WBEM_INFINITE, 1, &p_obj_, &num);
	//ASSERT_THROW(SUCCEEDED(hres), "Next failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "Next failed: " << hres << endl;
#endif
		return false;
	}

	//ASSERT_THROW(0 < num, "Next failed");
	if (num < 1) {
#ifdef _DEBUG
		cout << "Next failed num < 1" << endl;
#endif
		return false;
	}

	VariantInit(&path_);
	hres = p_obj_->Get(L"__PATH", 0, &path_, NULL, NULL);
	//ASSERT_THROW(SUCCEEDED(hres), "Get path failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "Get failed: " << hres << endl;
#endif
		return false;
	}
	hres = p_service_->GetObject(_bstr_t(L"Win32_NetworkAdapterConfiguration"), 0, NULL, &p_config, NULL);
	//ASSERT_THROW(SUCCEEDED(hres), "GetObject Win32_NetworkAdapterConfiguration failed");
	if (FAILED(hres)) {
#ifdef _DEBUG
		cout << "GetObject failed: " << hres << endl;
#endif
		return false;
	}
	is_init_ = true;
	return true;
}

std::shared_ptr<SAFEARRAY> NetConfig::create_SAFEARRAY(const std::vector<std::string> &args)
{
	SAFEARRAY *psa = SafeArrayCreateVector(VT_BSTR, 0, args.size());
	long idx[] = { 0 };
	for (int i = 0; i < args.size(); i++) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conversion;
		BSTR ip = SysAllocString(conversion.from_bytes(args[i]).c_str());
		idx[0] = i;
		if (FAILED(SafeArrayPutElement(psa, idx, ip))) {
			return (false);
		}
		SysFreeString(ip);
	}
	return shared_ptr<SAFEARRAY>(psa, [](SAFEARRAY *psa) {SafeArrayDestroy(psa); });
}

bool NetConfig::set_dns_base(bool is_auto, const std::string & default_dns, const std::string & backup_dns)
{
	bool rt = false;
	if (!init())
		return rt;

	IWbemClassObject *params = NULL;
	IWbemClassObject *paramsInst = NULL;
	p_config->GetMethod(_bstr_t("SetDNSServerSearchOrder"), 0, &params, NULL);
	params->SpawnInstance(0, &paramsInst);

	shared_ptr<SAFEARRAY> p1;
	if (is_auto) {
		paramsInst->Put(L"DNSServerSearchOrder", 0, NULL, NULL);
	}
	else {
		if (backup_dns.size()) {
			p1 = create_SAFEARRAY({ default_dns, backup_dns });
		}
		else {
			p1 = create_SAFEARRAY({ default_dns });
		}
		VARIANT paramVt;
		paramVt.vt = VT_ARRAY | VT_BSTR;
		paramVt.parray = p1.get();
		paramsInst->Put(L"DNSServerSearchOrder", 0, &paramVt, NULL);
	}

	rt = exec_method(L"SetDNSServerSearchOrder", paramsInst);
	if (params) {
		params->Release();
	}
	return rt;
}

bool NetConfig::exec_method(const wchar_t * method, IWbemClassObject * params_instance)
{
	bool rt = false;
	IWbemClassObject *results = NULL;
	auto res = p_service_->ExecMethod(path_.bstrVal, _bstr_t(method), 0, NULL, params_instance, &results, NULL);
	if (SUCCEEDED(res)) {
		VARIANT vtRet;
		VariantInit(&vtRet);
		if (!FAILED(results->Get(L"ReturnValue", 0, &vtRet, NULL, 0))) {
			if (vtRet.uintVal == 0 || vtRet.uintVal == 1) {
				rt = true;
			}
			else {
				last_error_code_ = vtRet.uintVal;
#ifdef _DEBUG
				wcout << method << " failed, result: " << last_error_code_ << endl;
#endif
			}
		}
#ifdef _DEBUG
		else {
			cout << "ExecMethod Get ReturnValue failed: " << res << endl;
		}
#endif
		VariantClear(&vtRet);
		results->Release();
	}
#ifdef _DEBUG
	else {
		cout << "ExecMethod failed: " << res << endl;
	}
#endif	
	if (params_instance) {
		params_instance->Release();
	}
	return rt;
}
