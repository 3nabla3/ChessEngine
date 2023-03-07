#pragma once
#include "Move.h"
#include "Player.h"

class Board {
public:
	Board() = default;
	Board(const Board& other) = default;

	[[nodiscard]] size_t size() const { return m_Board.size(); }

	void ApplyMove(const Move& move);
	void ApplyMove(const Coord& from, const Coord& to) { return ApplyMove({from, to, std::nullopt}); }
	void ApplyMove(const std::string& notation) { return ApplyMove(Chess2Move(notation)); }

	char& GetPieceRef(const Coord& coord) { return GetPieceRef(coord.first, coord.second); }
	char& GetPieceRef(int col, int row) { return m_Board[CoordToIndexInBoard(col, row)]; }
	[[nodiscard]] char GetPiece(const Coord& coord) const { return GetPiece(coord.first, coord.second); }
	[[nodiscard]] char GetPiece(int col, int row) const { return m_Board[CoordToIndexInBoard(col, row)]; }

	[[nodiscard]] bool IsCheck(Player player) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMoves(Player player, std::optional<Coord> enPassant, const CastlingRights& castlingRights) const;

	[[nodiscard]] char& operator[](size_t index) { return m_Board[index]; }

	[[nodiscard]] char operator[](size_t index) const {return m_Board[index]; }
	friend std::ostream& operator<<(std::ostream& ostream, const Board& board);
	static constexpr int SIZE = 8;
private:
	constexpr static int CoordToIndexInBoard(int col, int row) {
		if (col < 0 or col > SIZE - 1 or row < 0 or row > SIZE - 1)
			std::cout << "ERROR: out of bounds in board" << std::endl;
		return col + SIZE * (SIZE - row - 1);
	}

	[[nodiscard]] static inline bool IsPlayerPiece(char piece, Player player) {
		return (player == Player::White and std::isupper(piece))
			   or (player == Player::Black and std::islower(piece));
	}
	[[nodiscard]] std::list<Move> GetLegalMovesForPiece(int col, int row, Player player, std::optional<Coord> enPassant, const CastlingRights& castlingRights) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesRook(int col, int row, Player player) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesKnight(int col, int row, Player player) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesBishop(int col, int row, Player player) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesQueen(int col, int row, Player player) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesKing(int col, int row, Player player, const CastlingRights& castlingRights) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesPawn(int col, int row, Player player, std::optional<Coord> enPassant) const;

	std::array<char, 64> m_Board;
};