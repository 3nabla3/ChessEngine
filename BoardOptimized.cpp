#include "BoardOptimized.h"

std::ostream& operator<<(std::ostream& os, BoardOptimized bb) {
	int rowCount = 8;
	os << rowCount << " | ";
	for (int i = 0; i < 64; i++) {
		unsigned long long int mask = 1ULL << i;

		if (bb.m_WhitePawns & mask)
			os << 'P';
		else if (bb.m_WhiteKnights & mask)
			os << 'N';
		else if (bb.m_WhiteBishops & mask)
			os << 'B';
		else if (bb.m_WhiteRooks & mask)
			os << 'R';
		else if (bb.m_WhiteQueens & mask)
			os << 'Q';
		else if (bb.m_WhiteKing & mask)
			os << 'K';
		else if (bb.m_BlackPawns & mask)
			os << 'p';
		else if (bb.m_BlackKnights & mask)
			os << 'n';
		else if (bb.m_BlackBishops & mask)
			os << 'b';
		else if (bb.m_BlackRooks & mask)
			os << 'r';
		else if (bb.m_BlackQueens & mask)
			os << 'q';
		else if (bb.m_BlackKing & mask)
			os << 'k';
		else
			os << '.';
		os << ' ';
		if (i % 8 == 7 and rowCount > 1)
			os << "\n" << --rowCount << " | ";
	}
	os << "\n   ----------------\n";
	os << "    a b c d e f g h";
	return os;
}

BoardOptimized::BoardOptimized(const std::string& fen) {
	std::string part;
	for (char c : fen) {
		if (c == ' ')
			break;
		part += c;
	}

	// parse the board
	int idx = 0;
	for (char c : part) {
		if (c == '/')
			continue;
		if (c >= '1' && c <= '8') {
			idx += c - '0';
			continue;
		}
		// BitBoard piece = 1ULL << idx;
		BitBoard piece = 1ULL << (63 - idx);
		switch (c) {

		case 'p':
			m_BlackPawns |= piece;
			break;
		case 'n':
			m_BlackKnights |= piece;
			break;
		case 'b':
			m_BlackBishops |= piece;
			break;
		case 'r':
			m_BlackRooks |= piece;
			break;
		case 'q':
			m_BlackQueens |= piece;
			break;
		case 'k':
			m_BlackKing |= piece;
			break;
		case 'P':
			m_WhitePawns |= piece;
			break;
		case 'N':
			m_WhiteKnights |= piece;
			break;
		case 'B':
			m_WhiteBishops |= piece;
			break;
		case 'R':
			m_WhiteRooks |= piece;
			break;
		case 'Q':
			m_WhiteQueens |= piece;
			break;
		case 'K':
			m_WhiteKing |= piece;
			break;
		default:
			throw std::runtime_error("Invalid FEN string");
		}
		idx++;
	}
}

BitBoard BoardOptimized::PawnAttacks(Player player) const {
	if (player == Player::White)
		return (m_WhitePawns & ~s_FILE_H) << 7 | (m_WhitePawns & ~s_FILE_A) << 9;
	else
		return (m_BlackPawns & ~s_FILE_A) >> 7 | (m_BlackPawns & ~s_FILE_H) >> 9;
}

BitBoard BoardOptimized::KnightAttacks(Player player) const {
	BitBoard knights = player == Player::White ? m_WhiteKnights : m_BlackKnights;
	BitBoard l1 = (knights >> 1) & ~s_FILE_A;
	BitBoard l2 = (knights >> 2) & ~(s_FILE_A | s_FILE_B);
	BitBoard r1 = (knights << 1) & ~s_FILE_H;
	BitBoard r2 = (knights << 2) & ~(s_FILE_G | s_FILE_H);
	BitBoard h1 = l1 | r1;
	BitBoard h2 = l2 | r2;
	return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

BitBoard BoardOptimized::BishopAttacks(BitBoard occ, enumSquare sq) const {
	BitBoard* attackTablePtr = m_BishopTable[sq].attack_table_ptr;
	occ      &= m_BishopTable[sq].mask;
	occ      *= m_BishopTable[sq].magic;
	occ     >>= m_BishopTable[sq].shift;
	return attackTablePtr[occ];
}

BitBoard BoardOptimized::RookAttacks(BitBoard occ, enumSquare sq) const {
	BitBoard* attackTablePtr = m_RookTable[sq].attack_table_ptr;
	occ      &= m_RookTable[sq].mask;
	occ      *= m_RookTable[sq].magic;
	occ     >>= m_RookTable[sq].shift;
	return attackTablePtr[occ];
}

std::vector<Move> BoardOptimized::GetLegalMoves() const {
	return {};
}

