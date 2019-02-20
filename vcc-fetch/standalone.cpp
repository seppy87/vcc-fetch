#include"Application.hpp"

class Application;

void standalone::progress(const char* path, size_t cur, size_t tot, void* payload) {
	Application* app = (Application*)payload;
	std::cout << "GIT PROGRESS\n" << path;
}

int standalone::fetch_progress(
	const git_transfer_progress *stats,
	void *payload) {
	system("cls");
	auto received = stats->received_bytes;
	std::cout << "Received: " <<received << '\n';
	return 0;
}