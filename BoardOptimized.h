#include "Move.h"
#include "Player.h"

// this board uses bit boards to store the pieces
typedef uint64_t BitBoard;

struct SMagic {
	BitBoard* attack_table_ptr;  // pointer to attack_table for each particular square
	BitBoard mask;  // to mask relevant squares of both lines (no outer squares)
	BitBoard magic; // magic 64-bit factor
	int shift; // shift right
};

enum enumSquare {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

class BoardOptimized {
public:
	explicit BoardOptimized(const std::string& fen);

	[[nodiscard]] std::vector<Move> GetLegalMoves() const;

	[[nodiscard]] inline BitBoard GetAllPieces() const {
		return GetWhitePieces() | GetBlackPieces();
	}

	[[nodiscard]] inline BitBoard GetWhitePieces() const {
		return m_WhitePawns | m_WhiteKnights | m_WhiteBishops | m_WhiteRooks | m_WhiteQueens | m_WhiteKing;
	}

	[[nodiscard]] inline BitBoard GetBlackPieces() const {
		return m_BlackPawns | m_BlackKnights | m_BlackBishops | m_BlackRooks | m_BlackQueens | m_BlackKing;
	}

	[[nodiscard]] inline BitBoard GetWhitePawns() const { return m_WhitePawns; }
	[[nodiscard]] inline BitBoard GetWhiteKnights() const { return m_WhiteKnights; }
	[[nodiscard]] inline BitBoard GetWhiteBishops() const { return m_WhiteBishops; }
	[[nodiscard]] inline BitBoard GetWhiteRooks() const { return m_WhiteRooks; }
	[[nodiscard]] inline BitBoard GetWhiteQueens() const { return m_WhiteQueens; }
	[[nodiscard]] inline BitBoard GetWhiteKing() const { return m_WhiteKing; }

	[[nodiscard]] inline BitBoard GetBlackPawns() const { return m_BlackPawns; }
	[[nodiscard]] inline BitBoard GetBlackKnights() const { return m_BlackKnights; }
	[[nodiscard]] inline BitBoard GetBlackBishops() const { return m_BlackBishops; }
	[[nodiscard]] inline BitBoard GetBlackRooks() const { return m_BlackRooks; }
	[[nodiscard]] inline BitBoard GetBlackQueens() const { return m_BlackQueens; }
	[[nodiscard]] inline BitBoard GetBlackKing() const { return m_BlackKing; }

	friend std::ostream& operator<<(std::ostream& os, BoardOptimized bb);

	static void PrintBitBoard(BitBoard board) {
		for (int i = 0; i < 64; i++) {
			std::cout << ((board << i) & 0x8000000000000000 ? '1' : '0');
			if (i % 8 == 7)
				std::cout << '\n';
		}
		std::cout << '\n';
	}
private:
	[[nodiscard]] BitBoard PawnAttacks(Player player) const;
	[[nodiscard]] BitBoard KnightAttacks(Player player) const;

	[[nodiscard]] BitBoard BishopAttacks(BitBoard occ, enumSquare sq) const;
	[[nodiscard]] BitBoard RookAttacks(BitBoard occ, enumSquare sq) const;

	BitBoard m_WhitePawns = 0;
	BitBoard m_WhiteKnights = 0;
	BitBoard m_WhiteBishops = 0;
	BitBoard m_WhiteRooks = 0;
	BitBoard m_WhiteQueens = 0;
	BitBoard m_WhiteKing = 0;

	BitBoard m_BlackPawns = 0;
	BitBoard m_BlackKnights = 0;
	BitBoard m_BlackBishops = 0;
	BitBoard m_BlackRooks = 0;
	BitBoard m_BlackQueens = 0;
	BitBoard m_BlackKing = 0;

	static constexpr BitBoard s_FILE_H = 0x0101010101010101ULL;
	static constexpr BitBoard s_FILE_G = s_FILE_H << 1;
	static constexpr BitBoard s_FILE_F = s_FILE_H << 2;
	static constexpr BitBoard s_FILE_E = s_FILE_H << 3;
	static constexpr BitBoard s_FILE_D = s_FILE_H << 4;
	static constexpr BitBoard s_FILE_C = s_FILE_H << 5;
	static constexpr BitBoard s_FILE_B = s_FILE_H << 6;
	static constexpr BitBoard s_FILE_A = s_FILE_H << 7;

	static constexpr BitBoard s_RANK_1 = 0xFF;
	static constexpr BitBoard s_RANK_2 = s_RANK_1 << (8 * 1);
	static constexpr BitBoard s_RANK_3 = s_RANK_1 << (8 * 2);
	static constexpr BitBoard s_RANK_4 = s_RANK_1 << (8 * 3);
	static constexpr BitBoard s_RANK_5 = s_RANK_1 << (8 * 4);
	static constexpr BitBoard s_RANK_6 = s_RANK_1 << (8 * 5);
	static constexpr BitBoard s_RANK_7 = s_RANK_1 << (8 * 6);
	static constexpr BitBoard s_RANK_8 = s_RANK_1 << (8 * 7);

	SMagic m_BishopTable[64]{};
	SMagic m_RookTable[64]{};
};