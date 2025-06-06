#include <Stratega/Agent/UnitMCTSAgent/Transition.h>
#include <Stratega/Agent/UnitMCTSAgent/UnitMCTSAgent.h>
#include <cassert>
#include <numeric>

namespace SGA {
    void UnitMCTSAgent::init(GameState initialState, const ForwardModel& /*forwardModel*/, Timer /*timer*/)
    {
        parameters_.PLAYER_ID = getPlayerID();
        parameters_.OPPONENT_ID = 1- parameters_.PLAYER_ID;
        //if (parameters_.heuristic == nullptr)
        //    parameters_.heuristic = std::make_unique<AbstractHeuristic>(initialState);

        if (parameters_.budgetType == Budget::UNDEFINED)
            parameters_.budgetType = Budget::TIME;
        parameters_.opponentModel = std::make_shared<RandomActionScript>();
        if (parameters_.HEURISTIC == "ktk") {
            parameters_.heuristic = std::make_unique< AimToKingHeuristic >(initialState);
        }
        else if (parameters_.HEURISTIC == "pta") {
            parameters_.heuristic = std::make_unique< PushThemAllHeuristic >(getPlayerID(), initialState);
        }
        else {
            std::cout<<"[ERROR] invalid heuristic name parameter\n";
        }
        //parameters_.heuristic = std::make_unique< AimToKingHeuristic >(initialState);
        //parameters_.heuristic = std::make_unique< PushThemAllHeuristic >(getPlayerID(), initialState);
        if(is_debug)
            parameters_.printDetails();
    }
    ActionAssignment UnitMCTSAgent::computeAction(GameState state, const ForwardModel& forwardModel, Timer timer){
        // auto stateCopy(state);
        // parameters_.DO_STATE_ABSTRACTION = false;
        // auto a_test = computeAction_test(stateCopy, forwardModel, timer, true);
        // printf("test finished\n");
        // parameters_.DO_STATE_ABSTRACTION = true;
        auto a_run = computeAction_test(state, forwardModel, timer, false);

        // int hash_test = unitActionHash(a_test);
        // int hash_run = unitActionHash(a_run);

        // state.printActionInfo(a_test);
      //   printf("\nFinal action\n");
      //   state.printActionInfo(a_run);
      //   printf("End final action\n");

      //   if (hash_test != -1 && hash_test != 0 && hash_test == hash_run){
      //    printf("change action: 0\n\n");
      //   } else{
      //    printf("change action: 1\n\n");
      //   }

        // printf("\n");
        return ActionAssignment::fromSingleAction(a_run);
    }

