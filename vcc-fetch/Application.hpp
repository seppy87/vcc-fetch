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
#include<Poco/File.h>
#include<Poco/Timespan.h>
#include<Poco/Data/Session.h>
#include<Poco/Data/SQLite/Connector.h>
#include<Poco/SevenZip/Archive.h>
#include<Poco/Delegate.h>
#include<tuple>
#include<iostream>
#include<exception>
#include<algorithm>
#include<git2.h>

enum class FuncTarget : unsigned int {
	DATABASE_UPDATE = 0,
	LIBRARIES = 1,
};

namespace standalone {
	void progress(const char* path, size_t cur, size_t tot, void* payload);
	int fetch_progress(
		const git_transfer_progress *stats,
		void *payload);
}
class Application : public Poco::Util::Application {
private:
	std::vector<std::string> libname;
	std::vector<std::string> dependencies;
	std::string action;
	bool ExtractSuccess = true;
protected:
	void initialize(Poco::Util::Application& application);
	void uninitialize();
	void defineOptions(Poco::Util::OptionSet& optionSet);
	int main(const std::vector<std::string>& arguments);
	void setOption(const std::string& key, const std::string& value);
	void addLib(const std::string& useless, const std::string& value);
	inline std::string HasOption(std::string name);
	bool downloadFile(FuncTarget target = FuncTarget::LIBRARIES, std::string filepath = "", std::string filename = "tmp");
	void setAction(const std::string& key, const std::string& val);
	void showHelp(const std::string &key, const std::string& val);
	std::string checkfile();
	std::vector<std::tuple<std::string, std::string, std::string, std::string,std::string>> searchLibs(std::string name);
	std::tuple<std::string, std::string, std::string, std::string> searchLibByID(std::string id);
	void extractFile(std::string filepath);
	void onExtractError(const Poco::SevenZip::Archive::FailedEventArgs& args);
	void onSevenZipSuccess(const Poco::SevenZip::Archive::ExtractedEventArgs& args);
	void prepareDependencies();
	std::string getSourceByLibId(std::string id);
	static void removeZeros(std::vector<std::string>& vec);

	//git clone
	void gitclone(const std::string& arg, const std::string& url);
};