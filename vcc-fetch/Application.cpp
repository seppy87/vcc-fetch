#include"Application.hpp"
#include<fstream>
#include<boost/algorithm/string.hpp>

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
		Option("help","h","HELP").repeatable(false).noArgument().required(false).callback(OptionCallback<Application>(this,&Application::showHelp))
	);
}

/// <summary>
/// Shows the help.
/// </summary>
/// <param name="key">useless</param>
/// <param name="val">useless</param>
void Application::showHelp(const std::string& key, const std::string& val) {
	Poco::Util::HelpFormatter helpformatter(options());
	helpformatter.setCommand(commandName());
	helpformatter.setUsage("OPTIONS");
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
/// <param name="target">To determine whether to download the Database Update or to download a library</param>
/// <param name="filepath">Library name.</param>
/// <returns></returns>
bool Application::downloadFile(FuncTarget target, std::string filepath) {
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
		std::ofstream file("temp/tmp.7z", std::ios::out | std::ios::trunc | std::ios::binary);
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
	auto text = this->config().getString("target");
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