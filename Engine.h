
#include "Chess.h"

typedef float Score;

struct TreeNode {
	Move delta;
	Score score = 0;
	std::optional<int> mate_in;
	bool whiteTurn = false;

	TreeNode* parent = nullptr;
	TreeNode* bestChild = nullptr;
	std::vector<std::unique_ptr<TreeNode>> children;

	// if we have calculated the board for this node,
	// we can store the fen here to index the cache
	// std::optional<std::string> fen;

	TreeNode() = default;
	TreeNode(Move delta, bool whiteTurn, TreeNode* parent = nullptr) :
			delta(std::move(delta)), whiteTurn(whiteTurn), parent(parent) {}

};

// compare two nodes based on their score
bool operator<(const TreeNode& lhs, const TreeNode& rhs);

struct MoveReturnData {
	Move move;
	Score score = 0;
	std::optional<int> mate_in;
};

class Engine {
public:
	explicit Engine(Chess& c);

	void Think();
	void StopThinking();
	void ApplyMove(const Move& move);
	void ApplyThinkingPolicy();


	[[nodiscard]] static Score StaticEvaluation(const Board& board);
	[[nodiscard]] static Score DefaultStaticEvaluation(const Board& board);

	[[nodiscard]] MoveReturnData GetBestMove();

	static std::vector<Move> GetLine(TreeNode* node);
	static std::string LineToString(const std::vector<Move>& line) ;
	[[nodiscard]] static std::string ScoreLabel(const TreeNode* node) {
		return ScoreLabel(node->score, node->mate_in, node->whiteTurn);
	}
	[[nodiscard]] static std::string ScoreLabel(Score score, std::optional<int> mate_in, bool whiteTurn);
private:

	static int Randint(int a, int b);
	void ExpandNode(TreeNode* node, int depth, int threadId);
	Score EvaluateNode(TreeNode* node, Score alpha, Score beta, int depth) const;

	void ThreadWorker(int threadId);
	static void LoadingBar(const std::stop_token& st, const Score* score);
	[[nodiscard]] Board GetBoardFromNode(TreeNode* node) const;
	void ClearQueue();
	void AddToQueue(TreeNode* node);

	void AddManyToQueue(const std::vector<std::unique_ptr<TreeNode>>::iterator& begin,
						const std::vector<std::unique_ptr<TreeNode>>::iterator& end);

	Chess& m_Chess;

	static const int n_Threads = 1;
	std::array<std::thread, n_Threads> m_Threads;
	std::mutex m_QueueMutex;
	std::condition_variable m_QueueCondition;
	std::queue<TreeNode*> m_Queue;
	std::atomic_bool m_Thinking = true;

	const int m_BatchDepth = 12;
	int m_msThinkTime = 1000;
	std::unique_ptr<TreeNode> m_Tree = nullptr;

	mutable std::array<int, n_Threads> m_NodesPerThread = {0};

	static std::unordered_map<std::string, Score> m_Evaluations;
	constexpr static Score LOSS = -1000;
	constexpr static Score WIN =   1000;
};