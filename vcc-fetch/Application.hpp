#pragma once

#include<Poco/Util/Application.h>
#include<Poco/Util/Option.h>
#include<Poco/Util/OptionSet.h>
#include<Poco/Util/OptionCallback.h>
#include<Poco/Util/HelpFormatter.h>

#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include<Poco/Net/HTTPStreamFactory.h>
#include "Poco/Net/FTPSStreamFactory.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SSLManager.h"
#include<iostream>

enum class FuncTarget : unsigned int {
	DATABASE_UPDATE = 0,
	LIBRARIES = 1,
};

class Application : public Poco::Util::Application {
private:
	std::vector<std::string> libname;
	std::string action;
protected:
	void initialize(Poco::Util::Application& application);
	void uninitialize();
	void defineOptions(Poco::Util::OptionSet& optionSet);
	int main(const std::vector<std::string>& arguments);
	void setOption(const std::string& key, const std::string& value);
	void addLib(const std::string& useless, const std::string& value);
	inline std::string HasOption(std::string key);
	bool downloadFile(FuncTarget target = FuncTarget::LIBRARIES);
	void setAction(const std::string& key, const std::string& val);
	void showHelp(const std::string &key, const std::string& val);
};