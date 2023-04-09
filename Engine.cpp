#include "Engine.h"

Engine::Engine(Chess& c) : m_Chess(c),
						   m_Tree(std::make_unique<TreeNode>(TreeNode(Move(),m_Chess.GetBoard().IsWhiteTurn()))) {
}

void Engine::ApplyMove(const Move& move) {
	m_Chess.ApplyMove(move);

	// set the new head of tree
	m_Tree = std::make_unique<TreeNode>(TreeNode(Move(),m_Chess.GetBoard().IsWhiteTurn()));
	for (auto& node : m_NodesPerThread)
		node = 0;
}

void Engine::LoadingBar(const std::stop_token& st, const Score* score) {
	using namespace std::chrono_literals;
	int i = 0;
	while (!st.stop_requested()) {
		std::cout << "\r" << "[" << std::string(i, '.') <<
				  std::string(10 - i, ' ') << "]" << " Score: " << *score << "        " << std::flush;
		i = (i + 1) % 10;
		std::this_thread::sleep_for(200ms);
	}
	std::cout << "\rDone!" << std::string(100, ' ') << std::endl;
}

void Engine::ApplyThinkingPolicy() {
	std::string message = "Thinking\t";
	// std::jthread loadingThread(LoadingBar, &message);
	std::this_thread::sleep_for(std::chrono::milliseconds(m_msThinkTime));
	message = "Stopping\t";
	StopThinking();
}

Board Engine::GetBoardFromNode(TreeNode* node) const {
	PROFILE_SCOPE;

	std::stack<Move> moveStack;
	while (node->parent) {
		moveStack.push(node->delta);
		node = node->parent;
	}

	Board board = m_Chess.GetBoard();

	while (not moveStack.empty()) {
		board.ApplyMove(moveStack.top());
		moveStack.pop();
	}

	return board;
}

// expands the node with a certain depth, and calculates the score of the nodes it creates
// that score is not permanent and will change over time as the tree is expanded
void Engine::ExpandNode(TreeNode* node, int depth, int threadId) {
	const Board board = GetBoardFromNode(node);

	// if the game is over, don't expand the node
	if (board.IsGameOver()) {
		node->score = StaticEvaluation(board);
		return;
	}

	// https://en.wikipedia.org/wiki/Principal_variation_search
	// use pvs to calculate the score of the node

}

void Engine::ThreadWorker(int threadId) {
	while (true) {
		// get the next node to expand
		TreeNode* node;
		{
			std::unique_lock lock(m_QueueMutex);
			// condition must be true to continue
			m_QueueCondition.wait(lock, [this] { return !m_Queue.empty() or !m_Thinking; });

			if (!m_Thinking){
				return;
			}

			node = m_Queue.front();
			m_Queue.pop();
		}
		ExpandNode(node, m_BatchDepth, threadId);
	}
}

void Engine::ClearQueue() {
	std::queue<TreeNode*> empty;
	std::swap(m_Queue,empty);
}

void Engine::AddToQueue(TreeNode* node) {
	std::lock_guard lock(m_QueueMutex);
	m_Queue.push(node);
	m_QueueCondition.notify_one();
}

void Engine::AddManyToQueue(const std::vector<std::unique_ptr<TreeNode>>::iterator& begin,
							const std::vector<std::unique_ptr<TreeNode>>::iterator& end) {
	std::lock_guard lock(m_QueueMutex);
	int size = 0;

	// add jobs to the queue
	for (auto it = begin; it != end; it++) {
		m_Queue.push(it->get());
		size++;
	}

	// notify for each new job
	for (int i = 0; i < size; i++)
		m_QueueCondition.notify_one();
}

std::vector<Move> Engine::GetLine(TreeNode* node) {
	std::vector<Move> line;
	line.push_back(node->delta);

	while (not node->children.empty()) {
		line.push_back(node->bestChild->delta);
		node = node->bestChild;
	}

	return line;
}

std::string Engine::LineToString(const std::vector<Move>& line) {
	std::stringstream ss;
	for (const Move& move : line) {
		ss << move << " | ";
	}
	return ss.str();
}

void Engine::Think() {
	m_Thinking = true;
	ClearQueue();

	// create the root node
	m_Tree = std::make_unique<TreeNode>(Move(), m_Chess.GetBoard().IsWhiteTurn(), nullptr);
	AddToQueue(m_Tree.get());

	// start the treads
	for (int threadId = 0; threadId < n_Threads ; threadId++)
		m_Threads[threadId] = std::thread(&Engine::ThreadWorker, this, threadId);

	// start a batch for each job in the queue
	for (int i = 0 ; i < m_Queue.size(); i++)
		m_QueueCondition.notify_one();
}

void Engine::StopThinking() {
	m_Thinking = false;
	// make sure no threads are waiting for a job
	m_QueueCondition.notify_all();

	for (std::thread& thread : m_Threads)
		thread.join();
}