    Action UnitMCTSAgent::computeAction_test(GameState state, const ForwardModel& forwardModel, Timer timer, bool test)
    {
      // printf("do abstraction %d\n", parameters_.DO_STATE_ABSTRACTION);
      //  if (test){
      //    printf("test\n");
      //  }
       bool newRound_cp = newRound;
       int global_absNodeIndex_cp = global_absNodeIndex;
       bool initialized_cp = initialized;
       int previousActionIndex_cp = previousActionIndex;
       int playerTurn_cp = playerTurn;
       int step_cp = step;
       // bool unitIndexInitialized_cp = unitIndexInitialized;
       int unitThisStep_cp = unitThisStep;
       int unitNextStep_cp = unitNextStep;

       // printf("before search, unitThisStep: %d, next: %d", unitThisStep, unitNextStep);

       const auto actionSpace_inspect = forwardModel.generateActions(state, getPlayerID());
       // printf("Action space: %lu\n", actionSpace_inspect.size());

       if(newRound) {
         newRound = false;
       }
       parameters_.global_nodeID = 0;  // reinitialize the ID for node, witch is incremental as nodes created
       auto units = state.getPlayerEntities(getPlayerID()); 
       // state.printBoard();
       // initialize the order of unit moving
       if(unitIndexInitialized == false) {
          for(auto unit : units) {
             unitIndex.push_back(unit.getID());
             // state.printEntityInfo(unit.getID());
          }
          unitIndexInitialized = true;
       }

	   // each step, the unit array's index would change because the unit dies
       std::map< int, int > eIDtoUnitArrayIndex = {};  

       int tmp_counter1 = 0;
       for(auto u : units) {
          eIDtoUnitArrayIndex.insert(std::pair< int, int >(u.getID(), tmp_counter1));
          tmp_counter1 += 1;
       }

       // to decide if we need to advance unit for this step. if current unit dies, or have no valid
       // action, move to the next unit
       bool needNextUnit = false;

       std::vector< Action > actionSpace_tmp = {};

       // Condition to move on: 1. actionSpace is empty, 2. dead
       bool unit_alive = eIDtoUnitArrayIndex.count(unitIndex[unitThisStep]);

       if(! unit_alive) {
          needNextUnit = true;
       } else {
          auto e = units[eIDtoUnitArrayIndex[unitIndex[unitThisStep]]];
          auto actionSpace_tmp = forwardModel.generateUnitActions(state, e, getPlayerID(), false);

          if(actionSpace_tmp.size() == 0) {
             // printf("Need new because this action space is empty!");
             needNextUnit = true;
          } else {
          }
       }
       // printf("need next unit: %d\n", needNextUnit);

	   //std::cout << "Do we need the next unit? " << needNextUnit << std::endl;

       // if need to roll, decide which is the next.
       if(needNextUnit) {
          int tmp_unitNextStep = unitThisStep + 1;
          for(; tmp_unitNextStep < unitIndex.size(); tmp_unitNextStep++) {
             if(eIDtoUnitArrayIndex.count(unitIndex[tmp_unitNextStep]))  // if alive
             {
                break;
             }
          }
          // printf("tmp_unitNextStep: %d", tmp_unitNextStep);

          // execute EndTurn if there is no valid next unit.
          if(tmp_unitNextStep == unitIndex.size()) {  // every unit moved, end the turn

             step++;  // for debugging

             Action endAction = Action::createEndAction(getPlayerID());

             unitThisStep = 0;
             unitNextStep = 1;
             newRound = true;

             // state.printBoard();
			 // std::cout<<"execute EndTurn if there is no valid next unit."<<std::endl;
             if (test){
               // printf("test recover point 1\n");
               newRound= newRound_cp;
               global_absNodeIndex = global_absNodeIndex_cp;
               initialized = initialized_cp;
               previousActionIndex = previousActionIndex_cp;
               playerTurn = playerTurn_cp;
               step = step_cp;
               // unitIndexInitialized = unitIndexInitialized_cp;
               unitThisStep = unitThisStep_cp;
               unitNextStep = unitNextStep_cp;
             }
             // printf("return with action\n");
             //state.printActionInfo(endAction);
             // return ActionAssignment::fromSingleAction(endAction);
             return endAction;
          }

          unitThisStep = tmp_unitNextStep;
       }

       // printf("unitThisStep %d\n", unitThisStep);
       parameters_.REMAINING_FM_CALLS = parameters_.maxFMCalls;


       if(state.getGameType() != GameType::TBS) {
          throw std::runtime_error("MCTSAgent only supports TBS-Games");
       }

       // initialize heuristic
       const auto processedForwardModel = parameters_.preprocessForwardModel(
          dynamic_cast< const TBSForwardModel& >(forwardModel));

       parameters_.maxDepth = 0;  // for debugging

       // generate actions and update unitThisStep
       // const auto actionSpace = forwardModel.generateActions(state, getPlayerID());
       std::vector< Action > actionSpace = forwardModel.generateUnitActions(
          state, units[eIDtoUnitArrayIndex[unitIndex[unitThisStep]]], getPlayerID(), false);
       // printf("unitActionSpace size %ld\n", actionSpace.size());
       // return the only action
       // todo update condition to an and in case we can compare gameStates, since we currently cannot
       // reuse the tree after an endTurnAction
       if(actionSpace.size() == 1) {
          rootNode = nullptr;
          previousActionIndex = -1;
          step++;
          newRound = true;

          // state.printBoard();
          // state.printActionInfo(actionSpace.at(0));
          if (test){
            newRound= newRound_cp;
            global_absNodeIndex = global_absNodeIndex_cp;
            initialized = initialized_cp;
            previousActionIndex = previousActionIndex_cp;
            playerTurn = playerTurn_cp;
            step = step_cp;
            // unitIndexInitialized = unitIndexInitialized_cp;
            unitThisStep = unitThisStep_cp;
            unitNextStep = unitNextStep_cp;
            }
          // return ActionAssignment::fromSingleAction(actionSpace.at(0));
          return actionSpace.at(0);
       } else {
          if(parameters_.CONTINUE_PREVIOUS_SEARCH && previousActionIndex != -1) {
             // in case of deterministic games we know which move has been done by us
             // reuse the tree from the previous iteration
             // rootNode = std::move(rootNode->children.at(previousActionIndex));

             int bestChildID = -1;
             for(int i = 0; i < rootNode->children.size(); i++) {
                if(rootNode->children[i]->childIndex == previousActionIndex) {
                   bestChildID = i;
                }
             }

             rootNode = std::move(rootNode->children.at(bestChildID));

             assert(rootNode != nullptr);
             rootNode->parentNode = nullptr;  // release parent
             rootNode->setDepth(0);

          } else  // start a new tree
          {
             // printf("make a new rootNode!\n player id: %d\n", getPlayerID());
             // auto a_space_unit = forwardModel.generateUnitActions(this->gameState, *unit, playerID, false);
             rootNode = std::make_unique< UnitMCTSNode >(
                *processedForwardModel, state, unitIndex, unitThisStep, getPlayerID(), 0);

          }

          // [Homomorphism] do batches
          int tmp_batch_used = 0;
          int tmp_index = 1000;
          if(! parameters_.DO_STATE_ABSTRACTION) {
              //std::cout<<"start search\n";
              rootNode->searchMCTS(
                 *processedForwardModel,
                 parameters_,
                 getRNGEngine(),
                 &depthToNodes,
                 &absNodeToStatistics);
              //std::cout<<"ends search\n";
          }


          int n_abs_iteration = 0;
          bool stop_abstraction = false;

          while(parameters_.DO_STATE_ABSTRACTION) {
             // printf("During search, remain budget: %f\n", parameters_.REMAINING_FM_CALLS);
             if(parameters_.REMAINING_FM_CALLS <= 0)
                break;

             // std::cout << "[BEFORE SEARCH]: " << parameters_.REMAINING_FM_CALLS <<std::endl;
             rootNode->searchMCTS(
                *processedForwardModel,
                parameters_,
                getRNGEngine(),
                &depthToNodes,
                &absNodeToStatistics);

             // The state abstraction loop, note that we do not cluster the leaves
             for(int i = parameters_.maxDepth - 1; (! stop_abstraction) && i > 0; i--)  // bottom-up
             {
                 // printf("try to abstract\n");
                 std::vector< UnitMCTSNode* > deep_layer = depthToNodes[i];

                 for(auto node1 : deep_layer) {  // each original node
                    if(node1->isAbstracted) continue;

                    if(absNodes[i].size() == 0) {  // this depth has no node cluster

                        absNodes[i].push_back(std::vector< UnitMCTSNode* >{node1});

                        // absNodeToStatistics, absNodeHASH -> absNode.value, absNode.visitingCount
                        absNodeToStatistics.insert(std::pair< int, std::vector< double > >(
                            i * tmp_index + absNodes[i].size() - 1,
                            std::vector< double >{node1->value, float(node1->nVisits)}));

                        node1->isAbstracted = true;
                        node1->absNodeID = i * 1000 + absNodes[i].size() - 1; // a hash approach to generate an ID

                        treeNodetoAbsNode.insert(std::pair< int, int >(node1->nodeID, i*1000+0)); // added 20221014

                        continue;
                    }
                    if(node1->isAbstracted) continue;

                    if(!(parameters_.random_abstraction)){
                       bool foundExistGroup = false;

                        for(int j = 0; j < absNodes[i].size(); j++)  // each abstract nodes: nodes cluster
                        {
                              bool match = false;
                              for(int k = 0; k < absNodes[i][j].size(); k++) {  // compare between new ground nodes to the abstract Nodes

                                 if(isTwoNodeApproxmateHomomorphism(
                                    forwardModel,
                                    node1,
                                    absNodes[i][j][k],
                                    parameters_.R_THRESHOLD,
                                    parameters_.T_THRESHOLD)) {
                                       match = true;
                                       break;
                                    } // end if
                           } // end for
                           if(match) {
                              node1->isAbstracted = true;
                              node1->absNodeID = i * tmp_index + j;
                              absNodes[i][j].push_back(node1);  // add into existing group
                              treeNodetoAbsNode.insert(std::pair< int, int >(node1->nodeID, node1->absNodeID));

                              foundExistGroup = true;
                           }//end if (match)
                        }
                        if(node1->isAbstracted) continue;


                       // not found existing abstract node to add in, create a new one
                       if(! foundExistGroup) {
                          absNodes[i].push_back(std::vector< UnitMCTSNode* >{node1});
                          absNodeToStatistics.insert(std::pair< int, std::vector< double > >(
                             i * tmp_index + absNodes[i].size() - 1,
                             std::vector< double >{node1->value, float(node1->nVisits)}));

                          // absNodeToStatistics[i * 1000 + absNodes[i].size() - 1] = std::vector<double>{
                          // node1->value, double(node1->nVisits) };

                          node1->isAbstracted = true;
                          node1->absNodeID = i * tmp_index + absNodes[i].size() - 1;

                       } // end if(! foundExistGroup) {
                    } else {
                       /*
                        * uniformly add a initial node to abstact node/ create a new abstract node
                        */
                       int nAction = absNodes[i].size();
                       
                       boost::random::uniform_int_distribution<size_t> actionDist(0, nAction); // close interval
                       auto actionIndex = actionDist(getRNGEngine());

                       // create a new abstact node
                       if (actionIndex == nAction) {

                          absNodes[i].push_back(std::vector< UnitMCTSNode* >{node1});

                          // absNodeToStatistics, absNodeHASH -> absNode.value, absNode.visitingCount
                          absNodeToStatistics.insert(std::pair< int, std::vector< double > >(
                             i * tmp_index + absNodes[i].size() - 1,
                             std::vector< double >{node1->value, float(node1->nVisits)}));

                          node1->isAbstracted = true;
                          node1->absNodeID = i * tmp_index + absNodes[i].size() - 1;
                          treeNodetoAbsNode.insert(std::pair<int, int>(node1->nodeID, node1->absNodeID ));

                          continue;
                       }
                       else {

                          //int j = absNodes[i][actionIndex].size()-1;
                          int j = actionIndex;
                          node1->isAbstracted = true;
                          node1->absNodeID = i * tmp_index + j;

                          absNodes[i][actionIndex].push_back(node1);  // add into existing group
                          treeNodetoAbsNode.insert(std::pair< int, int >(node1->nodeID, i * 1000 + j));

                       }
                    }

                    node1->isAbstracted = true;
                 }

             }

             n_abs_iteration++;
             tmp_batch_used++;


             // printf("absNodeToStatistics.size: %lu\n", absNodeToStatistics.size());
             /* analyze the compression rate
             if (tmp_batch_used < 21 && (absNodeToStatistics.size() != 0) && (treeNodetoAbsNode.size() != 0))
                    std::cout<<"compression_rate: " << tmp_batch_used << " (batch) " << float(treeNodetoAbsNode.size()) / absNodeToStatistics.size() << " (rate) " << std::endl;
             //*/

             // tmp_batch_used >=20 means do maximum 20 times abstraction in a step
             if(parameters_.REMAINING_FM_CALLS <= 0 || rootNode->n_search_iteration >= parameters_.maxFMCalls) {

                 std::cout<<"End searching, number of abs Node each depth:\n";
                 //rootNode->printTree();
                 printAbsNodeStatus();

                 rootNode->eliminateAbstraction(&absNodeToStatistics);
                 //deleteAbstraction();
                 //std::cout<<"maximum batch: "<< n_abs_iteration << " \n";

                 break;
             }


             if(!stop_abstraction && tmp_batch_used >= parameters_.absBatch) {
                 printf("batch_used: %d, try to eliminate abs\n", tmp_batch_used);
                 printAbsNodeStatus();

                 //std::cout<<"\n";
                 stop_abstraction = true;
                 rootNode->eliminateAbstraction(&absNodeToStatistics);
                 deleteAbstraction();
                 break; // this is not the algorithm logic, just for inspection
             }

          }
          /*
          if (parameters_.DO_STATE_ABSTRACTION) {
              std::cout<<rootNode->n_search_iteration<<"\n";
          }
          //*/
          //std::cout<<"iteration " << rootNode->n_search_iteration<<"\n";
          //std::cout<<n_abs_iteration<<"\n";

          // printf("After search, root visted: %d\n", rootNode->n_search_iteration);
          auto bestActionIndex = rootNode->mostVisitedAction( parameters_, getRNGEngine() );

          //printAbsNodeStatus();
         // for (size_t i = 0; i < (rootNode->children).size(); ++i)
         // {
         //    UnitMCTSNode* child = rootNode->children[i].get();

         //    child->print();

         // }
          /*if (bestActionIndex == actionSpace.size()-1) { // this action is an endTurn, reinitialize
              //unitThisStep = 0;
              unitNextStep = 0;
          }*/

          auto bestAction = rootNode->getActionSpace(forwardModel, getPlayerID()).at(bestActionIndex);

		  /*
		  for (auto a : rootNode->getActionSpace(forwardModel, getPlayerID())) {
		    state.printActionInfo(a);
		  }
		  state.printActionInfo(bestAction);
		  //*/

          // calculate the branching factor
          std::vector< int > branching_number = {};
          int n_node = 0;
          double mean_braching_factor = 0.0;

          /*
          rootNode->get_branching_number(&branching_number, &n_node);

          if(branching_number.size() != 0) {
             mean_braching_factor = std::accumulate(
                                       branching_number.begin(), branching_number.end(), 0.0)
                                    / branching_number.size();
          }
          */

          if(false && step % 20 == 0) {
             if(parameters_.DO_STATE_ABSTRACTION)
                std::cout << " ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ HomoMCTS Branching Factor " << step << " "
                          << n_node << " " << mean_braching_factor << " " << parameters_.maxDepth
                          << " ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ " << std::endl;
             else
                std::cout << " ------------------------------ UnitMCTS Branching Factor " << step << " "
                          << n_node << " " << mean_braching_factor << " " << parameters_.maxDepth
                          << " ------------------------------ " << std::endl;
          }

          if(bestAction.getActionFlag() == ActionFlag::EndTickAction)
             step++;

          previousActionIndex = (bestAction.getActionFlag() == ActionFlag::EndTickAction)
                                   ? -1
                                   : bestActionIndex;  // std::cout << "*** 9" << std::endl;

          if(n_node == 1)
             previousActionIndex = -1;

          // std::cout << "-> UnitMCTS Action Took: ";
          // state.printActionInfo(bestAction);
          // std::cout << unitActionHash(bestAction) << std::endl;
          // std::cout << " End printActionInfo " << std::endl;
          //state.printBoard();
          //state.printActionInfo(bestAction);
          //auto reward = parameters_.heuristic->evaluateGameState(forwardModel, state, getPlayerID());
          //std::cout<<"reward: "<<reward<<"\n";
          //rootNode->printTree();
          //double score  = parameters_.heuristic->evaluateGameState(forwardModel, state,parameters_.PLAYER_ID );
          //std::cout<<score << "\n";

         /*
          for (int i = 1; i < 10; i++) { // depth
               if (absNodes[i].size() == 0) {
                  std::cout<<"\n";
                  break;
              }

              std::cout << "depth: "<< i << ", N absNode: " << absNodes[i].size() << " : \n";

             
              for (int j = 0; j < absNodes[i].size(); j++) { // abs node
                  if (j > 0) {
                      std::cout << "\t";
                  }
                  std::cout <<absNodes[i][j].size();

                  //for (int k = 0; k < absNodes[i][j].size(); k++) // original tree nodes
                  //{
                  //    std::cout << " " << absNodes[i][j][k]->value / absNodes[i][j][k]->nVisits << " , "
                  //      << absNodes[i][j][k]->nVisits <<";";
                  //}
                  //std::cout << "[END]" << std::endl;
              }
              std::cout << std::endl;
          }
          //std::cout << " [DEBUG382]: parameters_.REMAINING_FM_CALLS: " << parameters_.REMAINING_FM_CALLS
          //<< std::endl;
          //*/

          if(bestAction.getActionFlag() == ActionFlag::EndTickAction) {
             newRound = true;
          }

          if (parameters_.DO_STATE_ABSTRACTION) {
              stepInit();  // reinitialize homomorphism
          }
          //system("pause");
          if (test){
               newRound= newRound_cp;
               global_absNodeIndex = global_absNodeIndex_cp;
               initialized = initialized_cp;
               previousActionIndex = previousActionIndex_cp;
               playerTurn = playerTurn_cp;
               step = step_cp;
               // unitIndexInitialized = unitIndexInitialized_cp;
               unitThisStep = unitThisStep_cp;
               unitNextStep = unitNextStep_cp;
          }
          // return ActionAssignment::fromSingleAction(bestAction);
          printf("\nFinal action\n");
          state.printActionInfo(bestAction);
          printf("End final action\n");
          return bestAction;
       }
    }


