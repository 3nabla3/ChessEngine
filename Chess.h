#pragma once

enum class Player: bool {
	White, Black
};

static std::ostream& operator<<(std::ostream& ostream, const Player player){
	if (player == Player::White)
		ostream << "white";
	else
		ostream << "black";
	return ostream;
}

typedef std::array<char, 8 * 8> Board;
typedef std::pair<int, int> Coord;

typedef struct Move {
	explicit Move(Coord from, Coord to)
	 	:from(std::move(from)), to(std::move(to)) {}

	Move(Coord from, Coord to, std::optional<char> promote)
		:from(std::move(from)), to(std::move(to)), promote(promote){
	}

	Move() = default;

	bool operator==(const Move& move) const {
		return from == move.from && to == move.to && promote == move.promote;
	}

	Coord from;
	Coord to;
	std::optional<char> promote;
} Move;

static std::ostream& operator<<(std::ostream& ostream, const Board& board) {

	int rowCount = 8;
	ostream << rowCount-- << " | ";

	for (int i = 0; i < board.size(); i++) {
		char piece = board[i];
		if (piece == ' ')
				piece = '.';
		ostream << piece << " ";

		if (i % 8 == 7 and rowCount > 0) {
			ostream << std::endl << rowCount-- << " | ";
		}
	}

	ostream << "\n   ----------------";
	ostream << "\n    a b c d e f g h";

	return ostream;
}

static Coord Chess2Coord(const char* notation) {
	char file = notation[0];
	char row = notation[1];
	return {file - 'a', row - '1'};
}

static Move Chess2Move(const char* notation) {
	/// accept input such as 	"b6b7" to move from b7 to b7
	///							"b7b8R" to promote to rook after move
	Coord from = Chess2Coord(notation);
	Coord to = Chess2Coord(notation + 2);

	// if there is a promotion included
	if (strlen(notation) > 4)
		return {from, to, notation[4]};

	return {from, to, std::nullopt};
}

static std::ostream& operator<<(std::ostream& ostream, const Coord& coord){
	auto [coord_col, coord_row] = coord;
	char coord_file = (char)('a' + coord_col);
	char coord_rank = (char)(coord_row + 1 + '0');

	ostream << coord_file << coord_rank;
	return ostream;
}

static std::ostream& operator<<(std::ostream& ostream, const Move& move) {
	ostream << move.from << " -> " << move.to;
	if (move.promote)
		ostream << "=" << move.promote.value();

	return ostream;
}

class Chess {
public:
	constexpr static int SIZE = 8;

	explicit Chess(const std::string& fen
		= std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));

	[[nodiscard]] std::string GetFen() const;
	[[nodiscard]] const std::string& GetPGN() const { return m_PGN; }
	[[nodiscard]] inline Player GetCurrentPlayer() const { return m_Playing; }
	[[nodiscard]] inline Player GetNotCurrentPlayer() const {
		return m_Playing == Player::White ? Player::Black : Player::White;
	}

	[[nodiscard]] inline char GetPiece(const Coord& coord) const { return GetPiece(coord.first, coord.second); }
	[[nodiscard]] inline char GetPiece(int col, int row) const { return m_Board[CoordToIndexInBoard(col, row)]; }
	[[nodiscard]] inline char& GetPieceRef(int col, int row) { return m_Board[CoordToIndexInBoard(col, row)]; }

	[[nodiscard]] std::list<Move> GetPseudoLegalMoves() const;
	[[nodiscard]] const std::list<Move>& GetLegalMoves() const;
	[[nodiscard]] bool IsCheck() const;

	Chess& ApplyMove(const Move& move);
	Chess& ApplyMove(const Coord& from, const Coord& to) { return ApplyMove({from, to, std::nullopt}); }

	static Board BoardFromFen(const std::string& fen);
	friend std::ostream& operator<<(std::ostream& ostream, const Chess& chess);
private:
	[[nodiscard]] std::list<Move> GetLegalMovesForPiece(int col, int row) const;

	[[nodiscard]] std::list<Move> GetPseudoLegalMovesRook(int col, int row) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesKnight(int col, int row) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesBishop(int col, int row) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesQueen(int col, int row) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesKing(int col, int row) const;
	[[nodiscard]] std::list<Move> GetPseudoLegalMovesPawn(int col, int row) const;

	[[nodiscard]] bool IsMoveLegal(const Move& move) const;

	[[nodiscard]] bool IsCurrentPlayerPiece(char piece) const;

	[[nodiscard]] static std::optional<Coord> ParseEnPassantFromFen(const std::string& fen) ;

	constexpr static int CoordToIndexInBoard(int col, int row) {
		if (col < 0 or col > SIZE - 1 or row < 0 or row > SIZE - 1)
			std::cout << "ERROR: out of bounds in board" << std::endl;
		return col + SIZE * (SIZE - row - 1);
	}

	std::optional<Coord> m_EnPassant;
	Player m_Playing = Player::White;
	Board m_Board{};
	int m_halfMovesRule = 0;
	int m_fullMoves = 1;
	std::string m_PGN;

	// caching legal moves when possible
	mutable std::list<Move> m_LegalMovesCache;
};