MoveReturnData Engine::GetBestMove() {
	// throw an error if game is over
	if (m_Chess.IsGameOver())
		throw std::runtime_error("Game is over");

	// calculate the score for each node
	{
		std::jthread thread(Engine::LoadingBar, &m_Tree->score);

		PROFILE_SCOPE_NAME("EvaluateNode");
		Engine::EvaluateNode(m_Tree.get(), LOSS, WIN, m_BatchDepth);
	}

	int totalNodes = std::accumulate(m_NodesPerThread.begin(), m_NodesPerThread.end(), 0);
	std::cout << totalNodes / 1000 << " k nodes" << std::endl;
	std::cout << "Cache hits: " << Board::GetCacheHits();
	std::cout << " Cache misses: " << Board::GetCacheMisses();
	std::cout << std::fixed << std::setprecision(2) << " Cache hit rate " <<
	Board::GetCacheHitRate() * 100 << "%" << std::endl;

	std::cout << "\nBest lines:\n";
	std::array<TreeNode*, 3> bestChildren{};
	for (auto & i : bestChildren) {
		TreeNode* bestChild = nullptr;
		for (auto& child : m_Tree->children) {
			if (std::find(bestChildren.begin(), bestChildren.end(), child.get()) != bestChildren.end())
				continue;

			if (!bestChild or child->score < bestChild->score)
				bestChild = child.get();
		}
		if (!bestChild)
			break;
		i = bestChild;

		std::cout << ScoreLabel(bestChild) << ":\t" << LineToString(GetLine(bestChild)) << std::endl;
		while (bestChild->bestChild) {
			bestChild = bestChild->bestChild;
		}
		std::cout << GetBoardFromNode(bestChild).GetFen() << std::endl;
	}
	std::cout << std::endl;

	// flip the 'mate in' value if it's black's turn
	std::optional<int> mate_in;
	if (bestChildren[0]->mate_in)
		mate_in = bestChildren[0]->whiteTurn ? bestChildren[0]->mate_in.value() : -bestChildren[0]->mate_in.value();
	return {bestChildren[0]->delta, bestChildren[0]->score, mate_in};
}

// use pvs to evaluate the score of the node
// https://en.wikipedia.org/wiki/Principal_variation_search#Pseudocode
Score Engine::EvaluateNode(TreeNode* node, Score alpha, Score beta, int depth) const {
	Board board = GetBoardFromNode(node);
	m_NodesPerThread[0]++;

	// if the node is a leaf, return the static evaluation
	if (depth == 0 or board.GetLegalMoves().empty()) {
		node->score = StaticEvaluation(board);
		if (node->score == LOSS)
			node->mate_in = 0;
		return node->score;
	}

	node->bestChild = nullptr;

	node->score = LOSS; // worst case scenario is that the child is a mate against us
	for (const auto& move : board.GetLegalMoves()) {
		// create a new node for the child
		node->children.push_back(std::make_unique<TreeNode>(move, !node->whiteTurn, node));
		TreeNode* child = node->children.back().get();

		// evaluate the child from the perspective of the current node
		Score childScore = -EvaluateNode(node->children.back().get(), -beta, -alpha, depth - 1);
		// higher child score is better for current node
		if (childScore > node->score) {
			node->score = childScore;
			node->bestChild = child;
		}
		alpha = std::max(alpha, node->score);
		if (alpha >= beta)
			break;
	}

	if (node->bestChild and node->bestChild->mate_in) {
		node->mate_in = node->bestChild->mate_in.value() + 1;
		// if the mate in value is even, it's a mate against us
		if (node->mate_in.value() % 2 == 0)
			node->score = LOSS + (Score)node->mate_in.value();
		else
			node->score = WIN - (Score)node->mate_in.value();
	}
	else
		node->mate_in = std::nullopt;
	return node->score;
}

// the static eval is from the perspective of the current player
Score Engine::StaticEvaluation(const Board& board) {
	PROFILE_SCOPE;
	if (board.GetLegalMoves().empty()) {
		// checkmate
		if (board.IsCheck())
			return LOSS;
		// stalemate
		else
			return 0.f;
	}

	return DefaultStaticEvaluation(board);
}

// always returns a score from the perspective of the current player
Score Engine::DefaultStaticEvaluation(const Board& board) {
	Score score = 0;
	for (int i = 0; i < Board::SIZE * Board::SIZE; i++) {
		switch (board[i]) {
		case 'Q': score += 9; break;
		case 'R': score += 5; break;
		case 'B':
		case 'N': score += 3; break;
		case 'P': score += 1; break;

		case 'q': score -= 9; break;
		case 'r': score -= 5; break;
		case 'b':
		case 'n': score -= 3; break;
		case 'p': score -= 1; break;
		}
	}
	// flip the score if we need to, to ensure that the score is from the perspective of the current player
	// a positive score means that the current player is winning
	return score * (board.IsWhiteTurn() ? 1.f : -1.f);
}

int Engine::Randint(int a, int b) {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(a, b); // distribution in range [a, b]

	return (int)dist6(rng);
}

std::string Engine::ScoreLabel(Score score, std::optional<int> mate_in, bool whiteTurn) {
	std::stringstream ss;
	// set the precision to 2 decimal places
	ss << std::fixed << std::setprecision(2);

	if (mate_in) {
		// if white will win
		if (score > 0 and whiteTurn or
								   score < 0 and !whiteTurn) {
			ss << "# " << mate_in.value() + 1;
			return ss.str();
		}
		// if black will win
		if (score < 0 and whiteTurn or
									score > 0 and !whiteTurn) {
			ss << "#-" << mate_in.value() + 1;
			return ss.str();
		}
	}

	// represent the score as better for white if it's positive
	// even though under the hood we use a negamax algorithm
	Score minmaxScore;
	if (whiteTurn)
		minmaxScore = score;
	else
		minmaxScore = -score;

	// if the score is positive, add a space in front of it
	if (minmaxScore > 0)
		ss << " " << minmaxScore;
	else if (minmaxScore < 0)
		ss << minmaxScore;
		// if the score is 0, write 0 directly to avoid checking for -0
	else
		ss << " 0.00";
	return ss.str();
}