#include "pch.h"
#include "Chess.h"
#include "NetworkHandler.h"

int main() {
	// NetworkHandler::Init("localhost:5000");
	//
	// ErrorCode code = NetworkHandler::SendMove("e2e4");
	// if (code == ErrorCode::IllegalMove)
	// 	std::cout << "Illegal move" << std::endl;
	// else if (code == ErrorCode::CurlError)
	// 	std::cout << "Error when calling curl" << std::endl;
	// else
	// 	std::cout << "Moved the piece" << std::endl;
	//
	// std::cout << NetworkHandler::GetTimeSecondsLeft() << std::endl;

	srand(time(nullptr));

	// Chess c("7k/8/Bp2r2K/8/1p1pn3/8/3r3P/3r4 w - - 0 1");
	Chess c;
	// std::cout << c << std::endl;
	//
	// for (const Move& m : c.GetLegalMoves()) {
	// 	std::cout << m << std::endl;
	// }
	//
	// std::cout << c.GetLegalMoves().size() << std::endl;
	//
	// std::string line;
	// std::cin >> line;
	//
	// c.ApplyMove(Chess2Move(line.c_str()));
	std::cout << c << std::endl;

	for (int i = 0; i < 400; i++) {
		auto moves = c.GetLegalMoves();
		if (moves.empty())
			break;

		int randMoveIndex = rand() % moves.size();

		int j = 0;
		Move chosen;
		for (const auto& move: moves) {
			if (j++ == randMoveIndex) {
				chosen = move;
				break;
			}
		}

		c.ApplyMove(chosen);

		std::cout << c << std::endl;
	}

	if (c.GetLegalMoves().empty())
		std::cout << "Mate or draw" << std::endl;

	std::cout << c.GetPGN() << std::endl;

	return 0;
}
