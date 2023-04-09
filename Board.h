#pragma once
#include "Move.h"
#include "Player.h"

typedef std::array<char, 64> RawBoard;

// holds all the information that the fen holds
class Board {
public:
	explicit Board(const std::string& fen);
	Board(const Board& other) = default;

	void ApplyMove(const Move& move);
	void ApplyMove(const Coord& from, const Coord& to) { return ApplyMove({from, to, std::nullopt}); }
	void ApplyMove(const std::string& notation) { return ApplyMove(Chess2Move(notation)); }

	char& GetPieceRef(const Coord& coord) { return GetPieceRef(coord.first, coord.second); }
	char& GetPieceRef(int col, int row) { return m_Board[CoordToIndexInBoard(col, row)]; }
	[[nodiscard]] char GetPiece(const Coord& coord) const { return GetPiece(coord.first, coord.second); }
	[[nodiscard]] char GetPiece(int col, int row) const { return m_Board[CoordToIndexInBoard(col, row)]; }

	[[nodiscard]] int GetFullMoves() const {return m_fullMoves;}
	[[nodiscard]] inline bool IsWhiteTurn() const { return m_Playing == Player::White; }
	[[nodiscard]] inline Player GetCurrentPlayer() const { return m_Playing; }
	[[nodiscard]] inline Player GetNotCurrentPlayer() const {
		return IsWhiteTurn() ? Player::Black : Player::White;
	}

	[[nodiscard]] bool IsCheckmate() const;
	[[nodiscard]] bool IsCheck() const;
	[[nodiscard]] bool IsDraw() const;
	[[nodiscard]] bool IsGameOver() const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMoves() const;
	[[nodiscard]] const std::vector<Move>& GetLegalMoves() const;

	[[nodiscard]] std::string GetFen() const;

	[[nodiscard]] char operator[](size_t index) const {return m_Board[index]; }
	friend std::ostream& operator<<(std::ostream& ostream, const Board& board);
	static constexpr int SIZE = 8;
	[[nodiscard]] static unsigned long GetCacheHits() { return s_LegalMovesCacheHits;}
	[[nodiscard]] static unsigned long GetCacheMisses() { return s_LegalMovesCacheMisses;}
	[[nodiscard]] static double GetCacheHitRate() {
		return static_cast<double>(s_LegalMovesCacheHits) / static_cast<double>(s_LegalMovesCacheHits + s_LegalMovesCacheMisses);
	}
private:
	constexpr static int CoordToIndexInBoard(int col, int row) {
		if (col < 0 or col > SIZE - 1 or row < 0 or row > SIZE - 1) {
			std::cout << "ERROR: out of bounds in board" << std::endl;
			// debug break
			raise(SIGTRAP);
		}
		return col + SIZE * (SIZE - row - 1);
	}

	[[nodiscard]] static RawBoard BoardFromFen(const std::string& fen);
	[[nodiscard]] static std::optional<Coord> ParseEnPassantFromFen(const std::string& fen);
	[[nodiscard]] static CastlingRights ParseCastlingRightsFromFen(const std::string& fen, Player player);
	[[nodiscard]] std::string GetPositionalFen() const {
		std::string fen = GetFen();
		// remove the last two fields
		fen = fen.substr(0, fen.find_last_of(' '));
		fen = fen.substr(0, fen.find_last_of(' '));
		return fen;
	}

	[[nodiscard]] static inline bool IsPlayerPiece(char piece, Player player) {
		return (player == Player::White and std::isupper(piece))
			   or (player == Player::Black and std::islower(piece));
	}
	[[nodiscard]] std::vector<Move> GetLegalMovesForPiece(int col, int row) const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMovesRook(int col, int row) const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMovesKnight(int col, int row) const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMovesBishop(int col, int row) const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMovesQueen(int col, int row) const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMovesKing(int col, int row) const;
	[[nodiscard]] std::vector<Move> GetPseudoLegalMovesPawn(int col, int row) const;

	void UpdateCastlingRights(const Move& move);
	[[nodiscard]] bool IsCastlingLegal(bool kingSide) const;

	// Legal moves cache
	static unsigned long s_LegalMovesCacheHits;
	static unsigned long s_LegalMovesCacheMisses;
	static std::shared_timed_mutex s_CacheMutex;
	static std::unordered_map<std::string, std::vector<Move>> s_FenToLegalMovesCache;

	RawBoard m_Board;
	std::optional<Coord> m_EnPassant;

	// king side, queen side
	CastlingRights m_WhiteCastlingRights;
	CastlingRights m_BlackCastlingRights;

	Player m_Playing = Player::White;
	int m_halfMovesRule = 0;
	int m_fullMoves = 1;
};