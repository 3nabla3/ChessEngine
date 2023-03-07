#include "pch.h"
#include "Chess.h"
#include "NetworkHandler.h"

int main() {
	NetworkHandler::Get().Init("172.20.10.2:5000");
	NetworkHandler::Get().Login("Alban");

	Player player = Player::Black;
	Chess c;

	for (int i = 0; i < 200; i++) {
		std::cout << c << std::endl;
		if (c.GetCurrentPlayer() == player) {
			Move move = c.GetRandomLegalMove();
			c.ApplyMove(move);
			std::cout << "Sending " << move << std::endl;
			StatusCode sc = NetworkHandler::Get().SendMove(move);
			std::cout << "Status: " << sc << std::endl;
			if (sc != StatusCode::OK)
				break;
		}
		else {
			Move move = NetworkHandler::Get().GetMove();
			std::cout << "Received " << move << std::endl;
			c.ApplyMove(move);
		}
		if (c.GetLegalMoves().empty()) {
			std::cout << "Game over" << std::endl;
			break;
		}
	}

	std::cout << c.GetPGN() << std::endl;

	return 0;
}
