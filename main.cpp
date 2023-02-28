#include "pch.h"
#include "Chess.h"
#include "NetworkHandler.h"

int main() {
	NetworkHandler::Get().Init("localhost:5000");
	NetworkHandler::Get().Login("Alban");

	Player player = Player::White;
	Chess c;

	for (int i = 0; i < 200; i++) {
		std::cout << c << std::endl;
		if (c.GetCurrentPlayer() == player) {
			Move move = c.GetRandomLegalMove();
			c.ApplyMove(move);
			NetworkHandler::Get().SendMove(move);
		}
		else {
			Move move = NetworkHandler::Get().GetMove();
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
