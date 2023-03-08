#include "pch.h"
#include "Chess.h"
#include "NetworkHandler.h"

int main() {
	NetworkHandler::Get().Init("172.20.10.2:5000");

	Chess c;
	for (int i = 0; i < 200; i++) {
		Move move = c.GetRandomLegalMove();
		c.ApplyMove(move);
		std::cout << "Playing " << move << std::endl;
		if (c.GetLegalMoves().empty()) {
			std::cout << "Game over" << std::endl;
			break;
		}
	}
	std::cout << c.GetPGN() << std::endl;

	// LoginResponse loginResp = NetworkHandler::Get().Login("Alban");
	//
	// std::cout << "Login status code " << loginResp.statusCode << std::endl;
	// Player player = loginResp.player;
	// std::cout << "Playing as " << player << std::endl;

	// for (int i = 0; i < 200; i++) {
	// 	std::cout << c << std::endl;
	// 	if (c.GetCurrentPlayer() == player) {
	// 		Move move = c.GetRandomLegalMove();
	// 		c.ApplyMove(move);
	// 		std::cout << "Sending " << move << std::endl;
	// 		HttpResponse resp = NetworkHandler::Get().SendMove(move);
	// 		std::cout << "Status: " << resp.statusCode << std::endl;
	// 		if (resp.statusCode != StatusCode::OK)
	// 			break;
	// 	}
	// 	else {
	// 		Move move = NetworkHandler::Get().GetMove();
	// 		std::cout << "Received " << move << std::endl;
	// 		c.ApplyMove(move);
	// 	}
	// 	if (c.GetLegalMoves().empty()) {
	// 		std::cout << "Game over" << std::endl;
	// 		break;
	// 	}
	// }
	//
	// std::cout << c.GetPGN() << std::endl;

	return 0;
}
