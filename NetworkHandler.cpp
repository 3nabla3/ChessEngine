#include "NetworkHandler.h"
#include <curl/curl.h>
#include <sstream>
#include <thread>

std::string NetworkHandler::s_SubmitMoveEndpoint;
std::string NetworkHandler::s_GetTimeLeftEndpoint;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void NetworkHandler::Init(const std::string& hostnamePort) {
	std::stringstream ss1;
	ss1 << "http://" << hostnamePort << "/submitmove";
	s_SubmitMoveEndpoint = ss1.str();

	std::stringstream ss2;
	ss2 << "http://" << hostnamePort << "/gettimeleft";
	s_GetTimeLeftEndpoint = ss2.str();
}

ErrorCode NetworkHandler::SendMove(const std::string& move) {
	using namespace std::chrono_literals;
	std::stringstream ss;
	// the first two chars of move are the "from" coord
	ss << "from=" << move[0] << move[1];
	ss << "&";
	ss << "to=" << move[2] << move[3];

	std::string postData = ss.str();

	CURL *curl = curl_easy_init();

	if (curl) {
		std::string readBuffer;
		curl_easy_setopt(curl, CURLOPT_URL, s_SubmitMoveEndpoint.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postData.size());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		auto res = curl_easy_perform(curl);

		long http_code = 200;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		curl_easy_cleanup(curl);

		if (http_code == 200)
			return ErrorCode::OK;
		if (http_code == 409)
			return ErrorCode::IllegalMove;
	}

	curl_easy_cleanup(curl);
	return ErrorCode::CurlError;
}

int NetworkHandler::GetTimeSecondsLeft() {
	using namespace std::chrono_literals;
	CURL *curl = curl_easy_init();
	std::string data = "player=1";

	if (curl) {
		std::string readBuffer;
		curl_easy_setopt(curl, CURLOPT_URL, s_GetTimeLeftEndpoint.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		// std::this_thread::sleep_for(1s);
		auto res = curl_easy_perform(curl);

		return stoi(readBuffer);
	}

	curl_easy_cleanup(curl);
	return -1;
}