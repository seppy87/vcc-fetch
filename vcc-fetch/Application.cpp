#include"Application.hpp"
#include<fstream>
#include<boost/algorithm/string.hpp>
//#include <Poco/Zip/Compress.h>
#include <locale>
#include <codecvt>




/// <summary>
/// Initializes the specified application.
/// </summary>
/// <param name="application">The application.</param>
void Application::initialize(Poco::Util::Application& application) {
	//this->config().setString("target", "defaultoption");
	this->loadConfiguration();
	Poco::Util::Application::initialize(application);
}

/// <summary>
/// Uninitializes this instance.
/// </summary>
void Application::uninitialize() {
	Poco::Net::uninitializeSSL();
	Poco::Util::Application::uninitialize();
}

/// <summary>
/// Defines the options.
/// </summary>
/// <param name="optionSet">The option set.</param>
void Application::defineOptions(Poco::Util::OptionSet& optionSet) {
	using Poco::Util::Option;
	using Poco::Util::OptionCallback;
	Poco::Util::Application::defineOptions(optionSet);
	optionSet.addOption(
		Poco::Util::Option("target", "t", "Target Directory").required(false).repeatable(false).argument("directory",true).callback(Poco::Util::OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("library", "lib", "Static, Dynamic or Both", false).repeatable(false).argument("library",true).callback(OptionCallback<Application>(this, &Application::setOption))
	);
	optionSet.addOption(
		Option("architecture","arc","Choose Architecture",false).repeatable(false).argument("architecture",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("name", "n", "Name of the Library you want to download", false).repeatable(true).argument("name", true).callback(OptionCallback<Application>(this, &Application::addLib))
	);
	optionSet.addOption(
		Option("install","", "Installs the library",false).repeatable(false).group("ActionGroup").noArgument().callback(OptionCallback<Application>(this,&Application::setAction))
	);
	optionSet.addOption(
		Option("search", "", "searches for the library", false).repeatable(false).group("ActionGroup").noArgument().callback(OptionCallback<Application>(this, &Application::setAction))
	);
	optionSet.addOption(
		Option("update","","Updates the database",false).repeatable(false).group("ActionGroup").noArgument().callback(OptionCallback<Application>(this,&Application::setAction))
	);
	optionSet.addOption(
		Option("pack","","Packs the Code for uploading",false).repeatable(false).group("ActionGroup").argument("packagename",true).callback(OptionCallback<Application>(this,&Application::setPackAction))
	);
	optionSet.addOption(
		Option("git","","GIT CLONE",false).repeatable(true).argument("git").callback(OptionCallback<Application>(this,&Application::gitclone))
	);
	optionSet.addOption(
		Option("help","h","HELP").repeatable(false).noArgument().required(false).callback(OptionCallback<Application>(this,&Application::showHelp))
	);
	optionSet.addOption(
		Option("bin","","BINARY Directory. Need to have the LIB and DLL if Dynamic").repeatable(false).argument("bindir",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("source","","Source Path",false).repeatable(false).argument("source",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("include","","Include Path",false).repeatable(false).argument("include",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("lang","","Sets Programming Language",false).repeatable(false).argument("lang",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("debugstatic","dlib","Sets The Debug .lib File - Path with Filename",false).repeatable(false).argument("debugstatic",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("releasestatic","rlib","Sets the Release .lib File - Path with Filename",false).repeatable(false).argument("releasestatic",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("debugdynamic","dynlib","Sets the Path to Dynamic Debug libraries. Path without filename",false).repeatable(false).argument("debugdynamic",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
	optionSet.addOption(
		Option("releasedynamic","dynlibr","Sets the Path to Dynamic Release libraries. Path without filename",false).repeatable(false).argument("releasedynamic",true).callback(OptionCallback<Application>(this,&Application::setOption))
	);
}

/// <summary>
/// Sets the pack action.
/// </summary>
/// <param name="arg">The argument.</param>
/// <param name="value">The value.</param>
void Application::setPackAction(const std::string& arg, const std::string& value) {
	this->action = "package";
	this->config().setString(arg, value);
}

/// <summary>
/// Packs the code for uploading.
/// </summary>
void Application::packCode() {
	auto language = boost::algorithm::to_lower_copy(this->config().getString("lang"));
	if (language == "c++") {
		this->packCPP();
		return;
	}
}

/// <summary>
/// Packs CPP code for uploading.
/// </summary>
void Application::packCPP() {
	using namespace Poco;
	std::vector<std::string> ext_source{ "cpp","cxx","c" };
	std::vector<std::string> ext_headers{ "h","hpp" };
	//Iterate Source files
	DirectoryIterator it(this->config().getString("source"));
	DirectoryIterator end;
	std::cout << "Starting to iterate\n";
	std::vector<Poco::File> sources;
	for (; it != end; it++) {
		if (it->isFile()) {
			if (Application::hasExtension(*it, ext_source)) {
				sources.insert(sources.end(), *it);
			}
			continue;
		}
		if (it->isDirectory()) {
			auto res = this->iterateSubfolder(*it, ext_source);
			sources.insert(sources.end(), res.begin(), res.end());
		}
	}
	this->copySource(sources);
	if (this->config().hasProperty("include")) {
		//iterate through headers
		DirectoryIterator it(this->config().getString("include"));
		DirectoryIterator end;
		std::cout << "Iterating Headers\n";
		std::vector<Poco::File> headers;
		for (; it != end; it++) {
			if (it->isFile()) {
				if (Application::hasExtension(*it, ext_headers))
					headers.insert(headers.end(), *it);
				continue;
			}
			if (it->isDirectory()) {
				auto res = this->iterateSubfolder(*it, ext_headers);
				headers.insert(headers.end(), res.begin(), res.end());
			}
		}
		this->copyHeaders(headers);
	}
	if (this->config().hasProperty("debugstatic")) {
		Poco::File debug(this->config().getString("debugstatic"));
		if (debug.isFile() == false) {
			std::cout << "Path to Debug Static lib needs to contain the static lib! Will skip this part!\n";
		}
		else {
			if (Application::hasExtension(debug, std::vector<std::string>{"lib"})) {
				auto target = this->config().getString("targetpack") + "\\debug";
				(Poco::File(target)).createDirectory();
				target += "\\static";
				(Poco::File(target)).createDirectory();
				debug.copyTo(target + "\\" + Application::getFilename(debug.path()));
			}
			else {
				std::cout << "A Static Lib has to have a .lib Extension! Will skip this part!\n";
			}
		}
	}
	if (this->config().hasProperty("releasestatic")) {
		Poco::File release(this->config().getString("releasestatic"));
		if (release.isFile() == false) {
			std::cout << "Path to Release Static lib needs to contain the static lib! Will skip this part!\n";
		}
		else {
			if (Application::hasExtension(release, std::vector<std::string>{"lib"})) {
				auto target = this->config().getString("targetpack") + "\\release";
				(Poco::File(target)).createDirectory();
				target += "\\static";
				(Poco::File(target)).createDirectory();
				release.copyTo(target + "\\" + Application::getFilename(release.path()));
			}
			else {
				std::cout << "A static lib has to have a .lib Extension! Will skip this part\n";
			}
		}
	}
	if (this->config().hasProperty("debugdynamic")) {
		Poco::File debug(this->config().getString("debugdynamic"));
		if (debug.isDirectory() == false) {
			std::cout << "The Variable for Dynamic Libraries has to be a Directory which contains the .lib and .dll File! Will skip this part!\n";
		}
		else {
			DirectoryIterator it(debug);
			DirectoryIterator end;
			
			for (; it != end; it++) {
				if (it->isFile() != true) continue;
				if (Application::hasExtension(*it, std::vector<std::string>{"lib", "dll"})) {
					auto target = this->config().getString("targetpack") + "\\dynamic";
					(Poco::File(target)).createDirectory();
					target += "\\debug";
					(Poco::File(target)).createDirectory();
					it->copyTo(target + "\\" + Application::getFilename(it->path()));
				}

			}
		}
	}
	if (this->config().hasProperty("releasedynamic")) {
		Poco::File release(this->config().getString("releasedynamic"));
		if (release.isDirectory() == false) {
			std::cout << "The Variable for Dynamic Libraries has to be a Directory which contains the .lib and .dll File! Will skip this part!\n";
		}
		else {
			DirectoryIterator it(release);
			DirectoryIterator end;
			for (; it != end; it++) {
				if (it->isFile() != true) continue;
				if (Application::hasExtension(*it, std::vector<std::string>{"lib", "dll"})) {
					auto target = this->config().getString("targetpack") + "\\dynamic";
					(Poco::File(target)).createDirectory();
					target += "\\release";
					(Poco::File(target)).createDirectory();
					it->copyTo(target + "\\" + Application::getFilename(it->path()));
				}
			}
		}
	}
	this->qbuild();
}

void Application::qbuild() {
	using namespace SevenZip;
	std::string fname = this->config().getString("target") + "\\" + this->config().getString("pack") + ".7z";
	SevenZipLibrary lib;
	lib.Load("7z.dll");
	SevenZipCompressor c(lib, fname);
	c.AddDirectory(this->config().getString("targetpack"), true);
	c.DoCompress();
	
}
/*
void Application::build() {
	auto target = this->config().getString("target");
	auto pack = this->config().getString("pack");
	auto filepath = target + "\\" + pack + ".7z";
	bit7z::Bit7zLibrary lib(L"7z.dll");
	bit7z::BitCompressor compressor(lib, bit7z::BitFormat::SevenZip);
	//std::vector<std::wstring> files;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::string narrow = converter.to_bytes(wide_utf16_source_string);
	std::wstring wide = converter.from_bytes(target+"\\"+pack);
	std::wstring _fn = converter.from_bytes(filepath);
	compressor.compressDirectory(wide,_fn);
	
}*/

void Application::copySource(const std::vector<Poco::File>& files) {
	auto sourcedir = this->config().getString("source");
	auto targetsrc = this->config().getString("target") +"\\"+this->config().getString("pack");
	(Poco::File(targetsrc)).createDirectory();
	this->config().setString("targetpack", targetsrc);
	targetsrc += "\\src";
	(Poco::File(targetsrc)).createDirectory();
	if (sourcedir[sourcedir.length() - 1] == '\\')
		sourcedir = sourcedir.substr(0, sourcedir.length() - 2);
	for (auto file : files) {
		auto remaining = Application::subtractPaths(sourcedir, file.path());
		auto rm = Application::removeFilename(targetsrc + remaining);
		if(rm.length() >0)
			(Poco::File(rm)).createDirectories();
		file.copyTo(targetsrc + remaining);
	}
}

void Application::copyHeaders(const std::vector<Poco::File>& files) {
	auto includedir = this->config().getString("include");
	auto targetinc = this->config().getString("targetpack") + "\\include";
	(Poco::File(targetinc)).createDirectory();
	if (includedir[includedir.length() - 1] == '\\')
		includedir = includedir.substr(0, includedir.length() - 2);
	for (auto file : files) {
		auto remaining = Application::subtractPaths(includedir, file.path());
		auto rm = Application::removeFilename(targetinc + remaining);
		if (rm.length() > 0)
			(Poco::File(rm)).createDirectories();
		file.copyTo(targetinc + remaining);
	}
}

std::string Application::removeFilename(std::string path) {

	while(path.length()!=0) {
		if (path[path.length()-1] == '\\') break;
		path = path.substr(0, path.length() - 1);
	}
	return path;
}

std::string Application::getFilename(std::string path) {
	std::string temp;
	for (int i = path.length() - 1; i >= 0; i--) {
		if (path[i] == '\\') break;
		temp += path[i];
	}
	std::reverse(temp.begin(), temp.end());
	return temp;
}

std::string Application::subtractPaths(std::string shorter, std::string longer) {
	return longer.substr(shorter.length(), longer.length() - 1);
}

/// <summary>
/// Determines whether the specified file has the specific extension.
/// </summary>
/// <param name="file">The file.</param>
/// <param name="extension">The extension vector</param>
/// <returns>
/// {D255958A-8513-4226-94B9-080D98F904A1}  <c>true</c> if the specified file has the specific extension; otherwise, <c>false</c>.
/// </returns>
bool Application::hasExtension(Poco::File& file, const std::vector<std::string>& extension) {
	std::string temp;
	auto filename = file.path();
	for (UINT i = filename.size() - 1; i >= 0; i--) {
		if (filename[i] == '.')
			break;
		temp += filename[i];
	}
	std::reverse(temp.begin(), temp.end());
	return std::find(extension.begin(), extension.end(), temp) == extension.end() ? false : true;
}

/// <summary>
/// Iterates the subfolder.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="extension">The extensions.</param>
/// <returns>A Vector with found files</returns>
std::vector<Poco::File> Application::iterateSubfolder(const Poco::File& path, const std::vector<std::string>& extension) {
	using namespace Poco;
	std::vector<File> result;
	DirectoryIterator it(path);
	DirectoryIterator end;
	for (; it != end; it++) {
		if (it->isFile()) {
			if (Application::hasExtension(*it, extension))
				result.insert(result.end(), *it);
			continue;
		}
		if (it->isDirectory()) {
			auto res = this->iterateSubfolder(*it, extension);
			result.insert(result.end(), res.begin(), res.end());
		}
	}
	return result;
}



/// <summary>
/// Clones a GIT Repository
/// </summary>
/// <param name="arg">useless</param>
/// <param name="url">The GIT URL.</param>
void Application::gitclone(const std::string& arg, const std::string& url) {

	this->action = "GIT";
	git_libgit2_init();
	git_repository* repo = NULL;
	git_clone_options opt = GIT_CLONE_OPTIONS_INIT;
	opt.checkout_opts.progress_cb = &standalone::progress;
	opt.checkout_opts.progress_payload = (void*)this;
	opt.fetch_opts.callbacks.transfer_progress = &standalone::fetch_progress;
	opt.fetch_opts.callbacks.payload = this;
	int ret = git_clone(&repo, url.c_str(), this->config().getString("target").c_str(), &opt);
	
	git_repository_free(repo);
	git_libgit2_shutdown();
	return;
}

/// <summary>
/// Shows the help.
/// </summary>
/// <param name="key">useless</param>
/// <param name="val">useless</param>
void Application::showHelp(const std::string& key, const std::string& val) {
	Poco::Util::HelpFormatter helpformatter(options());
	helpformatter.setCommand(commandName());
	helpformatter.setUsage("OPTIONS libraryname1 libraryname2 libraryname3");
	helpformatter.setHeader("HELP");
	helpformatter.format(std::cout);
}

/// <summary>
/// Sets the action.
/// </summary>
/// <param name="key">The Action</param>
/// <param name="val">useless but POCO requires it</param>
void Application::setAction(const std::string& key, const std::string& val) {
	if (!this->action.empty()) {
		std::cout << "It is not allowed to have two Actions at once! Closing application\nERROR 1";
		
	}
	this->action = key;
}

/// <summary>
/// Parses String Options for further use!
/// </summary>
/// <param name="key">The key as referenced const STL String.</param>
/// <param name="value">The value as referenced const STL String.</param>
void Application::setOption(const std::string& key, const std::string& value) {
	this->config().setString(key, value);
}

/// <summary>
/// Adds the library to Download Queue. Alternative usage.
/// </summary>
/// <param name="useless">The useless, pass empty string.</param>
/// <param name="value">STL String containing the name of the Library.</param>
void Application::addLib(const std::string& useless, const std::string& key) {
	if (std::find(this->libname.begin(), this->libname.end(), key) != libname.end()) {
		std::cout << "The Library with name " + key + " already exists in List. Will be skipped!\n";
		return;
	}
	this->libname.insert(libname.end(), key);
}


/// <summary>
/// Downloads the file.
/// </summary>
/// <param name="target">The target.</param>
/// <param name="filepath">The Destination filepath.</param>
/// <param name="filename">The filename.</param>
/// <returns></returns>
bool Application::downloadFile(FuncTarget target, std::string filepath, std::string filename) {
	Poco::Net::HTTPStreamFactory::registerFactory();
	Poco::Net::HTTPSStreamFactory::registerFactory();
	Poco::Net::initializeSSL();
	Poco::Net::SSLManager::InvalidCertificateHandlerPtr ptrHandler(new Poco::Net::AcceptCertificateHandler(false));
	Poco::Net::Context::Ptr ptrContext(new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, ""));
	Poco::Net::SSLManager::instance().initializeClient(0, ptrHandler, ptrContext);
	auto& opener = Poco::URIStreamOpener::defaultOpener();

	//HERE DOWNLOAD
	switch (target) {
	case FuncTarget::DATABASE_UPDATE:
	{
		auto uri = Poco::URI{ std::string("http://septest.bplaced.net/sqlite/database.sqlite") };
		auto is = opener.open(uri);
		std::ofstream file("database.sqlite", std::ios::out | std::ios::trunc | std::ios::binary);
		Poco::StreamCopier::copyStream(*is, file);
		file.close();
		
	}
		return true;
	case FuncTarget::LIBRARIES: {
		auto uri = Poco::URI{ std::string("http://" + filepath) };
		auto is = opener.open(uri);
		Poco::File dir("temp");
		dir.createDirectory();
		std::ofstream file("temp/"+filename+".7z", std::ios::out | std::ios::trunc | std::ios::binary);
		Poco::StreamCopier::copyStream(*is, file);
		file.close();
	}
		return true;
	default:
		std::cout << "There was an error processing Download!\n";
		return true;
	}
	return false;
	//auto uri = Poco::URI{ std::string("https://github.com") };
	//auto is = opener.open(uri);
	//Poco::StreamCopier::copyStream(*is, std::cout);
}

/// <summary>
/// Mains the specified arguments.
/// </summary>
/// <param name="arguments">The arguments.</param>
/// <returns>ERROR CODE</returns>
int Application::main(const std::vector<std::string>& arguments) {
	if (this->action == "package") {
		this->packCode();
		return ERROR_SUCCESS;
	}
	auto text = this->config().getString("target");
	if (this->action == "GIT") return ERROR_SUCCESS;
	std::cout << "Target dir is " << HasOption("target") << '\n'
		<<"Library will be (static||dynamic) " << Application::HasOption("library")<<'\n'
		<< "The Architecture is " << HasOption( "architecture") << '\n'
		<<"Following Libraries will be installed:\n"
		;
	for (auto argument : arguments) {
		if (argument[0] != '/') {
			//std::cout << argument << '\n';
			this->addLib("",argument);
		}
	}
	for (auto lib : libname) {
		std::cout << lib << '\n';
	}
	
	this->checkfile();
	if (this->action == "update") {
		auto erg = this->downloadFile(FuncTarget::DATABASE_UPDATE);
		return erg;
	}
	//SEARCH LIBs
	std::vector< std::tuple<std::string, std::string, std::string, std::string, std::string>> todownload;
	for (auto lib : this->libname) {
		auto res = this->searchLibs(lib);
		todownload.insert(todownload.end(), res.begin(), res.end());
	}
	if (this->action == "search") return ERROR_SUCCESS;
	//DEBUG
	for (auto dl : todownload) {

		std::cout << "URL=" + std::get<4>(dl) << '\n';
		this->downloadFile(FuncTarget::LIBRARIES, std::get<4>(dl));
		this->extractFile("temp/tmp.7z");
		(Poco::File("temp/tmp.7z")).remove();
	}
	this->prepareDependencies();
	if (this->dependencies.size() > 0) {
		std::cout << "Do you want to download the Dependencies too?\n";
		std::string y;
		std::getline(std::cin, y);
		boost::algorithm::to_lower(y);
		if ((y == "yes") || (y == "y")) {
			
			
			for (auto d : this->dependencies) {
				
				auto res = this->searchLibByID(d);
				this->downloadFile(FuncTarget::LIBRARIES, this->getSourceByLibId(std::get<0>(res)));
				(Poco::File("temp/tmp.7z")).remove();
			}
		}
		
	}

	system("pause");
	return ERROR_SUCCESS;
}

/// <summary>
/// Removes the zeros inside a vector.
/// </summary>
/// <param name="vec">The vector as Reference.</param>
void Application::removeZeros(std::vector<std::string>& vec) {
	auto iter = vec.begin();
	while (iter != vec.end()) {
		if (*iter == "0") {
			iter = vec.erase(iter);
			continue;
		}
		iter++;
	}
}


/// <summary>
/// Gets the source url by library identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns>URL to download</returns>
std::string Application::getSourceByLibId(std::string id) {
	using Poco::Data::Session;
	using Poco::Data::Keywords::into;
	using Poco::Data::Keywords::range;
	using Poco::Data::Statement;
	Session session("SQLite", "database.sqlite");
	Statement select(session);
	std::string source,result;
	select << "SELECT source FROM tbl_download WHERE lib_id=" + id,
		into(source),
		range(0, 1);
	while (!select.done()) {
		select.execute();
		result = source;
		break;
	}
	session.close();
	return result;
}


/// <summary>
/// Prepares the dependencies Vector for processing.
/// </summary>
void Application::prepareDependencies() {
	std::vector<std::string> deps;
	for (auto d : this->dependencies) {
		std::vector<std::string> spVec;
		boost::algorithm::split(spVec,d, boost::is_any_of(";"));
		deps.insert(deps.end(), spVec.begin(), spVec.end());
	}
	std::sort(deps.begin(), deps.end());
	deps.erase(std::unique(deps.begin(), deps.end()),deps.end());
	this->dependencies = deps;
	Application::removeZeros(this->dependencies);
}

/// <summary>
/// Determines if there is an Option with this specific name
/// </summary>
/// <param name="name">The name of the option.</param>
/// <returns>The Arguement of the Option or a hint that this option is not set</returns>
std::string Application::HasOption(std::string name) {
	if (this->config().hasProperty(name)) {
		auto text = this->config().getString(name);
		return text;
	}
	else {
		return std::string("Not set. Will be default");
	}
}

/// <summary>
/// Check if Database exists. If not it will download it!
/// </summary>
/// <returns>the filename of the Database</returns>
std::string Application::checkfile() {
	using Poco::File;
	using Poco::Timestamp;
	File db("database.sqlite");
	if (db.exists()) {
		auto lm = db.getLastModified();
		Timestamp ts;
		//ts.update();
		auto diff = ts - lm;
		std::cout << "The Time Difference of the File is: " << diff << '\n';
		return "database.sqlite";
	}

	if (this->downloadFile(FuncTarget::DATABASE_UPDATE)) {
		return std::string("database.sqlite");
	}
	return "ERROR";
}

/// <summary>
/// Searches for the libraries.
/// </summary>
/// <param name="name">The name of the wanted library.</param>
/// <returns>A Vector of Informations about the library</returns>
std::vector<std::tuple<std::string, std::string, std::string, std::string, std::string>> Application::searchLibs(std::string name) {
	using Poco::Data::Session;
	using Poco::Data::Keywords::into;
	using Poco::Data::Keywords::range;	
	using Poco::Data::Statement;
	Poco::Data::SQLite::Connector::registerConnector();
	Session session("SQLite", "database.sqlite");
	std::vector<std::tuple<std::string, std::string, std::string, std::string,std::string>> result;
	Statement select(session);
	std::string id, dependencies, version;
	std::vector<std::tuple<std::string, std::string, std::string>> choice;
	select << "SELECT id, dependencies, version FROM tbl_libraries WHERE name='" + name + "'",
		into(id),
		into(dependencies),
		into(version),
		range(0, 1);
	UINT index = 1;
	std::cout << "Following Matches were found for "+name+":\n";
	while (select.done() != true) {
		select.execute();
		choice.insert(choice.end(), std::tuple(id, dependencies, version));
		std::cout <<"["+std::to_string(index)+"]"<< "ID=" << id << " Version=" << version << '\n';
	}
	if (this->action == "search") return std::vector<std::tuple<std::string, std::string, std::string, std::string,std::string>>();
	int c = 0;
	while (true) {
		std::cout << "Choose your library!\n";
		std::cin >> c;
		std::cout << '\n';
		if (c < 1) return result;
		if (c <= choice.size()) break;
	}
	auto libid = std::get<0>(choice[c-1]);
	this->dependencies.insert(this->dependencies.end(),std::get<1>(choice[c - 1]));
	std::string dl_id, _version, linkage, _interface, source;
	Statement select2(session);
	select2 << "SELECT dl_id, version,linkage, interface,source FROM tbl_download WHERE lib_id=" + libid,
		into(dl_id),
		into(_version),
		into(linkage),
		into(_interface),
		into(source),
		range(0, 1);
	while (!select2.done()) {
		select2.execute();
		result.insert(result.end(), std::tuple(dl_id, _version, linkage, _interface, source));
	}
	session.close();
	return result;
}

/// <summary>
/// Extracts the file.
/// </summary>
/// <param name="filepath">Target Directory for the files.</param>
void Application::extractFile(std::string filepath) {
	Poco::SevenZip::Archive zip(filepath);
	zip.failed += Poco::delegate(this, &Application::onExtractError);
	zip.extracted += Poco::delegate(this, &Application::onSevenZipSuccess);
	zip.extract(this->config().getString("target")+"/");
	

}

/// <summary>
/// On Extraction success.
/// </summary>
/// <param name="args">The Event arguments.</param>
void Application::onSevenZipSuccess(const Poco::SevenZip::Archive::ExtractedEventArgs& args) {
	
}

/// <summary>
/// Ons the extract error.
/// </summary>
/// <param name="args">The Event arguments.</param>
void Application::onExtractError(const Poco::SevenZip::Archive::FailedEventArgs& args) {
	std::cout << args.pException->message() << '\n';
	(Poco::File("temp/backup")).createDirectory();
	(Poco::File("temp/tmp.7z")).copyTo("temp/backup/tmp.7z");
	//(Poco::File("temp/tmp.7z")).remove();
}

/// <summary>
/// Searches for the library by identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns>Informations about the wanted libraries</returns>
std::tuple<std::string, std::string, std::string, std::string> Application::searchLibByID(std::string id) {
	using Poco::Data::Session;
	using Poco::Data::Keywords::into;
	using Poco::Data::Keywords::range;
	using Poco::Data::Statement;

	Session session("SQLite", "database.sqlite");
	Statement select(session);
	std::string name, version, dependencies;
	std::tuple<std::string, std::string, std::string, std::string> result;
	select << "SELECT name, version, dependencies FROM tbl_libraries",
		into(name),
		into(version),
		into(dependencies),
		range(0, 1);
	while (!select.done()) {
		select.execute();
		result = std::tuple(id, name, version, dependencies);
		break;
	}
	session.close();
	return result;
}