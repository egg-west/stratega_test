#pragma once
#include <Stratega/Agent/NaiveMCTSAgent/NaiveMCTSParameters.h>
#include <Stratega/Agent/TreeSearchAgents/TreeNode.h>

namespace SGA {

	class NaiveMCTSNode: public ITreeNode< NaiveMCTSNode >
	{
		/*each node except for the root node is generated by the parent node gameState executing a combination of unit actions for all units
		* the parent node maintains the visit counts and value for each combination as well as each unit action
		* 
		* Notation: X_i is a action-combination
		*/
	public:
		int nodeDepth = 0;			//Depth of this node in the tree.
		bool isBottom = false;
		int treesize = 1;			//Reference to the number of nodes in the tree behind this node
        int childExpanded = 0;

        std::vector<std::vector<double> >  combinationValue = {};  // value of each value of each X_i
		std::vector<std::vector<int> >  combinationCount = {};
        int numUnit = 0;						  // number of units decides the maximum i
        int entityActionSpaceSize[100] = {};

		/*[unit ID][unit Action]*/
		std::vector< std::vector<SGA::Action> > nodeActionSpace;
		std::vector< double > childrenValue;
		std::vector< int > childrenCount;
		std::unordered_map<std::string, int> combinationToChildID;

		/*action combination executed from its parent node*/
		std::vector< int > actionCombinationTook = {};
		

	protected:
		int nVisits = 0;			//number of visits to this node.
		double bounds[2] = { 0, 1 };	//Reward bounds in this node.


	public:

		/// <summary>
		/// Initializes the node.
		/// </summary>
		void initializeNode(ForwardModel& forwardModel, GameState newGameState, int newOwnerID);

		/// <summary>
		/// MCTS Tree Policy phase.
		/// </summary>
		NaiveMCTSNode* treePolicy(ForwardModel& forwardModel, NaiveMCTSParameters& params, boost::mt19937& randomGenerator);

		/// <summary>
		/// MCTS Expansion phase.
		/// </summary>
		NaiveMCTSNode* expand(ForwardModel& forwardModel, NaiveMCTSParameters& params, boost::mt19937& randomGenerator);

		/// <summary>
		/// Chooses a child node with the highest UCB value.
		/// </summary>
		NaiveMCTSNode* uct(NaiveMCTSParameters& params, boost::mt19937& randomGenerator, ForwardModel& forwardModel);

		/// <summary>
		/// MCTS Rollout phase.
		/// </summary>
		double rollOut(ForwardModel& forwardModel, NaiveMCTSParameters& params, boost::mt19937& randomGenerator);

		/// <summary>
		/// Indicates if a rollout should be completed.
		/// </summary>
		static bool rolloutFinished(GameState& rollerState, int depth, NaiveMCTSParameters& params);

		/// <summary>
		/// MCTS Back-propagation phase.
		/// </summary>
		static void backUp(NaiveMCTSNode* node, double result, NaiveMCTSParameters& params);

		/// <summary>
		/// Returns the best action found by the tree.
		/// </summary>
		int bestAction(NaiveMCTSParameters& params, boost::mt19937& randomGenerator);

		// helper function: normalizes a value between a range aMin - aMax.
		static double normalize(double aValue, double aMin, double aMax);

		// helper function: adds a small random noise to a value and returns it.
		static double noise(double input, double epsilon, double random);

		// setter for the depth of this node and all nodes in the sub-tree this is root of.
		void setDepth(int depth);

		// Increments the counter of nodes below this one.
		void increaseTreeSize();


		//Applies an action "action" to a game state "gameState", using "forwardModel". It updates "params" for budget concerns.
		void applyActionToGameState(ForwardModel& forwardModel, 
									GameState& targetGameState, 
									Action& action, 
									NaiveMCTSParameters& params, 
									int playerID) const;

		// update values for backUp
		void updateValue(std::vector<int> actionCombinationTook, const double result, NaiveMCTSParameters& params);

	public:
		// Root Node Constructor
		NaiveMCTSNode(ForwardModel& forwardModel, GameState gameState, std::vector<int> actionCombination, int ownerID);

		/// <summary>
		/// Function for the main iteration of the MCTS algorithm
		/// </summary>
		void searchMCTS(ForwardModel& forwardModel, NaiveMCTSParameters& params, boost::mt19937& randomGenerator);

		/// <summary>
		/// Prints information of this node.
		/// </summary>
		void print() const override;

		void printActionTookInfo() const;

        void printActionInfo() const;

		void printNaiveInfo() const;

		void printNaiveInfo(std::string& prefix) const;

		void printNaiveTree(std::string& prefix);

	private:

		// Creates a new node to be inserted in the tree.
		NaiveMCTSNode(ForwardModel& forwardModel, 
					  GameState gameState, 
					  NaiveMCTSNode* parent, 
					  std::vector<int> actionCombination,
					  int childIndex, 
					  int ownerID);

	};
}