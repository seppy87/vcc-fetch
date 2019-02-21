#pragma once
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */




#include <string>
#include<7zpp/7zpp.h>


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
#include<Poco/DirectoryIterator.h>
#include<Poco/File.h>
#include<Poco/Path.h>
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
	//static FUNCS
	static void removeZeros(std::vector<std::string>& vec);
	static bool hasExtension(Poco::File& file, const std::vector<std::string>& ext);
	static std::string getFilename(std::string path);
	static std::string subtractPaths(std::string shorter, std::string longer);
	static std::string removeFilename(std::string path);

	//git clone
	void gitclone(const std::string& arg, const std::string& url);

	//create package
	void packCode();
	void setPackAction(const std::string& arg, const std::string& value);
	void packCPP();
	std::vector<Poco::File> iterateSubfolder(const Poco::File& path, const std::vector<std::string>& extension);
	void copySource(const std::vector<Poco::File>& files);
	void copyHeaders(const std::vector<Poco::File>& files);
	//void build();
	void qbuild();
};