#pragma once
#include "Board.h"
#include "Player.h"

class Chess {
public:
	explicit Chess(const std::string& fen
		= std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));

	[[nodiscard]] const std::string& GetPGN() const { return m_PGN; }
	[[nodiscard]] const Board& GetBoard() const {return m_Board; }

	Chess& ApplyMove(const Move& move);
	Chess& ApplyMove(const Coord& from, const Coord& to) { return ApplyMove({from, to, std::nullopt}); }
	Chess& ApplyMove(const std::string& notation) { return ApplyMove(Chess2Move(notation)); }

	[[nodiscard]] bool IsGameOver() const;

	[[nodiscard]] const std::vector<Move>& GetLegalMoves() const;
	[[nodiscard]] Move GetRandomLegalMove() const;

	friend std::ostream& operator<<(std::ostream& ostream, const Chess& chess);
private:
	[[nodiscard]] bool IsMoveLegal(const Move& move) const;
	[[nodiscard]] static std::string TrimFen(const std::string& fen) ;

	std::string m_PGN;
	Board m_Board;

	std::vector<std::string> m_ReachedFens;
};