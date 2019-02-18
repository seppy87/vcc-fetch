#include"Application.hpp"
#include<fstream>

void Application::initialize(Poco::Util::Application& application) {
	//this->config().setString("target", "defaultoption");
	this->loadConfiguration();
	Poco::Util::Application::initialize(application);
}

void Application::uninitialize() {
	Poco::Net::uninitializeSSL();
	Poco::Util::Application::uninitialize();
}

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
		Option("name", "n", "Name of the Library you want to download", true).repeatable(true).argument("name", true).callback(OptionCallback<Application>(this, &Application::addLib))
	);
	optionSet.addOption(
		Option("install","", "Installs the library",false).repeatable(false).group("ActionGroup").noArgument().callback(OptionCallback<Application>(this,&Application::setAction))
	);
	optionSet.addOption(
		Option("search", "", "searches for the library", false).repeatable(false).group("ActionGroup").noArgument().callback(OptionCallback<Application>(this, &Application::setAction))
	);
	optionSet.addOption(
		Option("help","h","HELP").repeatable(false).noArgument().required(false).callback(OptionCallback<Application>(this,&Application::showHelp))
	);
}

void Application::showHelp(const std::string& key, const std::string& val) {
	Poco::Util::HelpFormatter helpformatter(options());
	helpformatter.setCommand(commandName());
	helpformatter.setUsage("OPTIONS");
	helpformatter.setHeader("HELP");
	helpformatter.format(std::cout);
}

void Application::setAction(const std::string& key, const std::string& val) {
	if (!this->action.empty()) {
		std::cout << "It is not allowed to have two Actions at once! Closing application\nERROR 1";
		
	}
	this->action = key;
}

void Application::setOption(const std::string& key, const std::string& value) {
	this->config().setString(key, value);
}

void Application::addLib(const std::string& useless, const std::string& key) {
	if (std::find(this->libname.begin(), this->libname.end(), key) != libname.end()) {
		std::cout << "The Library with name " + key + " already exists in List. Will be skipped!\n";
		return;
	}
	this->libname.insert(libname.end(), key);
}

bool Application::downloadFile(FuncTarget target) {
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
		auto uri = Poco::URI{ std::string("https://mycooolsite.mycoolsite/db/update.sqlite") };
		auto is = opener.open(uri);
		std::ofstream file("database.sqlite", std::ios::out | std::ios::trunc | std::ios::binary);
		Poco::StreamCopier::copyStream(*is, file);
		file.close();
		
	}
		return true;
	case FuncTarget::LIBRARIES:
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

int Application::main(const std::vector<std::string>& arguments) {
	auto text = this->config().getString("target");
	std::cout << "Target dir is " << HasOption("target") << '\n'
		<<"Library will be (static||dynamic) " << Application::HasOption("library")<<'\n'
		<< "The Architecture is " << HasOption( "architecture") << '\n'
		<<"Following Libraries will be installed:\n"
		;
	for (auto lib : libname) {
		std::cout << lib << '\n';
	}
	//this->downloadFile();
	system("pause");
	return ERROR_SUCCESS;
}

std::string Application::HasOption(std::string key) {
	if (this->config().hasProperty(key)) {
		auto text = this->config().getString(key);
		return text;
	}
	else {
		return std::string("Not set. Will be default");
	}
}