    bool UnitMCTSAgent::isTwoNodeApproxmateHomomorphism(const ForwardModel& forwardModel, UnitMCTSNode* node1, UnitMCTSNode* node2, double reward_threshold, double transition_threshold)
    {
        // node 2 is a ground node from an abstract node
       auto actionSpace1 = node1->getActionSpace(forwardModel, getPlayerID());// ->actionSpace;
       auto actionSpace2 = node2->getActionSpace(forwardModel, getPlayerID());
       double max_reward_difference = 0.0;
       std::map< int, int > commonActions = std::map< int, int >();
       std::map< int, int > actionsUnion = std::map<int, int> ();
       std::vector< int > commonActionsVector = std::vector< int >();

       // reward checking
       for(int i = 0; i < node1->actionHashVector.size(); i++) {
          double diff_this = 0.0;
          int a = node1->actionHashVector[i];
          actionsUnion.insert(std::pair< int, int >(a, 1));
          if(node2->actionHashes.count(a)) {
             commonActions.insert(std::pair< int, int >(a, 1));
             commonActionsVector.push_back(a);
             diff_this = abs(node1->actionToReward[a] - node2->actionToReward[a]);
          } else {
             diff_this = abs(
                node1
                   ->actionToReward[a]);  // not common action.
             // different setting. multiply [0-1]
          }
          if(diff_this > max_reward_difference) {
             max_reward_difference = diff_this;
          }
       }

       for(int i = 0; i < node2->actionHashVector.size(); i++) {
          int a = node2->actionHashVector[i];
          if (!actionsUnion.count(a)) {
              actionsUnion.insert(std::pair<int, int>(a, 1));
          }
          if(! commonActions.count(a))
             continue;
          double diff_this = 0.0;

          if(node1->actionHashes.count(a)) {
             diff_this = abs(node1->actionToReward[a] - node2->actionToReward[a]);
          } else {
             diff_this = abs(node2->actionToReward[a]);
          }
          if(diff_this > max_reward_difference) {
             max_reward_difference = diff_this;
          }
       }
       if(max_reward_difference > reward_threshold) {
          return false;
       }

       /*
       A   1, 2
       B   2, 3
       */
       // for all transition, in deterministic situation, it could only be 
       // for common action a
       // 0, when T(s1, a, s3) =1.0 and T(s2, a, s3)=1.0
       // 2.0, when T(s1, a, s3) =1.0 and T(s2, a, s3) = 0.0; T(s1, a, s4) =1.0 and T(s2, a, s4) = 0.0;
       // for non-common action a1 for s1, a2 for s2
       // 2.0, T(s1, a1, s3) =1.0, T(s2, a1, s3) = 0.0; T(s1, a2, s4) =0.0, T(s2, a2, s4) = 1.0;
       for (auto a_pair : actionsUnion) {
           int a = a_pair.first;
           double diff_this = 0.0;
           if (node1->actionHashes.count(a) && node2->actionHashes.count(a)) {
               if (node1->actionToNextState[a] == node2->actionToNextState[a]) {
               }
               else {
                   diff_this+=2.0;
               }
           }
           else {
               diff_this+=2.0;
           }
           if(diff_this > transition_threshold)return false;
       }

       return true;
    }

