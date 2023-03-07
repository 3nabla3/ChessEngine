#pragma once
#include "Board.h"
#include "Player.h"

class Chess {
public:
	constexpr static int SIZE = 8;

	explicit Chess(const std::string& fen
		= std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));

	[[nodiscard]] std::string GetFen() const;
	[[nodiscard]] const std::string& GetPGN() const { return m_PGN; }
	[[nodiscard]] int GetFullMoves() const {return m_fullMoves;}
	[[nodiscard]] inline Player GetCurrentPlayer() const { return m_Playing; }
	[[nodiscard]] inline Player GetNotCurrentPlayer() const {
		return m_Playing == Player::White ? Player::Black : Player::White;
	}

	[[nodiscard]] const std::list<Move>& GetLegalMoves() const;

	Chess& ApplyMove(const Move& move);
	Chess& ApplyMove(const Coord& from, const Coord& to) { return ApplyMove({from, to, std::nullopt}); }
	Chess& ApplyMove(const std::string& notation) { return ApplyMove(Chess2Move(notation)); }

	Move GetRandomLegalMove() const;

	static Board BoardFromFen(const std::string& fen);

	friend std::ostream& operator<<(std::ostream& ostream, const Chess& chess);
private:
	[[nodiscard]] bool IsMoveLegal(const Move& move) const;

	[[nodiscard]] static std::optional<Coord> ParseEnPassantFromFen(const std::string& fen);
	[[nodiscard]] static CastlingRights ParseCastlingRightsFromFen(const std::string& fen, Player player);


	std::optional<Coord> m_EnPassant;

	// king side, queen side
	CastlingRights m_WhiteCastlingRights;
	CastlingRights m_BlackCastlingRights;

	Player m_Playing = Player::White;
	Board m_Board{};
	int m_halfMovesRule = 0;
	int m_fullMoves = 1;
	std::string m_PGN;

	// caching legal moves when possible
	mutable std::list<Move> m_LegalMovesCache;
};