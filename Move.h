#pragma once

typedef std::pair<int, int> Coord;
typedef std::pair<bool, bool> CastlingRights;

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


static Coord Chess2Coord(const char* notation) {
	char file = notation[0];
	char row = notation[1];
	return {file - 'a', row - '1'};
}

static std::string Coord2Chess(const Coord& coord) {
	std::string notation;
	notation += (char)('a' + coord.first);
	notation += (char)('1' + coord.second);
	return notation;
}

static Move Chess2Move(const std::string& notation) {
	/// accept input such as 	"b6b7" to move from b7 to b7
	///							"b7b8R" to promote to rook after move
	Coord from = Chess2Coord(notation.c_str());
	Coord to = Chess2Coord(notation.c_str() + 2);

	// if there is a promotion included
	if (notation.size() > 4)
		return {from, to, notation[4]};

	return {from, to, std::nullopt};
}

static std::string Move2Chess(const Move& move) {
	std::string notation;
	notation += (char)('a' + move.from.first);
	notation += (char)('1' + move.from.second);
	notation += (char)('a' + move.to.first);
	notation += (char)('1' + move.to.second);

	if (move.promote)
		notation += move.promote.value();

	return notation;
}

static std::ostream& operator<<(std::ostream& ostream, const Coord& coord){
	auto [coord_col, coord_row] = coord;
	char coord_file = (char)('a' + coord_col);
	char coord_rank = (char)(coord_row + 1 + '0');

	ostream << coord_file << coord_rank;
	return ostream;
}

static std::ostream& operator<<(std::ostream& ostream, const Move& move) {
	ostream << move.from << move.to;
	if (move.promote)
		ostream << "=" << move.promote.value();

	return ostream;
}