    // actual reward, what if using the approximate Q? combination of heuristic score [stage1]
    bool UnitMCTSAgent::isActionIndependentHomomorphism(const ForwardModel& forwardModel, UnitMCTSNode* node1, UnitMCTSNode* node2, double reward_threshold, double transition_threshold)
    {
       auto actionSpace1 = node1->getActionSpace(forwardModel, getPlayerID());// ->actionSpace;
       auto actionSpace2 = node2->getActionSpace(forwardModel, getPlayerID());
       double max_reward_difference = 0.0;
       std::map< int, int > commonActions = std::map< int, int >();
       std::vector< int > commonActionsVector = std::vector< int >();

       // reward checking
       for(int i = 0; i < node1->actionHashVector.size(); i++) {
          double diff_this = 0.0;
          int a = node1->actionHashVector[i];
          if(node2->actionHashes.count(a)) {
             commonActions.insert(std::pair< int, int >(a, 1));
             commonActionsVector.push_back(a);
             diff_this = abs(node1->actionToReward[a] - node2->actionToReward[a]);
          } else {
             diff_this = abs(
                node1
                   ->actionToReward[a]);  // not common action. how many common actions in general [9/3]
             // different setting. multiply [0-1]
          }
          if(diff_this > max_reward_difference) {
             max_reward_difference = diff_this;
          }
       }

       for(int i = 0; i < node2->actionHashVector.size(); i++) {
          int a = node2->actionHashVector[i];
          if(! commonActions.count(a))
             continue;
          double diff_this = 0.0;

          if(node1->actionHashes.count(a)) {
             diff_this = abs(node1->actionToReward[a] - node2->actionToReward[a]);
          } else {
             diff_this = abs(node2->actionToReward[a]);
          }
          if(diff_this > max_reward_difference) {
             max_reward_difference = diff_this;
          }
       }
       if(max_reward_difference > reward_threshold) {
          return false;
       }
       

       
       //A   1, 2
      // B   2, 3
       
       // for all transition
       double transition_error = 0;
       for(int i = 0; i < node1->nextStateHashVector.size(); i++) {
          int s_prime = node1->nextStateHashVector[i];  // s' next state
          if(! (node2->stateCounter.count(s_prime))) {  // get the same next state
             transition_error += 1.0;
          }
       }

       for(int i = 0; i < node2->nextStateHashVector.size(); i++) {
          int s_prime = node2->nextStateHashVector[i];
          if(! (node1->stateCounter.count(s_prime))) {  // get the same next state
             transition_error += 1.0;
          }
       }

       if(transition_error > transition_threshold) {
          return false;
       }

       // one of the two condition make the merging happen. 9/3
       return true;
    }

    void UnitMCTSAgent::printAbsNodeStatus() {
         printf("\nprintAbsNodeStatus\n");
         for (int i = 1; i < 2; i++) {
            int abs_size = absNodes[i].size();
            // printf("checking depth %d\n", absNodes[i][0][0]->nodeDepth);
            //std::cout<< "depth: "<< i << " abs Node: "<< abs_size << "\n";
            if(abs_size == 0) continue;

            for (int j = 0; j < abs_size; j++) {
               // std::cout<< absNodes[i][j].size()<< " ";
               if (i == 1 && absNodes[i][j].size() == 1){
                  // printf("\n");
                  // for (int k = 0; k < absNodes[i][j].size(); k++) {
                  //    absNodes[i][j][k]->print();
                  // }
                  // printf("\n\n");
                  absNodes[i][j][0]->printActionInfo();
               }
            }
            //std::cout<<"\n";
        }
        printf("\nEnd printAbsNodeStatus\n");
    }
   // void UnitMCTSAgent::printAbsNodeStatus() {
   //    printf("\nprintAbsNodeStatus\n");
   //    for (int i = 1; i < parameters_.maxDepth; i++) {
   //       int abs_size = absNodes[i].size();
   //       // printf("checking depth %d\n", absNodes[i][0][0]->nodeDepth);
   //       std::cout<< "depth: "<< i << " abs Node: "<< abs_size << "\n";
   //       if(abs_size == 0) continue;

   //       for (int j = 0; j < abs_size; j++) {
   //          std::cout<< absNodes[i][j].size()<< " ";
   //          if (i == 1){
   //             printf("\n");
   //             for (int k = 0; k < absNodes[i][j].size(); k++) {
   //                absNodes[i][j][k]->print();
   //             }
   //             printf("\n\n");
   //          }
   //       }
   //       std::cout<<"\n";
   //    }
   // }
}  // namespace SGA